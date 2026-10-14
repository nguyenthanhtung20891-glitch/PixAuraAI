#include "pixaura/inference.h"
#include "model_package_internal.hpp"
#include "onnx_profile.hpp"
#include "evaluation.hpp"
#include <algorithm>
#include <atomic>
#include <cfenv>
#include <cmath>
#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <string_view>
namespace pixaura::inference {
namespace {
constexpr uint64_t live_tag=0x5058494e46455231ULL,dead_tag=0x5058494e46455230ULL,control=131072;
constexpr std::string_view provider="cpu.reference.v1";
struct State;
struct Lease {
    State* owner;uint64_t bytes;
    Lease(State&,uint64_t);~Lease();Lease(const Lease&)=delete;
};
struct Package { std::shared_ptr<Lease> lease;std::unique_ptr<const pixaura_model_package> value; };
struct ExecutionControl {void(*check)(void*,evaluation::Checkpoint);void* state;};
struct PreparedExecution {
    std::array<uint32_t,4> input{},output{};
    virtual ~PreparedExecution()=default;
    virtual void run(const float*,float*,const ExecutionControl&)const=0;
};
struct CpuReference final:PreparedExecution {
    Plan plan;
    explicit CpuReference(Plan p):plan(p){input=p.input;output=p.output;}
    void run(const float* in,float* out,const ExecutionControl& execution_control)const override{
        const auto pixels=static_cast<size_t>(input[2])*input[3];
        for(size_t i=0;i<pixels;++i){if(i%1024==0){execution_control.check(execution_control.state,evaluation::Checkpoint::tile);}double sum=0;
            for(size_t channel=0;channel<3;++channel)sum=sum+binary32_value(in[channel*pixels+i])*binary32_value(plan.weights[channel]);
            out[i]=binary32_round(sum);
        }
    }
};
std::unique_ptr<const PreparedExecution> prepare_cpu(const pixaura_model_package& value){
    const auto& binding=value.binding;
    require(value.info.file_count==1&&binding.runtime_floor==1&&binding.api_floor==1&&binding.operator_set==1&&binding.operators.size()==1&&binding.operators.front()==1,PIXAURA_INFERENCE_MODEL_UNSUPPORTED);
    const pixaura::model::File* file=nullptr;for(const auto& f:value.files){if(f.name==value.binding.cpu_payload){file=&f;}}
    require(file,PIXAURA_INFERENCE_MODEL_INVALID);return std::make_unique<CpuReference>(parse_onnx(file->bytes.data(),file->bytes.size()));
}
struct Backend {std::string_view id;std::unique_ptr<const PreparedExecution>(*prepare)(const pixaura_model_package&);};
const std::array<Backend,1> backends{{{provider,prepare_cpu}}};
struct Model {
    std::shared_ptr<Lease> lease;std::shared_ptr<const Package> package;std::unique_ptr<const PreparedExecution> prepared;
    evaluation::Cancellation cancellation;std::atomic<bool> retired{false};uint64_t last=0,active=0;
};
struct Output { std::shared_ptr<Lease> lease;std::unique_ptr<float[]> values;size_t count=0;pixaura_inference_output info{}; };
struct Entry { uint32_t kind=0;std::shared_ptr<const Package> package;std::shared_ptr<Model> model;std::shared_ptr<const Output> output; };
struct State {
    std::mutex mutex;std::map<uint64_t,Entry> entries;uint64_t next=1,ceiling;
    std::atomic<uint64_t> used{control};bool executing=false;
#ifdef PIXAURA_CERTIFICATION_TESTING
    evaluation::Observer observer=nullptr;void* observer_state=nullptr;bool output_fault=false;
#endif
    explicit State(uint64_t n):ceiling(n){}
};
Lease::Lease(State& s,uint64_t n):owner(&s),bytes(n){auto used=s.used.load();
    for(;;){require(n<=s.ceiling-used,PIXAURA_INFERENCE_RESOURCE);if(s.used.compare_exchange_weak(used,used+n))break;}}
Lease::~Lease(){owner->used.fetch_sub(bytes);}
struct Header {uint64_t tag;State* state;uint8_t identity[32];};
static_assert(sizeof(Header)<=sizeof(pixaura_inference_context::opaque),"opaque context");
static_assert(sizeof(pixaura_inference_handle)==56&&sizeof(pixaura_inference_tensor)==72,"inference ABI");
static_assert(sizeof(pixaura_inference_request)==96&&sizeof(pixaura_inference_output)==120,"request/result ABI");
static_assert(sizeof(pixaura_inference_descriptor)==152&&sizeof(pixaura_inference_resources)==32,"descriptor/resources ABI");
static_assert(sizeof(pixaura_inference_model_metadata)==200,"model metadata ABI");
bool overlap(const void* a,uint64_t an,const void* b,uint64_t bn){
    if(!an||!bn){return false;}require(a&&b);const auto x=reinterpret_cast<uintptr_t>(a),y=reinterpret_cast<uintptr_t>(b);
    require(an<=UINTPTR_MAX-x&&bn<=UINTPTR_MAX-y);return x<y+bn&&y<x+an;
}
void separate(const void* out,uint64_t n,const void* input,uint64_t bytes){require(!overlap(out,n,input,bytes));}
template<class F> int32_t boundary(F&& f)noexcept{try{f();return 0;}catch(const Error&e){return e.code;}
    catch(const std::bad_alloc&){return PIXAURA_INFERENCE_RESOURCE;}catch(...){return PIXAURA_INFERENCE_INTERNAL;}}
Header live(pixaura_inference_context* c){require(c);Header h{};std::memcpy(&h,c->opaque,sizeof(h));
    require(c->api_version==1&&c->struct_size==sizeof(*c)&&c->reserved==0&&h.tag==live_tag&&h.state,PIXAURA_INFERENCE_STALE_HANDLE);return h;}
void output_pointer(const Header& h,pixaura_inference_context* c,void* out,uint32_t size,uint32_t expected){
    require(out&&size==expected,size==expected?PIXAURA_INFERENCE_INVALID_ARGUMENT:PIXAURA_INFERENCE_UNSUPPORTED_ABI);
    separate(out,size,c,sizeof(*c));separate(out,size,h.identity,0);
}
const Backend* find_provider(const char* p,uint64_t n){if(p){for(const auto& backend:backends){if(n==backend.id.size()&&!std::memcmp(p,backend.id.data(),static_cast<size_t>(n))){return &backend;}}}return nullptr;}
bool is_provider(const char* p,uint64_t n){return find_provider(p,n)!=nullptr;}
Entry acquire_locked(const Header& h,const pixaura_inference_handle* token,uint32_t kind){
    require(token);require(token->version==1&&token->struct_size==sizeof(*token)&&!token->reserved&&token->serial&&
        !std::memcmp(token->context_id,h.identity,32),PIXAURA_INFERENCE_STALE_HANDLE);
    const auto it=h.state->entries.find(token->serial);require(it!=h.state->entries.end()&&it->second.kind==token->kind,PIXAURA_INFERENCE_STALE_HANDLE);
    require(!kind||it->second.kind==kind,kind==1?PIXAURA_INFERENCE_UNVERIFIED_MODEL:PIXAURA_INFERENCE_STALE_HANDLE);return it->second;
}
pixaura_inference_handle insert_locked(const Header& h,Entry entry){
    auto& s=*h.state;require(s.entries.size()<32&&s.next!=UINT64_MAX,PIXAURA_INFERENCE_RESOURCE);
    pixaura_inference_handle t{};t.version=1;t.struct_size=sizeof(t);t.kind=entry.kind;t.serial=s.next;
    std::memcpy(t.context_id,h.identity,32);s.entries.emplace(s.next,std::move(entry));++s.next;return t;
}
std::shared_ptr<const Package> adopt(const Header& h,std::unique_ptr<pixaura_model_package> p){
    uint64_t bytes=65536;for(const auto& f:p->files){require(f.bytes.size()<=UINT64_MAX-bytes,PIXAURA_INFERENCE_RESOURCE);bytes+=f.bytes.size();}
    auto result=std::make_shared<Package>();result->lease=std::make_shared<Lease>(*h.state,bytes);result->value=std::move(p);return result;
}
pixaura_inference_tensor tensor(const std::array<uint32_t,4>& shape,const std::array<uint32_t,6>& normalization){
    pixaura_inference_tensor t{};t.version=1;t.struct_size=sizeof(t);t.type=1;t.layout=1;t.rank=4;
    std::copy(shape.begin(),shape.end(),t.dimensions);std::copy(normalization.begin(),normalization.end(),t.normalization);
    uint64_t n=1;for(auto d:shape){require(d&&d<=1024&&n<=4194304/d,PIXAURA_INFERENCE_INPUT_SHAPE);n*=d;}t.bytes=n*4;return t;
}
void checkpoint(const std::shared_ptr<Model>& m,State& state,evaluation::Checkpoint phase){
#ifdef PIXAURA_CERTIFICATION_TESTING
    if(state.observer)state.observer(phase,state.observer_state);
#else
    (void)state;(void)phase;
#endif
    require(!m->retired.load()&&!m->cancellation.requested.load(),PIXAURA_INFERENCE_CANCELLED);
}
}
}
using namespace pixaura::inference;
namespace evaluation = pixaura::evaluation;
extern "C" int32_t pixaura_inference_descriptor_query(uint32_t version,const char* id,uint64_t n,pixaura_inference_descriptor* out,uint32_t size){return boundary([&]{
    require(version==1&&size==sizeof(*out),PIXAURA_INFERENCE_UNSUPPORTED_ABI);require(out);require(is_provider(id,n),PIXAURA_INFERENCE_PROVIDER_UNAVAILABLE);separate(out,size,id,n);
    pixaura_inference_descriptor d{};d.version=1;d.struct_size=sizeof(d);d.provider_version=1;d.backend=1;d.task=1;d.type=1;d.layout=1;
    d.deterministic=1;d.availability=2;d.validation=1;d.max_handles=32;d.max_dimension=1024;d.max_context_bytes=268435456;
    std::memcpy(d.provider_id,provider.data(),provider.size());const char profile[]="pixaura.onnx.reference-segmentation.v1";std::memcpy(d.profile_id,profile,sizeof(profile));*out=d;
});}
extern "C" int32_t pixaura_inference_context_init(uint32_t version,pixaura_inference_context* c,uint32_t size,const uint8_t* id,uint64_t n,uint64_t ceiling){return boundary([&]{
    require(version==1&&size==sizeof(*c),PIXAURA_INFERENCE_UNSUPPORTED_ABI);require(c&&id&&n==32);separate(c,size,id,n);
    require(ceiling>=control&&ceiling<=268435456,PIXAURA_INFERENCE_RESOURCE);for(size_t i=0;i<32;++i)require((id[i]>='0'&&id[i]<='9')||(id[i]>='a'&&id[i]<='f'));
    const auto* bytes=reinterpret_cast<const uint8_t*>(c);for(size_t i=0;i<sizeof(*c);++i)require(bytes[i]==0,PIXAURA_INFERENCE_STALE_HANDLE);
    auto s=std::make_unique<State>(ceiling);Header h{live_tag,s.get(),{}};std::memcpy(h.identity,id,32);
    c->api_version=1;c->struct_size=sizeof(*c);std::memcpy(c->opaque,&h,sizeof(h));s.release();
});}
extern "C" int32_t pixaura_inference_context_destroy(pixaura_inference_context* c){return boundary([&]{auto h=live(c);require(!h.state->executing,PIXAURA_INFERENCE_BUSY);
    h.state->entries.clear();require(h.state->used.load()==control,PIXAURA_INFERENCE_INTERNAL);delete h.state;h.state=nullptr;h.tag=dead_tag;std::memcpy(c->opaque,&h,sizeof(h));});}
