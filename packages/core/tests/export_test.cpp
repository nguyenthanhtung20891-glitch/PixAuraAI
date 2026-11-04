#include "pixaura/export.h"
#include "pixaura/agent_execution.h"
#include "../src/document.hpp"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>
extern "C" int32_t pixaura_test_editor_document(pixaura_editor_context*,uint8_t*,uint64_t,uint64_t*);
static std::atomic<unsigned> checks{0};
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"export %d %s\n",__LINE__,#x);std::abort();}}while(false)
static const uint8_t* bytes(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
static int64_t importer(void* p,uint8_t* out,uint64_t n){auto& at=*static_cast<std::size_t*>(p);n=std::min<uint64_t>(n,sizeof(decode_png)-at);std::memcpy(out,decode_png+at,static_cast<std::size_t>(n));at+=static_cast<std::size_t>(n);return static_cast<int64_t>(n);}
struct Owner{
 pixaura_editor_context c{};std::filesystem::path root;
 explicit Owner(const std::filesystem::path& parent,unsigned serial){char id[33];std::snprintf(id,sizeof(id),"%032x",serial);root=parent/(std::string("export-")+id);
  std::filesystem::create_directory(root);const auto path=root.generic_string();std::size_t at=0;
  pixaura_capability_profile profile{};CHECK(pixaura_capability_baseline(1,&profile)==0);
  profile.hardware.logical_cpu_count=8;profile.hardware.physical_memory_bytes=UINT64_C(8589934592);
  profile.runtime={1,sizeof(pixaura_capability_dynamic),1,1,0,1,UINT64_C(536870912),1};
  const pixaura_policy_observation observation{1,sizeof(pixaura_policy_observation),1,1,0,1,0,0,UINT64_C(536870912),1};
  pixaura_execution_strategy strategy{};
  CHECK(pixaura_editor_open_policy(1,&c,bytes(path),path.size(),reinterpret_cast<const uint8_t*>(id),32,importer,&at,sizeof(decode_png),&profile,&observation,&strategy)==0);}
 ~Owner(){CHECK(pixaura_editor_stop(&c)==0);CHECK(pixaura_editor_destroy(&c)==0);}
 std::string document(){uint64_t n=0;CHECK(pixaura_test_editor_document(&c,nullptr,0,&n)==0);std::string s(static_cast<std::size_t>(n),' ');CHECK(pixaura_test_editor_document(&c,reinterpret_cast<uint8_t*>(s.data()),n,&n)==0);return s;}
 std::string binding(){uint64_t n=0;CHECK(pixaura_editor_agent_binding(1,&c,nullptr,0,&n)==11);std::string s(static_cast<std::size_t>(n),' ');CHECK(pixaura_editor_agent_binding(1,&c,reinterpret_cast<uint8_t*>(s.data()),n,&n)==0);s.pop_back();return s;}
 std::string plan(std::string fixture){const auto begin=fixture.find("\"binding\":")+10;const auto end=fixture.find('}',begin)+1;fixture.replace(begin,end-begin,binding());return fixture;}
 pixaura_agent_approval approve(const std::string& p){pixaura_agent_approval a{};pixaura_agent_execution_request r{1,sizeof(r),1,0,{0,0,0,0}};CHECK(pixaura_editor_agent_approve(1,&c,&r,bytes(p),p.size(),&a)==0);return a;}
 int execute(const std::string& p,const pixaura_agent_approval* a){pixaura_agent_execution_request r{1,sizeof(r),1,0,{0,0,0,0}};pixaura_agent_execution_receipt out;std::memset(&out,0xa5,sizeof(out));return pixaura_editor_agent_execute(1,&c,&r,a,bytes(p),p.size(),&out);}
 pixaura_export_request bind(){pixaura_export_request request{};CHECK(pixaura_export_bind(1,&c,&request)==0);CHECK(request.version==1&&request.struct_size==sizeof(request)&&request.format==PIXAURA_EXPORT_PNG&&request.quality==0);return request;}
};
struct Encoded{int32_t code=0;uint32_t state=99,width=0,height=0;std::vector<uint8_t> bytes;};
static Encoded encode(Owner& o,pixaura_export_request request,uint64_t capacity=1048576){
 Encoded out;out.bytes.assign(static_cast<std::size_t>(capacity),0x5a);uint64_t n=0;const auto sentinel=out.bytes;
 out.code=pixaura_export_encode(1,&o.c,&request,nullptr,out.bytes.data(),capacity,&n,&out.width,&out.height,&out.state);
 if(out.code){CHECK(out.bytes==sentinel);out.bytes.clear();}else{CHECK(n&&n<=capacity&&out.state==PIXAURA_EXPORT_SUCCEEDED);out.bytes.resize(static_cast<std::size_t>(n));}
 return out;
}
static pixaura::document::Snapshot snap(const std::string& manifest){auto parsed=pixaura::document::deserialize(manifest,std::string(32,'a'));CHECK(parsed.code==0);return parsed.value;}
static bool chunk(const std::vector<uint8_t>& p,const char* type){
    if(p.size()<8||std::memcmp(p.data(),"\x89PNG\r\n\x1a\n",8)!=0){
        return false;
    }
    std::size_t i=8;
    bool seen=false;
    while(i+12<=p.size()){
        const uint32_t len=(uint32_t)p[i]<<24|(uint32_t)p[i+1]<<16|(uint32_t)p[i+2]<<8|p[i+3];
        if(i+12+len>p.size()){
            return false;
        }
        if(std::memcmp(p.data()+i+4,type,4)==0){
            seen=true;
        }
        if(std::memcmp(p.data()+i+4,"IEND",4)==0){
            return seen;
        }
        i+=12+static_cast<std::size_t>(len);
    }
    return false;
}
static bool jpeg_app1(const std::vector<uint8_t>& p){
    if(p.size()<4||p[0]!=0xff||p[1]!=0xd8){
        return true;
    }
    std::size_t i=2;
    while(i+3<p.size()&&p[i]==0xff){
        while(i<p.size()&&p[i]==0xff){
            ++i;
        }
        if(i>=p.size()){
            return true;
        }
        const auto marker=p[i++];
        if(marker==0xd9||marker==0xda){
            return false;
        }
        if(marker>=0xd0&&marker<=0xd9){
            continue;
        }
        if(i+1>=p.size()){
            return true;
        }
        const auto len=(static_cast<std::size_t>(p[i])<<8)|p[i+1];
        if(len<2||i+len>p.size()){
            return true;
        }
        if(marker==0xe1){
            return true;
        }
        i+=len;
    }
    return true;
}
static std::string exposure(){return std::string("{\"actions\":[{\"action_index\":0,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":5000},\"tool_id\":\"pixaura.exposure\"}],\"binding\":{},\"plan_version\":1}\n");}
static std::string geometry(){return std::string("{\"actions\":[{\"action_index\":0,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"height_ppm\":500000,\"width_ppm\":500000,\"x_ppm\":0,\"y_ppm\":0},\"tool_id\":\"pixaura.crop\"},{\"action_index\":1,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"quarter_turns\":1},\"tool_id\":\"pixaura.rotate\"}],\"binding\":{},\"plan_version\":1}\n");}
int main(int argc,char** argv){
 CHECK(argc==2);const auto parent=std::filesystem::u8path(argv[1])/("export-suite-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 CHECK(std::filesystem::create_directory(parent));unsigned serial=91000;
 {Owner o(parent,++serial);const auto before=o.document();const auto model=snap(before);CHECK(model->revisions().size()==1);CHECK(model->source().metadata.width==2&&model->source().metadata.height==3);
  auto request=o.bind();CHECK(std::string(request.revision_id,32)==model->current().text());
  const auto png=encode(o,request);CHECK(png.code==0&&png.width==2&&png.height==3&&chunk(png.bytes,"IHDR")&&!chunk(png.bytes,"tEXt")&&!chunk(png.bytes,"eXIf")&&!chunk(png.bytes,"iCCP"));
  CHECK(png.bytes.size()>=24&&png.bytes[24]==8&&png.bytes[25]==6);const auto again=encode(o,request);CHECK(again.bytes==png.bytes);
  request.format=PIXAURA_EXPORT_JPEG;request.quality=90;const auto jpeg=encode(o,request);CHECK(jpeg.code==0&&jpeg.width==2&&jpeg.height==3&&!jpeg_app1(jpeg.bytes));
  request.quality=30;const auto low=encode(o,request);CHECK(low.code==0&&low.bytes!=jpeg.bytes);CHECK(o.document()==before);
  request.quality=0;const auto bad=encode(o,request);CHECK(bad.code!=0&&bad.state==PIXAURA_EXPORT_FAILED);
  request=o.bind();request.format=3;CHECK(encode(o,request).code!=0);request=o.bind();request.mode=PIXAURA_EXPORT_FIT;request.max_width=1;request.max_height=1;const auto fit=encode(o,request);CHECK(fit.code==0&&fit.width==1&&fit.height==1);
  request=o.bind();request.epoch+=1;std::vector<uint8_t> sentinel(128,0x5a),kept=sentinel;uint64_t n=7;uint32_t w=9,h=9,state=4;CHECK(pixaura_export_encode(1,&o.c,&request,nullptr,sentinel.data(),sentinel.size(),&n,&w,&h,&state)==9);
  CHECK(state==PIXAURA_EXPORT_STALE&&sentinel==kept&&n==7&&w==9&&h==9);CHECK(o.document()==before);
  const int32_t crop_values[]={500000,500000,0,0};CHECK(pixaura_editor_select(&o.c,3)==0);CHECK(pixaura_editor_update(&o.c,crop_values,4)==0);CHECK(pixaura_editor_action(&o.c,1)==0);
  const auto cropped_doc=o.document();CHECK(snap(cropped_doc)->revisions().size()==2);const auto cropped=encode(o,o.bind());CHECK(cropped.code==0&&cropped.width==1&&cropped.height==2&&o.document()==cropped_doc);
  CHECK(pixaura_editor_action(&o.c,2)==0);const auto undone=o.document();CHECK(snap(undone)->revisions().size()==2);const auto older=snap(undone)->revisions().back().id.text();
  auto selected=o.bind();CHECK(std::string(selected.revision_id,32)==snap(undone)->current().text());std::memcpy(selected.revision_id,older.data(),32);const auto historical=encode(o,selected);CHECK(historical.code==0&&historical.width==1&&historical.height==2&&o.document()==undone);
  CHECK(pixaura_editor_action(&o.c,3)==0);CHECK(encode(o,o.bind()).bytes==cropped.bytes);
 }
 {Owner o(parent,++serial);const auto p=o.plan(exposure());auto a=o.approve(p);CHECK(o.execute(p,&a)==0);const auto after=o.document();CHECK(snap(after)->revisions().size()==2&&snap(after)->revisions().back().actor=="ai");
  const auto png=encode(o,o.bind());CHECK(png.code==0&&png.width==2&&png.height==3);const auto root=encode(o,{});CHECK(root.code!=0);CHECK(o.document()==after);CHECK(pixaura_editor_agent_release(1,&o.c,&a)==0);
 }
 {Owner o(parent,++serial);const auto p=o.plan(geometry());auto a=o.approve(p);CHECK(o.execute(p,&a)==0);const auto after=o.document();const auto png=encode(o,o.bind());CHECK(png.code==0&&png.width==2&&png.height==1&&o.document()==after);CHECK(pixaura_editor_agent_release(1,&o.c,&a)==0);}
 {Owner o(parent,++serial);const auto before=o.document();auto request=o.bind();std::atomic<int> first{-1},second{-1};
  std::thread a([&]{first=encode(o,request).code;});std::thread b([&]{second=encode(o,request).code;});a.join();b.join();
  CHECK((first==0||first==8)&&(second==0||second==8));CHECK(o.document()==before);
 }
 {Owner o(parent,++serial);auto request=o.bind();std::atomic<int> result{-1};std::vector<uint8_t> output(1048576,0x5a),sentinel=output;uint64_t n=3;uint32_t w=4,h=5,state=1;
  std::thread worker([&]{result=pixaura_export_encode(1,&o.c,&request,nullptr,output.data(),output.size(),&n,&w,&h,&state);});
  while(result<0){(void)pixaura_export_cancel(&o.c);std::this_thread::yield();}worker.join();CHECK(result==0||result==13);if(result==13){CHECK(output==sentinel&&state==PIXAURA_EXPORT_CANCELLED);}
 }
 {Owner o(parent,++serial);CHECK(pixaura_editor_stop(&o.c)==0);std::vector<uint8_t> output(128,0x5a),sentinel=output;uint64_t n=1;uint32_t w=1,h=1,state=1;pixaura_export_request blank{};
  CHECK(pixaura_export_encode(1,&o.c,&blank,nullptr,output.data(),output.size(),&n,&w,&h,&state)==13);CHECK(output==sentinel);
 }
 std::printf("export checks %u\n",checks.load());
}
