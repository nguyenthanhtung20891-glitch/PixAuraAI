#include "../src/document.hpp"
#include "../src/storage.hpp"
#include "pixaura/manual.h"
#include "pixaura/geometry.h"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <chrono>
#include <filesystem>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>
using namespace pixaura;
namespace fs = std::filesystem;
static unsigned checks = 0;
#define CHECK(x) do { ++checks; if (!(x)) { std::fprintf(stderr,"geometry check %d: %s\n",__LINE__,#x); std::exit(1); } } while(false)
static std::string id(unsigned n) { char text[33]; std::snprintf(text,sizeof(text),"%032x",n); return text; }
static const uint8_t* ptr(const std::string& s) { return reinterpret_cast<const uint8_t*>(s.data()); }
static std::string operation(unsigned n, bool crop, const std::string& parameters) {
    return "{\"operations\":[{\"id\":\""+id(n)+"\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":"+parameters+",\"type\":\"pixaura."+(crop?"crop":"rotate")+"\"}]}";
}
static std::string turn(unsigned n, int q) { return operation(n,false,"{\"quarter_turns\":"+std::to_string(q)+"}"); }
static std::string crop(unsigned n, int x, int y, int w, int h) {
    return operation(n,true,"{\"height_ppm\":"+std::to_string(h)+",\"width_ppm\":"+std::to_string(w)+",\"x_ppm\":"+std::to_string(x)+",\"y_ppm\":"+std::to_string(y)+"}");
}
struct Input : storage::Reader {
    std::size_t offset = 0;
    std::size_t read(uint8_t* output, std::size_t capacity) override {
        const auto count = std::min(capacity,sizeof(decode_png)-offset);
        std::memcpy(output,decode_png+offset,count); offset+=count; return count;
    }
};
static std::string serialized(pixaura_document_context& c, const pixaura_document_handle& h) {
    uint64_t required = 0; CHECK(pixaura_document_serialize(&c,&h,nullptr,0,&required,nullptr)==0);
    std::string s(static_cast<std::size_t>(required),' ');
    CHECK(pixaura_document_serialize(&c,&h,reinterpret_cast<uint8_t*>(s.data()),s.size(),&required,nullptr)==0); return s;
}
static std::string projection(pixaura_document_context& c, const pixaura_manual_gesture& g, const pixaura_document_handle& h, uint64_t& seq) {
    uint64_t required = 0; CHECK(pixaura_manual_geometry_projection(&c,&g,&h,nullptr,0,&required,&seq)==11);
    std::string s(static_cast<std::size_t>(required),' ');
    CHECK(pixaura_manual_geometry_projection(&c,&g,&h,reinterpret_cast<uint8_t*>(s.data()),s.size(),&required,&seq)==0); return s;
}
static document::Snapshot snapshot(const std::string& s, const std::string& session) {
    auto parsed=document::deserialize(s,session); CHECK(parsed.code==0); return parsed.value;
}
int main(int argc,char** argv) {
    CHECK(argc==2);
    const auto root=fs::absolute(fs::path(argv[1]))/std::string("manual-geometry-").append(std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    CHECK(fs::create_directory(root));
    const std::string path=root.generic_string(), session=id(500), context_id=id(600), raster_id=id(601);
    storage::AssetStore assets(path); Input input; const auto asset=assets.ingest(input,sizeof(decode_png));
    const std::string initial="{\"current_revision_id\":\""+id(100)+"\",\"document_id\":\""+id(2)+"\",\"operations\":[],\"project_id\":\""+id(1)+"\",\"redo\":[],\"revisions\":[{\"actor\":\"import\",\"id\":\""+id(100)+"\",\"parent_id\":null,\"plan_id\":null,\"stack\":[]}],\"schema_version\":1,\"source\":{\"byte_length\":"+std::to_string(sizeof(decode_png))+",\"metadata\":{\"codec\":\"png\",\"has_alpha\":true,\"height\":3,\"icc_sha256\":null,\"orientation\":1,\"width\":2},\"sha256\":\""+std::string(asset.digest)+"\"}}\n";
    pixaura_document_context context{}; pixaura_document_handle base{};
    CHECK(pixaura_document_context_init(1,&context,sizeof(context),ptr(context_id),32,nullptr)==0);
    CHECK(pixaura_document_create(1,&context,ptr(initial),initial.size(),ptr(session),32,&base,nullptr)==0);
    auto repository=std::make_unique<storage::Repository>(path);
    CHECK(repository->create(*snapshot(initial,session)).epoch==1);
    pixaura_decode_context raster{}; pixaura_decode_limits dl{}; pixaura_working_limits wl{};
    pixaura_decode_handle source{}, decoded{}, original{};
    CHECK(pixaura_decode_default_limits(1,&dl)==0 && pixaura_working_default_limits(1,&wl)==0);
    CHECK(pixaura_decode_context_init(1,&raster,sizeof(raster),ptr(raster_id),32,&dl)==0);
    CHECK(pixaura_decode_open(&raster,ptr(path),path.size(),reinterpret_cast<const uint8_t*>(asset.digest.data()),64,sizeof(decode_png),&source)==0);
    CHECK(pixaura_decode_image(&raster,&source,&decoded)==0 && pixaura_working_normalize(&raster,&decoded,&wl,&original)==0);
    CHECK(pixaura_decode_release(&raster,&decoded)==0 && pixaura_decode_release(&raster,&source)==0);
    float original_pixels[24]; CHECK(pixaura_working_copy(&raster,&original,0,original_pixels,24)==0);
    unsigned fresh=1000;
    auto begin=[&](bool is_crop, const pixaura_document_handle& live) {
        pixaura_manual_gesture g{}; const auto gid=id(++fresh); const std::string tool=is_crop?"pixaura.crop":"pixaura.rotate";
        CHECK(pixaura_manual_geometry_begin(1,&context,&live,ptr(gid),32,ptr(tool),tool.size(),nullptr,0,&g)==0); return g;
    };
    auto persist=[&](const pixaura_document_handle& proposal,const std::string& request) {
        const auto proposed=serialized(context,proposal); auto doc=snapshot(proposed,session); const auto loaded=repository->read();
        auto stack=document::replay(*doc,doc->current()); CHECK(stack.code==0);
        std::string ordered="["; for(const auto& op:stack.value) { if(ordered.size()>1) ordered+=','; ordered+='"'; ordered+=op.id.text(); ordered+='"'; } ordered+=']';
        const auto start=request.find('['), end=request.rfind(']');
        std::string command="{\"command_version\":1,\"kind\":\"commit\",\"expected_revision_id\":\"";
        command+=loaded.snapshot->current().text(); command+="\",\"expected_session_id\":\""; command+=loaded.snapshot->session().id.text();
        command+="\",\"expected_generation\":"; command+=std::to_string(loaded.snapshot->session().generation);
        command+=",\"revision_id\":\""; command+=doc->current().text(); command+="\",\"actor\":\"manual\",\"plan_id\":null,\"operations\":";
        command+=request.substr(start,end-start+1); command+=",\"stack\":"; command+=ordered; command+='}';
        CHECK(repository->apply(command,loaded.epoch).epoch==loaded.epoch+1);
        CHECK(document::serialize(*repository->read().snapshot).value==proposed);
        repository.reset(); repository=std::make_unique<storage::Repository>(path);
        const auto restarted=repository->read(); CHECK(document::serialize(*restarted.snapshot).value==proposed);
        CHECK(restarted.snapshot->source().sha256==doc->source().sha256 && doc->source().sha256.text()==asset.digest);
        CHECK(document::serialize(*snapshot(proposed,id(501))).value==proposed);
        assets.verify(asset);
    };
    // Boundary rejection preserves the last valid update and output sentinel.
    auto g=begin(true,base); uint64_t seq=99;
    for(unsigned field=0;field<5;++field) {
        auto invalid=g;
        if(field==0) invalid.reserved=1;
        if(field==1) invalid.api_version=2;
        if(field==2) invalid.kind=0;
        if(field==3) invalid.context_id[0]='f';
        if(field==4) invalid.serial=base.serial;
        CHECK(pixaura_manual_geometry_current(&context,&invalid,&base,1)==3);
    }
    pixaura_manual_gesture unopened{}; std::memset(&unopened,0x5a,sizeof(unopened)); const auto before_token=unopened;
    const auto unapproved_id=id(++fresh);
    for(const std::string tool:{"pixaura.resize","pixaura.exposure","pixaura.flip"}) {
        CHECK(pixaura_manual_geometry_begin(1,&context,&base,ptr(unapproved_id),32,ptr(tool),tool.size(),nullptr,0,&unopened)==5);
        CHECK(std::memcmp(&unopened,&before_token,sizeof(unopened))==0);
    }
    const std::string crop_tool="pixaura.crop";
    CHECK(pixaura_manual_geometry_begin(2,&context,&base,ptr(unapproved_id),32,ptr(crop_tool),crop_tool.size(),nullptr,0,&unopened)==2);
    CHECK(pixaura_manual_geometry_begin(1,&context,&base,ptr(unapproved_id),32,ptr(crop_tool),crop_tool.size(),nullptr,0,&unopened)==1);
    const auto valid=crop(++fresh,999999,999999,1,1);
    CHECK(pixaura_manual_geometry_update(&context,&g,&base,ptr(valid),valid.size(),&seq)==0 && seq==1);
    for(const auto& bad:{crop(++fresh,0,0,0,1),crop(++fresh,0,0,1,0),crop(++fresh,999999,0,2,1),crop(++fresh,0,999999,1,2),crop(++fresh,2147483647,0,1,1),turn(++fresh,1)}) {
        CHECK(pixaura_manual_geometry_update(&context,&g,&base,ptr(bad),bad.size(),&seq)==7 && seq==1);
    }
    CHECK(pixaura_manual_geometry_current(&context,&g,&base,1)==0);
    // Actual PRV1 render, cancellation and admission preserve previous valid preview.
    pixaura_preview_ticket ticket{}; pixaura_preview_request pr{1,sizeof(pr),PIXAURA_PREVIEW_EXACT,0,0,0};
    pixaura_decode_handle preview{}; uint64_t rendered=999;
    CHECK(pixaura_preview_begin(&raster,1,&ticket)==0);
    CHECK(pixaura_manual_geometry_render(&context,&g,&base,&raster,&original,&wl,nullptr,&ticket,&pr,&preview,&rendered)==0 && rendered==1);
    pixaura_preview_metadata pm{}; CHECK(pixaura_preview_query(&raster,&preview,&pm)==0 && pm.width==1 && pm.height==1);
    const auto displayed=preview; pixaura_decode_handle failed=preview;
    pixaura_working_limits restricted=wl; restricted.image_bytes=16;
    CHECK(pixaura_preview_begin(&raster,1,&ticket)==0);
    CHECK(pixaura_manual_geometry_render(&context,&g,&base,&raster,&original,&restricted,nullptr,&ticket,&pr,&failed,&rendered)==8);
    CHECK(std::memcmp(&failed,&displayed,sizeof(failed))==0 && rendered==1);
    CHECK(pixaura_manual_geometry_current(&context,&g,&base,1)==0);
    CHECK(pixaura_preview_cancel(&raster,&ticket)==0);
    CHECK(pixaura_manual_geometry_render(&context,&g,&base,&raster,&original,&wl,nullptr,&ticket,&pr,&failed,&rendered)==13);
    CHECK(std::memcmp(&failed,&displayed,sizeof(failed))==0 && rendered==1);
    CHECK(pixaura_preview_query(&raster,&displayed,&pm)==0);
    CHECK(pixaura_manual_geometry_update(&context,&g,&base,ptr(valid),valid.size(),&seq)==0 && seq==2);
    CHECK(pixaura_manual_geometry_current(&context,&g,&base,1)==13 && pixaura_manual_geometry_current(&context,&g,&base,2)==0);
    CHECK(pixaura_preview_release(&raster,&preview)==0);
    CHECK(pixaura_manual_geometry_cancel(&context,&g)==0 && pixaura_manual_geometry_release(&context,&g)==0);
    // Different ordered geometry yields exact source-pixel selections.
    auto ordered_pixels=[&](const std::vector<std::string>& requests, uint32_t& width, uint32_t& height) {
        pixaura_document_handle live{};
        CHECK(pixaura_document_open(1,&context,ptr(initial),initial.size(),ptr(session),32,&live,nullptr)==0);
        std::string projected;
        for(const auto& request:requests) {
            auto edit=begin(request.find("pixaura.crop")!=std::string::npos,live);
            uint64_t sequence=0; CHECK(pixaura_manual_geometry_update(&context,&edit,&live,ptr(request),request.size(),&sequence)==0);
            projected=projection(context,edit,live,sequence);
            pixaura_document_handle next{}; uint32_t changed=0; const auto revision=id(++fresh);
            CHECK(pixaura_manual_geometry_commit(&context,&edit,&live,ptr(revision),32,&next,&changed)==0 && changed==1);
            CHECK(pixaura_manual_geometry_release(&context,&edit)==0 && pixaura_document_release(&context,&live)==0); live=next;
        }
        pixaura_decode_handle result{}; CHECK(pixaura_working_evaluate(&raster,&original,1,ptr(projected),projected.size(),&wl,&result)==0);
        pixaura_working_metadata metadata{}; CHECK(pixaura_working_query(&raster,&result,&metadata)==0);
        width=metadata.width; height=metadata.height; std::vector<float> pixels(static_cast<std::size_t>(width)*height*4);
        CHECK(pixaura_working_copy(&raster,&result,0,pixels.data(),pixels.size())==0);
        CHECK(pixaura_decode_release(&raster,&result)==0 && pixaura_document_release(&context,&live)==0); return pixels;
    };
    uint32_t width=0,height=0;
    const auto cr=ordered_pixels({crop(++fresh,999999,0,1,1000000),turn(++fresh,1)},width,height);
    CHECK(width==3 && height==1);
    for(unsigned i=0;i<3;++i) CHECK(std::memcmp(cr.data()+i*4,original_pixels+(5-i*2)*4,16)==0);
    const auto rc=ordered_pixels({turn(++fresh,1),crop(++fresh,999999,0,1,1000000)},width,height);
    CHECK(width==1 && height==2 && rc!=cr);
    CHECK(std::memcmp(rc.data(),original_pixels,16)==0 && std::memcmp(rc.data()+4,original_pixels+4,16)==0);
    const auto four=ordered_pixels({turn(++fresh,1),turn(++fresh,1),turn(++fresh,1),turn(++fresh,1)},width,height);
    CHECK(width==2 && height==3 && std::memcmp(four.data(),original_pixels,sizeof(original_pixels))==0);
    // No-op crop/rotation gestures never allocate a revision.
    for(bool is_crop:{false,true}) {
        g=begin(is_crop,base); const auto neutral=is_crop?crop(++fresh,0,0,1000000,1000000):turn(++fresh,0);
        CHECK(pixaura_manual_geometry_update(&context,&g,&base,ptr(neutral),neutral.size(),&seq)==0);
        pixaura_document_handle same{}; uint32_t changed=99; const auto revision=id(++fresh);
        CHECK(pixaura_manual_geometry_commit(&context,&g,&base,ptr(revision),32,&same,&changed)==0 && changed==0 && same.serial==base.serial);
        CHECK(pixaura_manual_geometry_release(&context,&g)==0);
    }
    const auto baseline=serialized(context,base);
    g=begin(false,base);
    pixaura_document_handle empty_proposal{}; uint32_t empty_changed=99; const auto empty_revision=id(++fresh);
    CHECK(pixaura_manual_geometry_commit(&context,&g,&base,ptr(empty_revision),32,&empty_proposal,&empty_changed)==0 && empty_changed==0 && empty_proposal.serial==base.serial);
    CHECK(pixaura_manual_geometry_release(&context,&g)==0);
    // Ordered crop->rotate->crop, four turns, interleaved neutral operations.
    std::vector<std::string> edits{crop(++fresh,0,0,1000000,666666),turn(++fresh,1),crop(++fresh,500000,0,500000,1000000),turn(++fresh,0),turn(++fresh,1),turn(++fresh,1),turn(++fresh,1),turn(++fresh,1)};
    unsigned expected_revisions=1;
    for(const auto& request:edits) {
        const bool is_crop=request.find("pixaura.crop")!=std::string::npos; g=begin(is_crop,base);
        const auto before=serialized(context,base);
        for(unsigned i=0;i<1001;++i) CHECK(pixaura_manual_geometry_update(&context,&g,&base,ptr(request),request.size(),&seq)==0 && seq==i+1);
        const auto projected=projection(context,g,base,seq); CHECK(pixaura_manual_geometry_current(&context,&g,&base,1000)==13);
        CHECK(serialized(context,base)==before);
        pixaura_decode_handle evaluated{}; CHECK(pixaura_working_evaluate(&raster,&original,1,ptr(projected),projected.size(),&wl,&evaluated)==0);
        CHECK(pixaura_preview_begin(&raster,1,&ticket)==0);
        CHECK(pixaura_manual_geometry_render(&context,&g,&base,&raster,&original,&wl,nullptr,&ticket,&pr,&preview,&rendered)==0 && rendered==1001);
        pixaura_working_metadata wm{}; CHECK(pixaura_working_query(&raster,&evaluated,&wm)==0);
        CHECK(pixaura_preview_query(&raster,&preview,&pm)==0 && pm.width==wm.width && pm.height==wm.height);
        CHECK(pixaura_preview_release(&raster,&preview)==0 && pixaura_decode_release(&raster,&evaluated)==0);
        pixaura_document_handle proposal{}; uint32_t changed=99; const auto revision=id(++fresh);
        CHECK(pixaura_manual_geometry_commit(&context,&g,&base,ptr(revision),32,&proposal,&changed)==0);
        if(changed) {
            ++expected_revisions; persist(proposal,request);
            CHECK(snapshot(serialized(context,proposal),session)->revisions().size()==expected_revisions);
            CHECK(pixaura_document_release(&context,&base)==0); base=proposal;
        } else CHECK(proposal.serial==base.serial);
        CHECK(pixaura_manual_geometry_current(&context,&g,&base,1001)==13);
        CHECK(pixaura_manual_geometry_release(&context,&g)==0);
    }
    CHECK(expected_revisions==8 && serialized(context,base)!=baseline);
    // Session/source/revision stale binding cancels; cannot retry with old base.
    g=begin(false,base); auto current=serialized(context,base); pixaura_document_handle reopened{};
    const auto new_session=id(502); CHECK(pixaura_document_open(1,&context,ptr(current),current.size(),ptr(new_session),32,&reopened,nullptr)==0);
    const auto change=turn(++fresh,1); seq=999;
    CHECK(pixaura_manual_geometry_update(&context,&g,&reopened,ptr(change),change.size(),&seq)==9 && seq==999);
    CHECK(pixaura_manual_geometry_update(&context,&g,&base,ptr(change),change.size(),&seq)==13);
    CHECK(pixaura_manual_geometry_release(&context,&g)==0 && pixaura_document_release(&context,&reopened)==0);
    auto altered=current; const auto digest_pos=altered.find(std::string(asset.digest)); CHECK(digest_pos!=std::string::npos);
    altered.replace(digest_pos,64,std::string(64,'b')); g=begin(false,base);
    CHECK(pixaura_document_open(1,&context,ptr(altered),altered.size(),ptr(session),32,&reopened,nullptr)==0);
    CHECK(pixaura_manual_geometry_current(&context,&g,&reopened,1)==9);
    CHECK(pixaura_manual_geometry_release(&context,&g)==0 && pixaura_document_release(&context,&reopened)==0);
    // Full combined context budget: failed changed commit retains retryable state.
    g=begin(false,base); CHECK(pixaura_manual_geometry_update(&context,&g,&base,ptr(change),change.size(),&seq)==0);
    std::vector<pixaura_document_handle> handles;
    for(unsigned i=0;i<62;++i) { pixaura_document_handle h{}; CHECK(pixaura_document_open(1,&context,ptr(current),current.size(),ptr(session),32,&h,nullptr)==0); handles.push_back(h); }
    pixaura_document_handle sentinel=base; uint32_t changed=99; const auto retry_revision=id(++fresh);
    CHECK(pixaura_manual_geometry_commit(&context,&g,&base,ptr(retry_revision),32,&sentinel,&changed)==8 && sentinel.serial==base.serial && changed==99);
    CHECK(pixaura_manual_geometry_current(&context,&g,&base,1)==0);
    CHECK(pixaura_document_release(&context,&handles.back())==0); handles.pop_back();
    CHECK(pixaura_manual_geometry_commit(&context,&g,&base,ptr(retry_revision),32,&sentinel,&changed)==0 && changed==1);
    for(const auto& h:handles) CHECK(pixaura_document_release(&context,&h)==0);
    CHECK(pixaura_manual_geometry_release(&context,&g)==0);
    g=begin(false,base); CHECK(pixaura_manual_geometry_current(&context,&g,&sentinel,1)==9);
    CHECK(pixaura_manual_geometry_release(&context,&g)==0 && pixaura_document_release(&context,&sentinel)==0);
    float unchanged[24]; CHECK(pixaura_working_copy(&raster,&original,0,unchanged,24)==0 && std::memcmp(unchanged,original_pixels,sizeof(unchanged))==0);
    CHECK(pixaura_decode_context_destroy(&raster)==0 && pixaura_document_context_destroy(&context)==0);
    std::printf("manual geometry: %u checks; ordered replay, 1001-update commits, PRV1, durable restart and admission retry PASS\n",checks);
    return 0;
}