extern "C" int32_t pixaura_inference_package_verify(pixaura_inference_context* c,const char* root,uint64_t n,pixaura_inference_handle* out,uint32_t size,pixaura_model_diagnostic* diagnostic){return boundary([&]{
    const auto h=live(c);output_pointer(h,c,out,size,sizeof(*out));require(root&&n&&n<=4096);separate(out,size,root,n);
    if(diagnostic){separate(diagnostic,sizeof(*diagnostic),out,size);separate(diagnostic,sizeof(*diagnostic),root,n);separate(diagnostic,sizeof(*diagnostic),c,sizeof(*c));}
    std::lock_guard<std::mutex> lock(h.state->mutex);require(!h.state->executing,PIXAURA_INFERENCE_BUSY);const auto available=h.state->ceiling-h.state->used.load();require(available>=65536,PIXAURA_INFERENCE_RESOURCE);
    pixaura_model_package* raw=nullptr;pixaura_model_diagnostic d{};const auto status=pixaura_model_package_verify(1,root,n,available,&raw,&d);
    if(status){if(diagnostic)*diagnostic=d;throw Error{PIXAURA_INFERENCE_UNVERIFIED_MODEL};}
    auto p=adopt(h,std::unique_ptr<pixaura_model_package>(raw));Entry e{};e.kind=1;e.package=std::move(p);const auto t=insert_locked(h,std::move(e));*out=t;if(diagnostic)*diagnostic=d;
});}
extern "C" int32_t pixaura_inference_prepare(pixaura_inference_context* c,const char* id,uint64_t n,const pixaura_inference_handle* package,pixaura_inference_handle* out,uint32_t size){return boundary([&]{
    const auto h=live(c);output_pointer(h,c,out,size,sizeof(*out));require(is_provider(id,n),PIXAURA_INFERENCE_PROVIDER_UNAVAILABLE);separate(out,size,id,n);separate(out,size,package,sizeof(*package));
    std::lock_guard<std::mutex> lock(h.state->mutex);const auto p=acquire_locked(h,package,1).package;const auto& value=*p->value;
    require(value.info.task==1,PIXAURA_INFERENCE_TASK_UNSUPPORTED);
    auto prepared_lease=std::make_shared<Lease>(*h.state,4096);
    auto prepared=find_provider(id,n)->prepare(value);require(prepared->input==value.binding.input&&prepared->output==value.binding.output,PIXAURA_INFERENCE_MODEL_UNSUPPORTED);
    auto model=std::make_shared<Model>();model->lease=std::move(prepared_lease);model->package=p;model->prepared=std::move(prepared);
    Entry e{};e.kind=2;e.model=std::move(model);*out=insert_locked(h,std::move(e));
});}
extern "C" int32_t pixaura_inference_model_query(pixaura_inference_context* c,const pixaura_inference_handle* model,pixaura_inference_model_metadata* out,uint32_t size){return boundary([&]{
    const auto h=live(c);output_pointer(h,c,out,size,sizeof(*out));separate(out,size,model,sizeof(*model));
    std::lock_guard<std::mutex> lock(h.state->mutex);const auto m=acquire_locked(h,model,2).model;
    pixaura_inference_model_metadata metadata{};metadata.version=1;metadata.struct_size=sizeof(metadata);metadata.task=m->package->value->info.task;
    metadata.input=tensor(m->prepared->input,m->package->value->binding.normalization);metadata.output=tensor(m->prepared->output,{{0,0,0,0,0,0}});
    require(metadata.input.bytes<=UINT64_MAX-metadata.output.bytes,PIXAURA_INFERENCE_RESOURCE);metadata.execution_bytes=metadata.input.bytes+metadata.output.bytes;
    std::memcpy(metadata.manifest_sha256,m->package->value->info.manifest_sha256,32);*out=metadata;
});}
extern "C" int32_t pixaura_inference_execute(pixaura_inference_context* c,const pixaura_inference_handle* model,const pixaura_inference_request* request,uint32_t request_size,const uint8_t* input,uint64_t input_bytes,pixaura_inference_handle* out,uint32_t size){return boundary([&]{
    const auto h=live(c);output_pointer(h,c,out,size,sizeof(*out));require(request&&request_size==sizeof(*request),PIXAURA_INFERENCE_UNSUPPORTED_ABI);
    require(request->version==1&&request->struct_size==sizeof(*request),PIXAURA_INFERENCE_UNSUPPORTED_ABI);require(request->task==1,PIXAURA_INFERENCE_TASK_UNSUPPORTED);
    require(request->generation&&request->pre_cancel<=1);require(input);separate(out,size,input,input_bytes);separate(out,size,request,sizeof(*request));separate(out,size,model,sizeof(*model));separate(input,input_bytes,c,sizeof(*c));
    std::shared_ptr<Model> m;{
        std::lock_guard<std::mutex> lock(h.state->mutex);m=acquire_locked(h,model,2).model;require(!h.state->executing,PIXAURA_INFERENCE_BUSY);
        require(request->generation>m->last,PIXAURA_INFERENCE_STALE_HANDLE);m->last=request->generation;
        require(!request->pre_cancel,PIXAURA_INFERENCE_CANCELLED);m->active=request->generation;m->cancellation.requested=false;h.state->executing=true;
    }
    struct Finish {Header h;std::shared_ptr<Model> m;~Finish(){std::lock_guard<std::mutex> lock(h.state->mutex);m->active=0;h.state->executing=false;}} finish{h,m};
    const auto expected=tensor(m->prepared->input,m->package->value->binding.normalization);const auto& t=request->input;
    require(t.version==1&&t.struct_size==sizeof(t)&&!t.reserved,PIXAURA_INFERENCE_UNSUPPORTED_ABI);
    require(t.type==1&&t.layout==1,PIXAURA_INFERENCE_INPUT_TYPE);require(t.rank==4&&!std::memcmp(t.dimensions,expected.dimensions,sizeof(t.dimensions)),PIXAURA_INFERENCE_INPUT_SHAPE);
    require(t.bytes==expected.bytes&&input_bytes==expected.bytes,PIXAURA_INFERENCE_INPUT_SHAPE);
    require(!std::memcmp(t.normalization,expected.normalization,sizeof(t.normalization)),PIXAURA_INFERENCE_NORMALIZATION);
    require(std::fegetround()==FE_TONEAREST&&std::numeric_limits<double>::is_iec559,PIXAURA_INFERENCE_INTERNAL);checkpoint(m,*h.state,evaluation::Checkpoint::admission);
    const auto count=static_cast<size_t>(expected.bytes/4),pixels=count/3;
    auto input_lease=std::make_shared<Lease>(*h.state,expected.bytes);auto owned=std::make_unique<float[]>(count);std::memcpy(owned.get(),input,static_cast<size_t>(expected.bytes));
    for(size_t i=0;i<count;i+=1024){checkpoint(m,*h.state,evaluation::Checkpoint::allocation);require(valid_probabilities(owned.get()+i,std::min<size_t>(1024,count-i)),PIXAURA_INFERENCE_INPUT_RANGE);}
    for(size_t i=0;i<count;++i){if(i%1024==0)checkpoint(m,*h.state,evaluation::Checkpoint::allocation);
        owned[i]=binary32_round((binary32_value(owned[i])*expected.normalization[(i/pixels)*2])/1000000.0);}
    auto result=std::make_shared<Output>();result->lease=std::make_shared<Lease>(*h.state,pixels*4);result->values=std::make_unique<float[]>(pixels);result->count=pixels;
    struct CheckState {std::shared_ptr<Model> m;State* state;} checked{m,h.state};
    const ExecutionControl execution{[](void* opaque,evaluation::Checkpoint phase){auto& state=*static_cast<CheckState*>(opaque);checkpoint(state.m,*state.state,phase);},&checked};
    m->prepared->run(owned.get(),result->values.get(),execution);
#ifdef PIXAURA_CERTIFICATION_TESTING
    if(h.state->output_fault)result->values[0]=std::numeric_limits<float>::quiet_NaN();
#endif
    require(valid_probabilities(result->values.get(),pixels),PIXAURA_INFERENCE_OUTPUT_INVALID);checkpoint(m,*h.state,evaluation::Checkpoint::publication);
    result->info.version=1;result->info.struct_size=sizeof(result->info);result->info.generation=request->generation;
    result->info.tensor=tensor(m->prepared->output,{{0,0,0,0,0,0}});std::memcpy(result->info.manifest_sha256,m->package->value->info.manifest_sha256,32);
    std::lock_guard<std::mutex> lock(h.state->mutex);require(!m->retired.load()&&!m->cancellation.requested.load(),PIXAURA_INFERENCE_CANCELLED);Entry e{};e.kind=3;e.output=std::move(result);*out=insert_locked(h,std::move(e));
});}
extern "C" int32_t pixaura_inference_cancel(pixaura_inference_context* c,const pixaura_inference_handle* model,uint64_t generation){return boundary([&]{const auto h=live(c);std::lock_guard<std::mutex> lock(h.state->mutex);
    auto m=acquire_locked(h,model,2).model;require(generation&&m->active==generation,PIXAURA_INFERENCE_STALE_HANDLE);m->cancellation.requested=true;});}
