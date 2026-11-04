#include "pixaura/export.h"
#include "decode_codec.h"
#include "document.hpp"
#include <cstring>
#include <mutex>
#include <vector>
namespace {
static_assert(sizeof(pixaura_export_request)==72,"export request ABI");
struct ExportLive { Service* service=nullptr; pixaura_decode_handle token{}; };
std::mutex& export_slots_mu(){static std::mutex gate;return gate;}
std::vector<ExportLive>& export_slots(){static std::vector<ExportLive> slots;return slots;}
void export_publish(Service& s,const pixaura_decode_handle& token){
    std::lock_guard<std::mutex> lock(export_slots_mu());
    export_slots().push_back(ExportLive{&s,token});
}
void export_retract(Service& s){
    std::lock_guard<std::mutex> lock(export_slots_mu());
    auto& slots=export_slots();
    for(auto it=slots.begin();it!=slots.end();++it){if(it->service==&s){slots.erase(it);return;}}
}
bool export_hex(const char* value){for(int i=0;i<32;++i){const auto c=value[i];if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')))return false;}return true;}
void export_fail(uint32_t* state,uint32_t terminal,int32_t code){if(state)*state=terminal;throw Failure{code};}
}
extern "C" int32_t pixaura_export_bind(uint32_t version,pixaura_editor_context* c,pixaura_export_request* request){return attempt([&]{
    require(version==1,2);require(request,1);
    execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{request,sizeof(*request),alignof(pixaura_export_request)}});
    auto& s=owner(c);if(!s.gate.try_lock())export_fail(nullptr,PIXAURA_EXPORT_FAILED,8);
    struct Unlock{std::mutex& gate;~Unlock(){gate.unlock();}} unlock{s.gate};
    s.ready();require(!s.agent_active&&!s.selected&&!s.proposal.serial,8);
    std::memset(request,0,sizeof(*request));
    request->version=1;request->struct_size=sizeof(*request);request->format=PIXAURA_EXPORT_PNG;request->epoch=s.loaded.epoch;
    const auto id=s.loaded.snapshot->current().text();require(id.size()==32,14);std::memcpy(request->revision_id,id.data(),32);
});}
extern "C" int32_t pixaura_export_cancel(pixaura_editor_context* c){return attempt([&]{
    auto& s=owner(c);std::lock_guard<std::mutex> slots(export_slots_mu());
    for(auto& slot:export_slots()){if(slot.service==&s&&slot.token.serial){check(pixaura_cancel_signal(&s.raster,&slot.token));return;}}
    require(false,1);
});}
extern "C" int32_t pixaura_export_encode(uint32_t version,pixaura_editor_context* c,const pixaura_export_request* request,
    const pixaura_decode_handle* cancellation,uint8_t* output,uint64_t capacity,uint64_t* bytes,uint32_t* width,uint32_t* height,uint32_t* state){return attempt([&]{
    require(version==1,2);require(request&&output&&bytes&&width&&height&&state,1);
    *state=PIXAURA_EXPORT_FAILED;
    struct Terminal{uint32_t* state;bool commit=false;~Terminal(){if(!commit&&state&&*state<PIXAURA_EXPORT_FAILED)*state=PIXAURA_EXPORT_FAILED;}} terminal{state};
    require(capacity>=64&&capacity<=UINT64_C(67108864),11);
    execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{request,sizeof(*request),alignof(pixaura_export_request)},
        {output,capacity},{bytes,sizeof(*bytes),alignof(uint64_t)},{width,sizeof(*width),alignof(uint32_t)},{height,sizeof(*height),alignof(uint32_t)},{state,sizeof(*state),alignof(uint32_t)}});
    if(cancellation)execution_ranges({{cancellation,sizeof(*cancellation),alignof(pixaura_decode_handle)}});
    auto& s=owner(c);if(!s.gate.try_lock())export_fail(state,PIXAURA_EXPORT_FAILED,8);
    struct Hold{std::mutex& gate;~Hold(){gate.unlock();}} hold{s.gate};
    s.ready();*state=PIXAURA_EXPORT_VALIDATING;
    require(!s.agent_active&&!s.selected&&!s.proposal.serial,8);
    require(!s.review_preview.active&&!s.review_preview.before.serial&&!s.review_preview.after.serial,8);
    require(request->version==1&&request->struct_size==sizeof(*request)&&request->reserved==0,1);
    require(request->format==PIXAURA_EXPORT_JPEG||request->format==PIXAURA_EXPORT_PNG,1);
    require(request->format==PIXAURA_EXPORT_PNG?request->quality==0:request->quality>=1&&request->quality<=100,1);
    require(request->mode==PIXAURA_EXPORT_EXACT||request->mode==PIXAURA_EXPORT_FIT,1);
    if(request->mode==PIXAURA_EXPORT_EXACT)require(request->max_width==0&&request->max_height==0,1);
    else require(request->max_width&&request->max_height,1);
    require(export_hex(request->revision_id),1);
    {std::lock_guard<std::mutex> slots(export_slots_mu());for(const auto& slot:export_slots()){if(slot.service==&s)export_fail(state,PIXAURA_EXPORT_FAILED,8);}}
    const auto id=std::string_view(request->revision_id,32);
    if(request->epoch!=s.loaded.epoch||!result_find(*s.loaded.snapshot,id))export_fail(state,PIXAURA_EXPORT_STALE,9);
    const auto operations=good(document::replay(*s.loaded.snapshot,document::Id::parse(id)));
    const auto projection=operations.empty()?document::String():good(document::serialize_evaluation(operations));
    *state=PIXAURA_EXPORT_RENDERING;
    pixaura_decode_handle evaluated{},preview{},token{};
    struct Cleanup{Service& s;pixaura_decode_handle* evaluated;pixaura_decode_handle* preview;pixaura_decode_handle* token;~Cleanup(){
        export_retract(s);
        if(token->serial)(void)pixaura_cancel_release(&s.raster,token);
        if(preview->serial)(void)pixaura_preview_release(&s.raster,preview);
        if(evaluated->serial)(void)pixaura_decode_release(&s.raster,evaluated);
    }} cleanup{s,&evaluated,&preview,&token};
    const pixaura_decode_handle* watched=cancellation;
    if(!cancellation){check(pixaura_cancel_create(&s.raster,1,&token));export_publish(s,token);watched=&token;}
    else export_publish(s,*cancellation);
    const auto rendered=[&]{if(operations.empty())return 0;
        return pixaura_working_evaluate_cancel(&s.raster,&s.original,1,byte_pointer(projection),projection.size(),&s.limits,watched,&evaluated);};
    if(const auto code=rendered())export_fail(state,code==13?PIXAURA_EXPORT_CANCELLED:PIXAURA_EXPORT_FAILED,code);
    const pixaura_preview_request fit{1,sizeof(fit),request->mode,request->max_width,request->max_height,0};
    if(const auto code=pixaura_preview_render(&s.raster,operations.empty()?&s.original:&evaluated,&fit,watched,&preview))
        export_fail(state,code==13?PIXAURA_EXPORT_CANCELLED:PIXAURA_EXPORT_FAILED,code);
    pixaura_preview_metadata metadata{};check(pixaura_preview_query(&s.raster,&preview,&metadata));
    require(metadata.width&&metadata.height&&metadata.image_bytes==static_cast<uint64_t>(metadata.width)*metadata.height*4,14);
    if(request->epoch!=s.loaded.epoch||!result_find(*s.loaded.snapshot,id))export_fail(state,PIXAURA_EXPORT_STALE,9);
    *state=PIXAURA_EXPORT_ENCODING;
    std::vector<uint8_t> encoded(static_cast<std::size_t>(capacity));size_t written=0;
    std::vector<uint8_t> pixels(static_cast<std::size_t>(metadata.image_bytes));
    check(pixaura_preview_copy(&s.raster,&preview,0,pixels.data(),metadata.image_bytes));
    const auto encoded_code=request->format==PIXAURA_EXPORT_JPEG
        ?pixaura_codec_encode_jpeg(pixels.data(),metadata.width,metadata.height,static_cast<int>(request->quality),encoded.data(),encoded.size(),&written,UINT64_C(33554432))
        :pixaura_codec_encode_png(pixels.data(),metadata.width,metadata.height,encoded.data(),encoded.size(),&written,UINT64_C(33554432));
    if(encoded_code)export_fail(state,PIXAURA_EXPORT_FAILED,encoded_code==8?11:encoded_code);
    if(request->epoch!=s.loaded.epoch||!result_find(*s.loaded.snapshot,id))export_fail(state,PIXAURA_EXPORT_STALE,9);
    require(written&&written<=capacity,11);
    std::memcpy(output,encoded.data(),written);
    *bytes=written;*width=metadata.width;*height=metadata.height;*state=PIXAURA_EXPORT_SUCCEEDED;terminal.commit=true;
});}
