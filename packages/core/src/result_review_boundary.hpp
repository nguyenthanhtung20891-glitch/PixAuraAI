#include "pixaura/result_review.h"
#include "document.hpp"
#include <cstring>
#include <string>
#include <variant>
namespace {
static_assert(sizeof(pixaura_result_review)==152,"result review ABI");
bool result_blank(const char* value){for(int i=0;i<32;++i){if(value[i])return false;}return true;}
bool result_hex(const char* value){for(int i=0;i<32;++i){const auto c=value[i];if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')))return false;}return true;}
bool result_hex_n(const uint8_t* value,std::size_t n){for(std::size_t i=0;i<n;++i){const auto c=static_cast<char>(value[i]);if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')))return false;}return true;}
void result_copy(char (&out)[32],std::string_view value){require(value.size()==32,14);std::memcpy(out,value.data(),32);}
bool result_same(const document::String& text,std::string_view id){
    return text.size()==id.size()&&std::memcmp(text.data(),id.data(),id.size())==0;
}
const document::Revision* result_find(const document::ImageDocument& doc,std::string_view id){
    for(const auto& revision:doc.revisions()){if(result_same(revision.id.text(),id))return &revision;}
    return nullptr;
}
bool result_geometry(const document::EditOperation& op){
    return std::holds_alternative<document::Crop>(op.parameters)||std::holds_alternative<document::Rotate>(op.parameters);
}
std::string result_geometry_text(const document::ImageDocument& doc,std::string_view id){
    std::string text;if(id.size()!=32)return text;
    for(const auto& op:good(document::replay(doc,document::Id::parse(id)))){
        if(!result_geometry(op)){continue;}
        text+=std::string(op.type);text.push_back(' ');
        if(const auto* crop=std::get_if<document::Crop>(&op.parameters)){
            text+=std::to_string(crop->x_ppm)+" "+std::to_string(crop->y_ppm)+" "+std::to_string(crop->width_ppm)+" "+std::to_string(crop->height_ppm);
        }
        if(const auto* turn=std::get_if<document::Rotate>(&op.parameters)){text+=std::to_string(turn->quarter_turns);}
        text.push_back('\n');
    }
    return text;
}
pixaura_result_review result_describe(Service& s,const pixaura_agent_execution_receipt* receipt){
    s.ready();const auto& doc=*s.loaded.snapshot;pixaura_result_review out{};out.api_version=1;out.struct_size=sizeof(out);
    result_copy(out.document_id,doc.identity().document.text());result_copy(out.live_revision,doc.current().text());
    out.revision_count=static_cast<uint32_t>(doc.revisions().size());
    const auto* live=result_find(doc,doc.current().text());require(live,14);
    if(live->parent){result_copy(out.previous_revision,live->parent->text());out.flags|=PIXAURA_RESULT_HAS_PREVIOUS;}
    if(!s.selected&&live->parent)out.flags|=PIXAURA_RESULT_CAN_UNDO;
    if(!s.selected&&!doc.redo().empty())out.flags|=PIXAURA_RESULT_CAN_REDO;
    if(!receipt){out.state=out.revision_count>1?PIXAURA_RESULT_UNCONFIRMED:PIXAURA_RESULT_NONE;return out;}
    require(receipt->api_version==1&&receipt->struct_size==sizeof(*receipt)&&receipt->approval_consumed==1&&receipt->reserved==0,1);
    require(result_hex_n(receipt->plan_sha256,64)&&result_hex_n(receipt->source_revision,32)&&result_hex_n(receipt->resulting_revision,32),1);
    const auto source=std::string_view(reinterpret_cast<const char*>(receipt->source_revision),32);
    const auto produced=std::string_view(reinterpret_cast<const char*>(receipt->resulting_revision),32);
    require(source!=produced,1);
    const auto* published=result_find(doc,produced);
    if(!published){std::memset(out.previous_revision,0,32);if(live->parent){result_copy(out.previous_revision,live->parent->text());out.flags|=PIXAURA_RESULT_HAS_PREVIOUS;}else out.flags&=~PIXAURA_RESULT_HAS_PREVIOUS;out.state=PIXAURA_RESULT_ABSENT;return out;}
    out.flags|=PIXAURA_RESULT_PUBLICATION_VISIBLE;result_copy(out.result_revision,published->id.text());
    std::memset(out.previous_revision,0,32);out.flags&=~PIXAURA_RESULT_HAS_PREVIOUS;
    if(published->parent){result_copy(out.previous_revision,published->parent->text());out.flags|=PIXAURA_RESULT_HAS_PREVIOUS;}
    const bool bound=published->parent&&result_same(published->parent->text(),source)&&result_same(doc.current().text(),produced);
    out.state=bound?PIXAURA_RESULT_CURRENT:PIXAURA_RESULT_STALE;
    if((out.flags&PIXAURA_RESULT_HAS_PREVIOUS)&&!result_blank(out.result_revision)){
        out.geometry_changed=result_geometry_text(doc,std::string_view(out.previous_revision,32))!=result_geometry_text(doc,std::string_view(out.result_revision,32));
    }
    return out;
}
void result_match(Service& s,const pixaura_result_review& binding){
    require(binding.api_version==1&&binding.struct_size==sizeof(binding)&&binding.state<=PIXAURA_RESULT_ABSENT,1);
    const auto fresh=result_describe(s,nullptr);const auto& doc=*s.loaded.snapshot;
    require(std::memcmp(binding.document_id,fresh.document_id,32)==0,9);
    require(std::memcmp(binding.live_revision,fresh.live_revision,32)==0,9);
    require(binding.revision_count==fresh.revision_count,9);
    require((binding.flags&PIXAURA_RESULT_CAN_UNDO)==(fresh.flags&PIXAURA_RESULT_CAN_UNDO),9);
    require((binding.flags&PIXAURA_RESULT_CAN_REDO)==(fresh.flags&PIXAURA_RESULT_CAN_REDO),9);
    if(binding.state==PIXAURA_RESULT_CURRENT||binding.state==PIXAURA_RESULT_STALE){
        require(!result_blank(binding.result_revision)&&result_hex(binding.result_revision),9);
        const auto* published=result_find(doc,std::string_view(binding.result_revision,32));require(published,9);
        require((binding.flags&PIXAURA_RESULT_PUBLICATION_VISIBLE)!=0,9);
        if(published->parent)require((binding.flags&PIXAURA_RESULT_HAS_PREVIOUS)&&std::memcmp(binding.previous_revision,published->parent->text().data(),32)==0,9);
        else require((binding.flags&PIXAURA_RESULT_HAS_PREVIOUS)==0&&result_blank(binding.previous_revision),9);
        require((doc.current().text()==published->id.text())==(binding.state==PIXAURA_RESULT_CURRENT),9);
    }else{
        require(result_blank(binding.result_revision)&&(binding.flags&PIXAURA_RESULT_PUBLICATION_VISIBLE)==0,9);
        require(std::memcmp(binding.previous_revision,fresh.previous_revision,32)==0,9);
        require((binding.flags&PIXAURA_RESULT_HAS_PREVIOUS)==(fresh.flags&PIXAURA_RESULT_HAS_PREVIOUS),9);
        require(binding.state==(fresh.revision_count>1?PIXAURA_RESULT_UNCONFIRMED:PIXAURA_RESULT_NONE)||binding.state==PIXAURA_RESULT_ABSENT,9);
    }
    uint32_t geometry=0;
    if((binding.flags&PIXAURA_RESULT_HAS_PREVIOUS)&&!result_blank(binding.result_revision)){
        geometry=result_geometry_text(doc,std::string_view(binding.previous_revision,32))!=result_geometry_text(doc,std::string_view(binding.result_revision,32));
    }
    require(binding.geometry_changed==geometry,9);
}
}
extern "C" int32_t pixaura_result_review_query(uint32_t version,pixaura_editor_context* c,
 const pixaura_agent_execution_receipt* receipt,pixaura_result_review* out){return attempt([&]{
    require(version==1,2);
    if(receipt)execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{receipt,sizeof(*receipt),alignof(pixaura_agent_execution_receipt)},{out,sizeof(*out),alignof(pixaura_result_review)}});
    else execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{out,sizeof(*out),alignof(pixaura_result_review)}});
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);const auto described=result_describe(s,receipt);*out=described;
});}
extern "C" int32_t pixaura_result_review_render(uint32_t version,pixaura_editor_context* c,
 const pixaura_result_review* binding,const char* revision_id,uint32_t max_width,uint32_t max_height,
 uint8_t* output,uint64_t capacity,uint32_t* width,uint32_t* height){return attempt([&]{
    require(version==1,2);require(max_width&&max_height&&max_width<=1024&&max_height<=1024,1);
    require(capacity>=static_cast<uint64_t>(max_width)*max_height*4,11);
    // The revision id may alias the binding. Copy it before the disjoint-range check.
    execution_ranges({{revision_id,32}});
    char revision_bytes[32];std::memcpy(revision_bytes,revision_id,32);revision_id=revision_bytes;
    execution_ranges({{c,sizeof(*c),alignof(pixaura_editor_context)},{binding,sizeof(*binding),alignof(pixaura_result_review)},
        {output,capacity},{width,sizeof(*width),alignof(uint32_t)},{height,sizeof(*height),alignof(uint32_t)}});
    require(result_hex(revision_id),1);
    auto& s=owner(c);std::lock_guard<std::mutex> lock(s.gate);s.ready();
    require(!s.agent_active&&!s.selected&&!s.proposal.serial,8);
    require(!s.review_preview.active&&!s.review_preview.before.serial&&!s.review_preview.after.serial,8);
    result_match(s,*binding);
    const auto id=std::string_view(revision_id,32);const auto& doc=*s.loaded.snapshot;require(result_find(doc,id),3);
    const bool listed=(!result_blank(binding->previous_revision)&&id==std::string_view(binding->previous_revision,32))||
        (!result_blank(binding->result_revision)&&id==std::string_view(binding->result_revision,32))||
        id==std::string_view(binding->live_revision,32);
    require(listed,3);
    const auto operations=good(document::replay(doc,document::Id::parse(id)));
    const auto projection=operations.empty()?document::String():good(document::serialize_evaluation(operations));
    pixaura_decode_handle evaluated{},preview{};
    struct Cleanup{pixaura_decode_context* raster;pixaura_decode_handle* evaluated;pixaura_decode_handle* preview;~Cleanup(){
        if(preview->serial){(void)pixaura_preview_release(raster,preview);}
        if(evaluated->serial){(void)pixaura_decode_release(raster,evaluated);}
    }} cleanup{&s.raster,&evaluated,&preview};
    if(!operations.empty())check(pixaura_working_evaluate(&s.raster,&s.original,1,byte_pointer(projection),projection.size(),&s.limits,&evaluated));
    const pixaura_preview_request fit{1,sizeof(fit),PIXAURA_PREVIEW_FIT,max_width,max_height,0};
    check(pixaura_preview_render(&s.raster,operations.empty()?&s.original:&evaluated,&fit,nullptr,&preview));
    pixaura_preview_metadata metadata{};check(pixaura_preview_query(&s.raster,&preview,&metadata));
    require(metadata.width&&metadata.height&&metadata.image_bytes==static_cast<uint64_t>(metadata.width)*metadata.height*4,14);
    require(metadata.image_bytes<=capacity,11);
    result_match(s,*binding);require(result_find(*s.loaded.snapshot,id),9);
    check(pixaura_preview_copy(&s.raster,&preview,0,output,metadata.image_bytes));
    *width=metadata.width;*height=metadata.height;
});}
