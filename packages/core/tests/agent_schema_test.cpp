#include "pixaura/agent_schema.h"
#include "../src/agent_schema_testing.hpp"
#include "agent_schema_fixtures.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"schema line %d: %s\n",__LINE__,#x);std::abort();}}while(false)
static const uint8_t* b(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
static std::string replace(std::string value,const std::string& from,const std::string& to){
    const auto p=value.find(from);CHECK(p!=value.npos);value.replace(p,from.size(),to);return value;
}
struct Owner {
    pixaura_document_context context{};pixaura_document_handle base{};
    explicit Owner(const char* identity){
        CHECK(pixaura_document_context_init(1,&context,sizeof(context),reinterpret_cast<const uint8_t*>(identity),32,nullptr)==0);
        CHECK(pixaura_document_open(1,&context,reinterpret_cast<const uint8_t*>(pixaura::agent_schema_fixtures::document),
            std::strlen(pixaura::agent_schema_fixtures::document),reinterpret_cast<const uint8_t*>("00000000000000000000000000000500"),32,&base,nullptr)==0);
    }
    ~Owner(){CHECK(pixaura_document_context_destroy(&context)==0);}
};
static std::string dump(Owner& o){std::array<uint8_t,8192> buffer{};uint64_t n=0;
    CHECK(pixaura_document_serialize(&o.context,&o.base,buffer.data(),buffer.size(),&n,nullptr)==0);
    return {reinterpret_cast<const char*>(buffer.data()),static_cast<std::size_t>(n)};
}
struct Hook {Owner* owner;uint32_t mode;};
static void checkpoint(void* state){
    auto& h=*static_cast<Hook*>(state);
    if(h.mode==1)CHECK(pixaura_agent_schema_cancel(1,&h.owner->context)==0);
    if(h.mode==2)CHECK(pixaura_document_release(&h.owner->context,&h.owner->base)==0);
    if(h.mode==3){
        pixaura_agent_schema_request r{};r.api_version=1;r.struct_size=sizeof(r);r.kind=1;
        const std::string input=pixaura::agent_schema_fixtures::cases[0].input;
        uint64_t required=777;
        CHECK(pixaura_agent_schema_canonicalize(1,&h.owner->context,&h.owner->base,&r,b(input),input.size(),nullptr,0,&required)==8);
        CHECK(required==777);
    }
}
int main(){
    CHECK(pixaura_agent_schema_test_boundary()==0);
    Owner a("000000000000000000000000000000a1"),other("000000000000000000000000000000a2");
    const auto original=dump(a),other_original=dump(other);
    std::array<uint8_t,8192> binding_bytes{};uint64_t binding_size=0;
    CHECK(pixaura_agent_schema_binding(1,&a.context,&a.base,binding_bytes.data(),binding_bytes.size(),&binding_size)==0);
    const std::string binding(reinterpret_cast<const char*>(binding_bytes.data()),static_cast<std::size_t>(binding_size-1));
    CHECK(std::string(pixaura::agent_schema_fixtures::cases[0].canonical).find("\"binding\":"+binding)!=std::string::npos);
    pixaura_agent_schema_request r{};r.api_version=1;r.struct_size=sizeof(r);r.kind=2;
    const std::string valid=pixaura::agent_schema_fixtures::cases[1].canonical;
    auto call=[&](const std::string& input,int32_t expected){
        std::array<uint8_t,8192> output;output.fill(0x5a);uint64_t required=777;
        const auto result=pixaura_agent_schema_canonicalize(1,&a.context,&a.base,&r,b(input),input.size(),output.data(),output.size(),&required);
        if(result!=expected)std::fprintf(stderr,"expected %d got %d input %.120s\n",expected,result,input.c_str());
        CHECK(result==expected);
        if(result){CHECK(required==777);for(auto c:output)CHECK(c==0x5a);}
    };
    call("",8);call("{",6);call(std::string(8193,' '),8);
    call(replace(valid,"\"plan_version\":1","\"plan_version\":2"),2);
    call(replace(valid,"pixaura.exposure","pixaura.unknown"),5);
    call(replace(valid,"\"operation_version\":1","\"operation_version\":2"),5);
    call(replace(valid,"\"parameter_version\":1","\"parameter_version\":2"),5);
    call(replace(valid,"\"milli_ev\":1000","\"milli_ev\":5001"),7);
    call(replace(valid,"\"milli_ev\":1000","\"milli_ev\":1.5"),6);
    call(replace(valid,"\"milli_ev\":1000","\"milli_ev\":1000,\"milli_ev\":0"),6);
    call(replace(valid,"\"milli_ev\":1000","\"milli_ev\":1000,\"unknown\":0"),7);
    call(replace(valid,"\"plan_version\":1","\"plan_version\":1,\"approved\":true"),6);
    call(replace(valid,"\"plan_version\":1","\"plan_version\":1,\"plan_version\":1"),6);
    call(replace(valid,",\"plan_version\":1",""),6);
    call(replace(valid,"\"action_index\":0","\"action_index\":1"),6);
    call(replace(valid,"\"action_index\":0","\"action_index\":-1"),6);
    call(replace(valid,"\"generation\":0","\"generation\":18446744073709551616"),6);
    call(replace(valid,"\"generation\":0","\"generation\":-1"),6);
    call(replace(valid,"\"generation\":0","\"generation\":1"),9);
    call(replace(valid,"00000000000000000000000000000002","bad"),6);
    call(replace(valid,"00000000000000000000000000000102","00000000000000000000000000000103"),9);
    call(valid+"garbage",6);call(valid.substr(0,valid.size()-3),6);
    call("{\"actions\":[[[[]]]]}\n",6);
    r.require_canonical=1;call(" "+valid,6);call(valid,0);r.require_canonical=0;
    r.kind=1;const std::string intent=pixaura::agent_schema_fixtures::cases[0].canonical;
    call(intent,0);call(replace(intent,"S\xc3\xa1ng h\xc6\xa1n",std::string(257,'a')),8);
    call(replace(intent,"S\xc3\xa1ng h\xc6\xa1n",std::string(256,'a')),0);
    call(replace(intent,"S\xc3\xa1ng h\xc6\xa1n","S\\u00e1ng h\\u01a1n"),0);
    call(replace(intent,"S\xc3\xa1ng h\xc6\xa1n",std::string("\xc0\xaf",2)),6);
    call(replace(intent,"S\xc3\xa1ng h\xc6\xa1n","\\ud800"),6);
    call(replace(intent,"S\xc3\xa1ng h\xc6\xa1n","\\u0000"),6);
    call(replace(intent,"S\xc3\xa1ng h\xc6\xa1n",""),6);
    call(replace(intent,"\"type\":\"edit\"","\"type\":\"generative\""),6);
    r.pre_cancel=1;call(intent,13);r.pre_cancel=0;
    r.reserved[1]=1;call(intent,1);r.reserved[1]=0;
    r.kind=3;call(intent,1);r.kind=1;
    r.api_version=2;call(intent,2);r.api_version=1;
    r.struct_size=31;call(intent,1);r.struct_size=sizeof(r);
    r.require_canonical=2;call(intent,1);r.require_canonical=0;
    std::array<uint8_t,8192> output{};uint64_t required=777;
    CHECK(pixaura_agent_schema_canonicalize(2,&a.context,&a.base,&r,b(intent),intent.size(),nullptr,0,&required)==2);
    CHECK(pixaura_agent_schema_canonicalize(1,&other.context,&other.base,&r,b(intent),intent.size(),nullptr,0,&required)==9);
    CHECK(required==777);
    CHECK(pixaura_agent_schema_canonicalize(1,&a.context,&other.base,&r,b(intent),intent.size(),nullptr,0,&required)==3);
    CHECK(pixaura_agent_schema_canonicalize(1,&a.context,&a.base,&r,b(intent),intent.size(),output.data(),output.size(),reinterpret_cast<uint64_t*>(output.data()))==1);
    CHECK(pixaura_agent_schema_canonicalize(1,&a.context,&a.base,&r,b(intent),intent.size(),output.data(),UINT64_MAX,&required)==1);
    CHECK(pixaura_agent_schema_canonicalize(1,&a.context,&a.base,&r,b(intent),intent.size(),nullptr,0,&required)==11 && required==intent.size());
    CHECK(pixaura_agent_schema_canonicalize(1,&a.context,&a.base,&r,b(intent),intent.size(),output.data(),1,&required)==11 && output[0]==0);
    Hook hook{&a,1};CHECK(pixaura_agent_schema_test_checkpoint(&a.context,checkpoint,&hook)==0);
    call(intent,13); // precise cancellation AFTER private parse, BEFORE publication
    CHECK(pixaura_agent_schema_test_checkpoint(&a.context,nullptr,nullptr)==0);
    call(intent,0); // recovery after cancellation
    hook.mode=3;CHECK(pixaura_agent_schema_test_checkpoint(&a.context,checkpoint,&hook)==0);
    call(intent,0);CHECK(pixaura_agent_schema_test_checkpoint(&a.context,nullptr,nullptr)==0);
    CHECK(dump(a)==original && dump(other)==other_original); // ZERO document/history effects
    hook.owner=&other;hook.mode=1;CHECK(pixaura_agent_schema_test_checkpoint(&a.context,checkpoint,&hook)==0);
    call(intent,0);CHECK(pixaura_agent_schema_test_checkpoint(&a.context,nullptr,nullptr)==0); // cancel isolation
    hook.owner=&a;hook.mode=2;CHECK(pixaura_agent_schema_test_checkpoint(&a.context,checkpoint,&hook)==0);
    call(intent,9); // released live token cannot publish
    CHECK(pixaura_agent_schema_test_checkpoint(&a.context,nullptr,nullptr)==0);
    CHECK(pixaura_agent_schema_canonicalize(1,&a.context,&a.base,&r,b(intent),intent.size(),nullptr,0,&required)==3);
    std::printf("Agent intent/plan v1 checks=%u failures=0; canonical/security/context/cancellation PASS; side_effects=0\n",checks);
}
