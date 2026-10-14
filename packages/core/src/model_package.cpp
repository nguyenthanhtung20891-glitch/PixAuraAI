#include "model_package_internal.hpp"
#include "sha256.hpp"
#include <sodium.h>
#include <algorithm>
#include <cstring>
#include <memory>
#include <mutex>
namespace pixaura::model {
namespace {
bool utf8(const std::string& s) {
    for(size_t i=0;i<s.size();) {
        const auto c=static_cast<uint8_t>(s[i++]); if(c<128){if(c<32||c==127)return false;continue;}
        uint32_t code=0,min=0;unsigned n=0;
        if(c>=0xc2&&c<=0xdf){code=c&31;min=128;n=1;}
        else if(c>=0xe0&&c<=0xef){code=c&15;min=2048;n=2;}
        else if(c>=0xf0&&c<=0xf4){code=c&7;min=65536;n=3;}else return false;
        while(n--){if(i>=s.size())return false;const auto d=static_cast<uint8_t>(s[i++]);if((d&0xc0)!=0x80)return false;code=(code<<6)|(d&63);}
        if(code<min||code>0x10ffff||(code>=0xd800&&code<=0xdfff))return false;
    }return true;
}
struct Reader {
    const std::vector<uint8_t>& b;size_t at=0;
    uint64_t integer(size_t n){need(n<=b.size()-at,PIXAURA_MODEL_PARSE);uint64_t v=0;while(n--)v=(v<<8)|b[at++];return v;}
    uint32_t u32(){return static_cast<uint32_t>(integer(4));}
    std::string text(size_t max){const auto n=integer(2);need(n<=max&&n<=b.size()-at,PIXAURA_MODEL_PARSE);std::string s(reinterpret_cast<const char*>(b.data()+at),static_cast<size_t>(n));at+=static_cast<size_t>(n);need(utf8(s),PIXAURA_MODEL_NONCANONICAL);return s;}
    std::array<uint8_t,32> digest(){need(32<=b.size()-at,PIXAURA_MODEL_PARSE);std::array<uint8_t,32>d{};std::copy_n(b.data()+at,32,d.begin());at+=32;return d;}
};
std::array<uint8_t,32> hash(const std::vector<uint8_t>& b) {
    storage::Sha256 h;h.update(b.data(),b.size());const auto hex=h.finish();std::array<uint8_t,32>d{};
    auto value=[](char c){return c<='9'?c-'0':c-'a'+10;};
    for(size_t i=0;i<32;++i){d[i]=static_cast<uint8_t>((value(hex[i*2])<<4)|value(hex[i*2+1]));}
    return d;
}
struct Tensor {uint32_t rank,type,layout;std::array<uint32_t,4> dim;};
Tensor tensor(Reader& r){Tensor t{r.u32(),r.u32(),r.u32(),{}};for(auto& d:t.dim)d=r.u32();return t;}
void valid_tensor(const Tensor& t,uint32_t channels){
    need(t.rank==4&&t.type==1&&t.layout==1&&t.dim[0]==1&&t.dim[1]==channels,PIXAURA_MODEL_TENSOR);
    uint64_t elements=1;for(auto d:t.dim){need(d&&d<=1024&&elements<=4194304/d,PIXAURA_MODEL_TENSOR);elements*=d;}
}
struct Payload {std::string name;uint32_t format;uint64_t size;std::array<uint8_t,32> digest;};
struct Provider {uint32_t id,runtime,api,file,opset;std::array<uint8_t,32> source;std::vector<uint32_t> ops;};
struct Manifest {
    uint32_t algorithm,signature_version,key,domain,generation,signature_encoding,model_version,task,tier,threads,budget,accel,validation;
    std::string signature_location;
    uint64_t memory;std::string id;Tensor input,output;std::array<uint32_t,8> ranges;std::array<uint32_t,6> normalization;
    std::string background,foreground,license,provenance,attribution;uint32_t use,review,attribution_required;
    std::vector<Provider> providers;std::vector<Payload> payloads;
};
Manifest parse(const std::vector<uint8_t>& b) {
    need(b.size()<=65536,PIXAURA_MODEL_MANIFEST_LIMIT_REASON);Reader r{b};
    need(r.u32()==0x50584d50,PIXAURA_MODEL_PARSE);need(r.u32()==1,PIXAURA_MODEL_VERSION);
    need(r.u32()==b.size(),PIXAURA_MODEL_NONCANONICAL);Manifest m{};
    m.algorithm=r.u32();m.signature_version=r.u32();m.key=r.u32();m.domain=r.u32();m.generation=r.u32();
    m.signature_location=r.text(64);m.signature_encoding=r.u32();
    need(m.signature_location=="signature.pxs"&&m.signature_encoding==1,PIXAURA_MODEL_SIGNATURE_VERSION);
    m.id=r.text(128);m.model_version=r.u32();m.task=r.u32();m.input=tensor(r);m.output=tensor(r);
    for(auto& x:m.ranges){x=r.u32();}
    for(auto& x:m.normalization){x=r.u32();}
    m.background=r.text(128);m.foreground=r.text(128);
    const auto providers=r.u32();need(providers&&providers<=3,PIXAURA_MODEL_PROVIDER);
    for(uint32_t i=0;i<providers;++i){Provider p{};p.id=r.u32();p.runtime=r.u32();p.api=r.u32();p.file=r.u32();p.opset=r.u32();p.source=r.digest();
        const auto count=r.u32();need(count&&count<=64,PIXAURA_MODEL_PROVIDER);for(uint32_t j=0;j<count;++j)p.ops.push_back(r.u32());m.providers.push_back(std::move(p));}
    m.tier=r.u32();m.threads=r.u32();m.memory=r.integer(8);m.budget=r.u32();m.accel=r.u32();m.validation=r.u32();
    m.license=r.text(128);m.provenance=r.text(512);m.use=r.u32();m.review=r.u32();m.attribution_required=r.u32();m.attribution=r.text(1024);
    const auto count=r.u32();need(count&&count<=8,PIXAURA_MODEL_COUNT);
    std::string last;
    for(uint32_t i=0;i<count;++i){Payload p{};p.name=r.text(64);need(leaf(p.name)&&p.name!="manifest.pxm"&&p.name!="signature.pxs",PIXAURA_MODEL_PATH,i);
        need(last!=p.name,PIXAURA_MODEL_DUPLICATE,i);need(last.empty()||last<p.name,PIXAURA_MODEL_NONCANONICAL,i);last=p.name;
        p.format=r.u32();p.size=r.integer(8);p.digest=r.digest();m.payloads.push_back(std::move(p));}
    need(r.at==b.size(),PIXAURA_MODEL_NONCANONICAL);return m;
}
void semantics(const Manifest& m){
    need(!m.id.empty()&&m.model_version,PIXAURA_MODEL_PARSE);need(m.task==1,PIXAURA_MODEL_TASK);
    valid_tensor(m.input,3);valid_tensor(m.output,1);need(m.input.dim[2]==m.output.dim[2]&&m.input.dim[3]==m.output.dim[3],PIXAURA_MODEL_TENSOR);
    const std::array<uint32_t,8> ranges{{0,1000000,0,1000000,0,1000000,0,1000000}};need(m.ranges==ranges,PIXAURA_MODEL_TENSOR);
    for(size_t i=0;i<6;i+=2){need(m.normalization[i]>0&&m.normalization[i]<=1000000&&m.normalization[i+1]==0,PIXAURA_MODEL_TENSOR);}
    need(m.background=="background"&&m.foreground=="foreground",PIXAURA_MODEL_TENSOR);
    bool cpu=false;uint32_t previous=0;
    for(const auto& p:m.providers){need(p.id>previous&&p.id<=3&&p.runtime&&p.api&&p.file<m.payloads.size()&&p.opset==1,PIXAURA_MODEL_PROVIDER);previous=p.id;
        const auto& payload=m.payloads[p.file];need(payload.format==p.id,PIXAURA_MODEL_PROVIDER);
        uint32_t op=0;for(auto x:p.ops){need(x>op&&x<=16,PIXAURA_MODEL_PROVIDER);op=x;}
        if(p.id==1){cpu=true;need(p.source==payload.digest,PIXAURA_MODEL_PROVIDER);}
        else need(p.source==m.providers.front().source,PIXAURA_MODEL_PROVIDER);
    }need(cpu&&m.providers.front().id==1,PIXAURA_MODEL_PROVIDER);
    need(m.tier<=2&&m.threads&&m.threads<=1024&&m.memory&&m.memory<=268435456&&m.budget<=2&&m.accel<=1&&m.validation<=2,PIXAURA_MODEL_RESOURCE);
    need(m.use==1&&m.review==1&&m.attribution_required<=1&&!m.provenance.empty()&&(!m.attribution_required||!m.attribution.empty()),PIXAURA_MODEL_LICENSE);
    need(m.license=="ISC"||m.license=="MIT"||m.license=="BSD-2-Clause"||m.license=="BSD-3-Clause"||m.license=="Apache-2.0"||m.license=="CC0-1.0",PIXAURA_MODEL_LICENSE);
    for(const auto& p:m.payloads){need(p.format>=1&&p.format<=3,PIXAURA_MODEL_PROVIDER);const auto suffix=p.format==1?".onnx":(p.format==2?".mlmodel":".tflite");const std::string ext=suffix;need(p.name.size()>ext.size()&&p.name.compare(p.name.size()-ext.size(),ext.size(),ext)==0,PIXAURA_MODEL_PROVIDER);}
}
}
int32_t verify_owned(uint32_t version,std::vector<File> files,const Trust& trust,uint64_t memory_limit,pixaura_model_package** output,pixaura_model_diagnostic* diagnostic){
    auto fail=[&](uint32_t r,uint32_t i=0){if(diagnostic)*diagnostic={1,r,i,0};return static_cast<int32_t>(r);};
    try {
        need(output&&memory_limit>=65536&&memory_limit<=268435456,PIXAURA_MODEL_ARGUMENT);need(version==1,PIXAURA_MODEL_VERSION);
        need(files.size()<=10,PIXAURA_MODEL_COUNT);uint64_t total=0;
        std::sort(files.begin(),files.end(),[](const File&a,const File&b){return a.name<b.name;});
        for(size_t i=0;i<files.size();++i){need(leaf(files[i].name),PIXAURA_MODEL_PATH,static_cast<uint32_t>(i));need(!i||files[i-1].name!=files[i].name,PIXAURA_MODEL_DUPLICATE);
            const auto n=files[i].bytes.size();need(n<=memory_limit-65536-total,PIXAURA_MODEL_RESOURCE);total+=n;}
        auto file=[&](const std::string& name)->const File*{for(const auto& f:files)if(f.name==name)return &f;return nullptr;};
        const auto* manifest=file("manifest.pxm");need(manifest,PIXAURA_MODEL_PAYLOAD_MISSING);const auto m=parse(manifest->bytes);
        need(m.algorithm==1,PIXAURA_MODEL_ALGORITHM);need(m.signature_version==1,PIXAURA_MODEL_SIGNATURE_VERSION);
        need(trust.version==1&&trust.count<=16&&trust.generation>=m.generation,PIXAURA_MODEL_TRUST_VERSION);
        need(m.domain==trust.purpose,PIXAURA_MODEL_TRUST_DOMAIN);
        const Key* key=nullptr;for(size_t i=0;i<trust.count;++i)if(trust.keys[i].id==m.key){need(!key,PIXAURA_MODEL_DUPLICATE);key=&trust.keys[i];}
        need(key,PIXAURA_MODEL_UNKNOWN_KEY);need(key->purpose==trust.purpose&&key->activation<=trust.generation&&m.model_version>=key->minimum_model_version,PIXAURA_MODEL_TRUST_VERSION);
        static std::once_flag initialized;static int init=-1;std::call_once(initialized,[]{init=sodium_init();});need(init>=0,PIXAURA_MODEL_RESOURCE);
        need(crypto_core_ed25519_is_valid_point(key->bytes.data())==1,PIXAURA_MODEL_KEY_INVALID);
        const auto* signature=file("signature.pxs");need(signature,PIXAURA_MODEL_PAYLOAD_MISSING);need(signature->bytes.size()==80,PIXAURA_MODEL_SIGNATURE_SIZE);
        Reader sr{signature->bytes};need(sr.u32()==0x50585347,PIXAURA_MODEL_PARSE);need(sr.u32()==1,PIXAURA_MODEL_SIGNATURE_VERSION);need(sr.u32()==1,PIXAURA_MODEL_ALGORITHM);need(sr.u32()==m.key,PIXAURA_MODEL_UNKNOWN_KEY);
        need(crypto_sign_ed25519_verify_detached(signature->bytes.data()+16,manifest->bytes.data(),manifest->bytes.size(),key->bytes.data())==0,PIXAURA_MODEL_SIGNATURE_INVALID);
        need(files.size()<=m.payloads.size()+2,PIXAURA_MODEL_PAYLOAD_UNEXPECTED);
        for(size_t i=0;i<m.payloads.size();++i){need(file(m.payloads[i].name),PIXAURA_MODEL_PAYLOAD_MISSING,static_cast<uint32_t>(i));}
        need(files.size()==m.payloads.size()+2,PIXAURA_MODEL_PAYLOAD_UNEXPECTED);
        uint64_t payload_total=0;
        for(size_t i=0;i<m.payloads.size();++i){const auto& p=m.payloads[i];const auto* f=file(p.name);need(f,PIXAURA_MODEL_PAYLOAD_MISSING,static_cast<uint32_t>(i));
            need(p.size&&p.size<=PIXAURA_MODEL_FILE_LIMIT&&p.size==f->bytes.size(),PIXAURA_MODEL_SIZE,static_cast<uint32_t>(i));need(p.size<=PIXAURA_MODEL_TOTAL_LIMIT-payload_total,PIXAURA_MODEL_SIZE);payload_total+=p.size;
        }
        for(size_t i=0;i<m.payloads.size();++i){need(hash(file(m.payloads[i].name)->bytes)==m.payloads[i].digest,PIXAURA_MODEL_DIGEST,static_cast<uint32_t>(i));}
        semantics(m);
        auto package=std::make_unique<pixaura_model_package>();package->info={1,m.model_version,m.task,static_cast<uint32_t>(m.payloads.size()),m.key,trust.generation,payload_total,m.memory,{}};
        package->binding.input=m.input.dim;package->binding.output=m.output.dim;
        package->binding.normalization=m.normalization;
        const auto& cpu=m.providers.front();package->binding.cpu_payload=m.payloads[cpu.file].name;
        package->binding.runtime_floor=cpu.runtime;package->binding.api_floor=cpu.api;
        package->binding.operator_set=cpu.opset;package->binding.operators=cpu.ops;
        const auto digest=hash(manifest->bytes);std::copy(digest.begin(),digest.end(),package->info.manifest_sha256);package->files=std::move(files);
        *output=package.release();return fail(PIXAURA_MODEL_OK);
    }catch(const Failure& f){return fail(f.reason,f.ordinal);}catch(...){return fail(PIXAURA_MODEL_RESOURCE);}
}
}
extern "C" int32_t pixaura_model_package_verify(uint32_t version,const char* root,uint64_t root_bytes,uint64_t memory_limit,pixaura_model_package** output,pixaura_model_diagnostic* diagnostic){
    try {
        pixaura::model::need(version==1,PIXAURA_MODEL_VERSION);pixaura::model::need(output&&memory_limit>=65536&&memory_limit<=268435456,PIXAURA_MODEL_ARGUMENT);
        return pixaura::model::verify_owned(version,pixaura::model::read_directory(root,root_bytes,memory_limit-65536),{1,1,1,nullptr,0},memory_limit,output,diagnostic);
    }catch(const pixaura::model::Failure& f){if(diagnostic)*diagnostic={1,f.reason,f.ordinal,0};return static_cast<int32_t>(f.reason);}catch(...){if(diagnostic)*diagnostic={1,PIXAURA_MODEL_RESOURCE,0,0};return PIXAURA_MODEL_RESOURCE;}
}
extern "C" int32_t pixaura_model_package_info(uint32_t version,const pixaura_model_package* p,pixaura_model_info* out){if(version!=1)return PIXAURA_MODEL_VERSION;if(!p||!out)return PIXAURA_MODEL_ARGUMENT;*out=p->info;return 0;}
extern "C" void pixaura_model_package_release(pixaura_model_package* p){delete p;}
#ifdef PIXAURA_CERTIFICATION_TESTING
#include "../tests/model_package_boundary.hpp"
extern "C" PIXAURA_API int32_t pixaura_model_test_boundary(){return pixaura::model::boundary();}
#endif
