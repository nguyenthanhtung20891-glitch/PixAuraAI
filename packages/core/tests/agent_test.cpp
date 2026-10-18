#include "pixaura/agent.h"
#include "pixaura/manual.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <thread>
#include <atomic>
#define CHECK(x) do { ++checks; if(!(x)){std::fprintf(stderr,"agent line %d: %s\n",__LINE__,#x);std::abort();} } while(false)
static unsigned checks=0;
static const uint8_t* b(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
static std::string id(unsigned n){char s[33];std::snprintf(s,sizeof(s),"%032x",n);return s;}
static std::string op(unsigned n=1){return "{\"id\":\""+id(n)+"\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":1000}}";}
static std::string envelope(const std::string& operation){return "{\"operations\":["+operation+"]}";}
static std::string replace(std::string s,const std::string& from,const std::string& to){const auto p=s.find(from);CHECK(p!=s.npos);s.replace(p,from.size(),to);return s;}
static std::string dump(pixaura_document_context& c,const pixaura_document_handle& h){
    std::array<uint8_t,8192> out{};uint64_t size=0;
    CHECK(pixaura_document_serialize(&c,&h,out.data(),out.size(),&size,nullptr)==0);
    return {reinterpret_cast<char*>(out.data()),static_cast<std::size_t>(size)};
}
int main(int argc,char** argv){
    CHECK(argc==2);std::ifstream file(argv[1],std::ios::binary);const std::string golden((std::istreambuf_iterator<char>(file)),{});CHECK(!golden.empty());
    pixaura_document_context c{},other{};pixaura_document_handle base{},foreign{};
    const auto session=id(500),cid=id(1000),oid=id(1001);
    CHECK(pixaura_document_context_init(1,&c,sizeof(c),b(cid),32,nullptr)==0);
    CHECK(pixaura_document_context_init(1,&other,sizeof(other),b(oid),32,nullptr)==0);
    CHECK(pixaura_document_open(1,&c,b(golden),golden.size(),b(session),32,&base,nullptr)==0);
    CHECK(pixaura_document_open(1,&other,b(golden),golden.size(),b(session),32,&foreign,nullptr)==0);
    const auto request=envelope(op());pixaura_agent_proposal proposal{};
    auto rejected=[&](const std::string& s,int32_t expected=-1){
        pixaura_agent_proposal out;std::memset(&out,0x5a,sizeof(out));const auto sentinel=out;
        const auto result=pixaura_agent_stage(1,&c,&base,b(s),s.size(),&out);
        CHECK(result!=0);
        if(expected!=-1){if(result!=expected)std::fprintf(stderr,"expected=%d actual=%d input=%.160s\n",expected,result,s.c_str());CHECK(result==expected);}
        CHECK(std::memcmp(&out,&sentinel,sizeof(out))==0);
    };
    CHECK(pixaura_agent_stage(2,&c,&base,b(request),request.size(),&proposal)==2);
    CHECK(pixaura_agent_stage(1,nullptr,&base,b(request),request.size(),&proposal)==1);
    CHECK(pixaura_agent_stage(1,&c,&foreign,b(request),request.size(),&proposal)==3);
    rejected(std::string(8193,'x'),8);rejected("{",6);rejected("{\"operations\":[]}",1);
    rejected(envelope(replace(op(),"pixaura.exposure","host.shell")),5);
    rejected(envelope(replace(op(),"operation_version\":1","operation_version\":2")),5);
    rejected(envelope(replace(op(),"parameter_version\":1","parameter_version\":2")),5);
    rejected(envelope(replace(op(),"1000}","5001}")),7);
    rejected(envelope(replace(op(),"1000}","1.5}")));
    rejected(envelope(replace(op(),"1000}","1000,\"milli_ev\":0}")));
    rejected(envelope(replace(op(),"1000}","1000,\"shell\":0}")));
    rejected(envelope(op()+","+op()));
    rejected(replace(request,"{\"operations\":","{\"approved\":true,\"operations\":"));
    rejected(replace(request,"{\"operations\":","{\"actor\":\"ai\",\"operations\":"));
    rejected(replace(request,"{\"operations\":","{\"current_revision_id\":\""+id(1)+"\",\"operations\":"));
    std::string many;
    for(unsigned i=1;i<=9;++i){if(i>1)many+=',';many+=op(i);}
    rejected(envelope(many),8);
    std::string eight;
    for(unsigned i=1;i<=8;++i){if(i>1)eight+=',';eight+=op(i);}
    const auto bounded=envelope(eight);
    CHECK(pixaura_agent_stage(1,&c,&base,b(bounded),bounded.size(),&proposal)==0);
    CHECK(pixaura_agent_release(1,&c,&proposal)==0);
    CHECK(pixaura_agent_stage(1,&c,&base,b(request),request.size(),&proposal)==0);
    CHECK(sizeof(proposal)==56 && proposal.serial!=base.serial);
    CHECK(pixaura_agent_validate(1,&c,&proposal,&base)==0);
    CHECK(pixaura_agent_execution_authority(1,&c,&proposal,&base)==15);
    CHECK(pixaura_agent_validate(2,&c,&proposal,&base)==2);
    alignas(8) std::array<uint8_t,64> misaligned{};
    CHECK(pixaura_agent_validate(1,&c,reinterpret_cast<const pixaura_agent_proposal*>(misaligned.data()+1),&base)==1);
    CHECK(pixaura_agent_validate(1,&other,&proposal,&foreign)==3);
    for(unsigned field=0;field<6;++field){auto bad=proposal;
        if(field==0)bad.api_version=2;
        if(field==1)bad.struct_size=55;
        if(field==2)bad.kind=0;
        if(field==3)bad.reserved=1;
        if(field==4)bad.serial=0;
        if(field==5)bad.serial=UINT64_MAX;
        CHECK(pixaura_agent_validate(1,&c,&bad,&base)==3);
    }
    auto wrong=proposal;wrong.serial=base.serial;CHECK(pixaura_agent_validate(1,&c,&wrong,&base)==3);
    pixaura_document_handle forged{};std::memcpy(&forged,&proposal,sizeof(forged));forged.reserved=0;
    CHECK(pixaura_document_release(&c,&forged)==3);
    CHECK(pixaura_agent_stage(1,&c,&base,b(request),request.size(),reinterpret_cast<pixaura_agent_proposal*>(&base))==1);
    std::array<uint8_t,8192> output;output.fill(0x5a);uint64_t size=999;
    CHECK(pixaura_agent_inspect(1,&c,&proposal,&base,nullptr,0,&size)==11 && size>0);
    const auto needed=size;
    CHECK(pixaura_agent_inspect(1,&c,&proposal,&base,output.data(),1,&size)==11 && size==needed && output[0]==0x5a);
    CHECK(pixaura_agent_inspect(1,&c,&proposal,&base,output.data(),output.size(),reinterpret_cast<uint64_t*>(output.data()))==1);
    CHECK(pixaura_agent_inspect(1,&c,&proposal,&base,output.data(),UINT64_MAX,&size)==1);
    CHECK(pixaura_agent_inspect(1,&c,&proposal,&base,output.data(),output.size(),&size)==0);
    CHECK(output[size-1]=='\n' && output[size]==0x5a);
    std::array<uint8_t,8192> canonical{};uint64_t manual_size=0;
    CHECK(pixaura_manual_canonical_operation(1,b(request),request.size(),canonical.data(),canonical.size(),&manual_size)==0);
    CHECK(manual_size==size && std::memcmp(canonical.data(),output.data(),size)==0);
    CHECK(dump(c,base)==golden); // create, validate, inspect, denied execution: ZERO history mutation
    CHECK(pixaura_agent_release(1,&c,&proposal)==0);
    CHECK(pixaura_agent_release(1,&c,&proposal)==3);
    CHECK(dump(c,base)==golden); // release: ZERO history mutation
    auto stale_token=proposal;
    CHECK(pixaura_agent_stage(1,&c,&base,b(request),request.size(),&proposal)==0);
    CHECK(proposal.serial>stale_token.serial);CHECK(pixaura_agent_validate(1,&c,&stale_token,&base)==3);
    std::atomic<bool> valid{true};
    std::thread reader([&]{for(unsigned i=0;i<100;++i){auto result=pixaura_agent_validate(1,&c,&proposal,&base);if(result!=0 && result!=13)valid=false;}});
    CHECK(pixaura_agent_cancel(1,&c,&proposal)==0);reader.join();CHECK(valid.load());
    CHECK(pixaura_agent_cancel(1,&c,&proposal)==0);
    CHECK(pixaura_agent_execution_authority(1,&c,&proposal,&base)==13);
    CHECK(pixaura_agent_release(1,&c,&proposal)==0);
    std::array<pixaura_agent_proposal,8> tokens{};
    for(auto& t:tokens)CHECK(pixaura_agent_stage(1,&c,&base,b(request),request.size(),&t)==0);
    CHECK(pixaura_agent_stage(1,&c,&base,b(request),request.size(),&proposal)==8);
    for(auto& t:tokens)CHECK(pixaura_agent_release(1,&c,&t)==0);
    std::array<pixaura_document_handle,62> full{};
    for(auto& h:full)CHECK(pixaura_document_open(1,&c,b(golden),golden.size(),b(session),32,&h,nullptr)==0);
    CHECK(pixaura_agent_stage(1,&c,&base,b(request),request.size(),&proposal)==0);
    pixaura_document_handle excess{};
    CHECK(pixaura_document_open(1,&c,b(golden),golden.size(),b(session),32,&excess,nullptr)==8);
    CHECK(pixaura_agent_release(1,&c,&proposal)==0);
    CHECK(pixaura_document_open(1,&c,b(golden),golden.size(),b(session),32,&excess,nullptr)==0);
    CHECK(pixaura_agent_stage(1,&c,&base,b(request),request.size(),&proposal)==8);
    CHECK(pixaura_document_release(&c,&excess)==0);
    for(auto& h:full)CHECK(pixaura_document_release(&c,&h)==0);
    CHECK(pixaura_agent_stage(1,&c,&base,b(request),request.size(),&proposal)==0);
    pixaura_document_handle changed{};
    const std::string undo="{\"command_version\":1,\"kind\":\"undo\",\"expected_revision_id\":\"00000000000000000000000000000102\",\"expected_session_id\":\""+session+"\",\"expected_generation\":0}";
    CHECK(pixaura_document_apply(1,&c,&base,b(undo),undo.size(),&changed,nullptr)==0); // manual path independent of proposals
    CHECK(pixaura_agent_validate(1,&c,&proposal,&changed)==9);
    CHECK(pixaura_agent_validate(1,&c,&proposal,&base)==9); // stale is terminal
    CHECK(pixaura_agent_release(1,&c,&proposal)==0);
    CHECK(pixaura_agent_stage(1,&c,&base,b(request),request.size(),&proposal)==0);
    CHECK(pixaura_document_release(&c,&base)==0);
    CHECK(pixaura_agent_validate(1,&c,&proposal,&base)==9);
    CHECK(pixaura_agent_validate(1,&other,&tokens[0],&foreign)==3);
    CHECK(dump(other,foreign)==golden); // cancellation and staleness isolated
    CHECK(pixaura_document_context_destroy(&c)==0);
    CHECK(pixaura_agent_validate(1,&c,&proposal,&base)==3);
    CHECK(pixaura_agent_release(1,&c,&proposal)==3);
    CHECK(pixaura_document_context_destroy(&other)==0);
    std::printf("Agent core v1 checks=%u failures=0; no approval issuer/execution; side effects=0\n",checks);
}