extern "C" int32_t pixaura_inference_output_info(pixaura_inference_context* c,const pixaura_inference_handle* output,pixaura_inference_output* out,uint32_t size){return boundary([&]{const auto h=live(c);output_pointer(h,c,out,size,sizeof(*out));separate(out,size,output,sizeof(*output));
    std::lock_guard<std::mutex> lock(h.state->mutex);*out=acquire_locked(h,output,3).output->info;});}
extern "C" int32_t pixaura_inference_output_copy(pixaura_inference_context* c,const pixaura_inference_handle* output,uint8_t* out,uint64_t capacity){return boundary([&]{const auto h=live(c);require(out);separate(out,capacity,c,sizeof(*c));separate(out,capacity,output,sizeof(*output));
    std::shared_ptr<const Output> value;{std::lock_guard<std::mutex> lock(h.state->mutex);value=acquire_locked(h,output,3).output;}
    require(capacity==value->count*4,PIXAURA_INFERENCE_BUFFER_TOO_SMALL);std::memcpy(out,value->values.get(),static_cast<size_t>(capacity));});}
extern "C" int32_t pixaura_inference_release(pixaura_inference_context* c,const pixaura_inference_handle* token){return boundary([&]{const auto h=live(c);std::lock_guard<std::mutex> lock(h.state->mutex);const auto entry=acquire_locked(h,token,0);
    if(entry.model){entry.model->retired=true;entry.model->cancellation.requested=true;}h.state->entries.erase(token->serial);});}
extern "C" int32_t pixaura_inference_resources_query(pixaura_inference_context* c,pixaura_inference_resources* out,uint32_t size){return boundary([&]{const auto h=live(c);output_pointer(h,c,out,size,sizeof(*out));std::lock_guard<std::mutex> lock(h.state->mutex);
    *out={1,sizeof(*out),static_cast<uint32_t>(h.state->entries.size()),h.state->executing?1u:0u,h.state->ceiling,h.state->used.load()};});}
#ifdef PIXAURA_CERTIFICATION_TESTING
#include "../tests/inference_boundary.hpp"
extern "C" PIXAURA_API int32_t pixaura_inference_test_boundary(){return boundary([&]{require(pixaura::inference::certification()==0,PIXAURA_INFERENCE_INTERNAL);});}
#endif
