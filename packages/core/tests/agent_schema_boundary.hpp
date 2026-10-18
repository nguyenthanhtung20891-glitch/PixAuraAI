#ifndef PIXAURA_AGENT_SCHEMA_BOUNDARY_HPP
#define PIXAURA_AGENT_SCHEMA_BOUNDARY_HPP
#include "agent_schema_fixtures.hpp"
#include "pixaura/agent_schema.h"
#include "pixaura/agent_tools.h"
#include "agent_tools_fixture.hpp"
#include "pixaura/reference_planner.h"
#include "reference_planner_fixtures.hpp"
#include <array>
#include <cstdio>
#include <cstring>
#include <string>
extern "C" int32_t pixaura_agent_schema_test_boundary(){
    using namespace pixaura::agent_schema_fixtures;
    pixaura_document_context context{};pixaura_document_handle base{};
    const uint8_t identity[]="000000000000000000000000000000a1",session[]="00000000000000000000000000000500";
    if(pixaura_document_context_init(1,&context,sizeof(context),identity,32,nullptr))return 1;
    struct Cleanup{pixaura_document_context* c;~Cleanup(){(void)pixaura_document_context_destroy(c);}} cleanup{&context};
#define SCHEMA_CHECK(x) do{if(!(x)){std::printf("Agent schema fixture failure line %d\n",__LINE__);return 1;}}while(false)
    SCHEMA_CHECK(pixaura_document_open(1,&context,reinterpret_cast<const uint8_t*>(document),std::strlen(document),session,32,&base,nullptr)==0);
    std::array<uint8_t,32768> registry{};uint64_t registry_size=0;
    SCHEMA_CHECK(pixaura_agent_tools_registry(1,1,registry.data(),registry.size(),&registry_size)==0);
    SCHEMA_CHECK(registry_size==std::strlen(agent_tools_golden) && std::memcmp(registry.data(),agent_tools_golden,registry_size)==0);
    for(const auto& f:cases){
        pixaura_agent_schema_request request{};request.api_version=1;request.struct_size=sizeof(request);request.kind=f.kind;
        std::array<uint8_t,8192> out{};out.fill(0x5a);uint64_t size=999;
        const auto result=pixaura_agent_schema_canonicalize(1,&context,&base,&request,
            reinterpret_cast<const uint8_t*>(f.input),std::strlen(f.input),out.data(),out.size(),&size);
        SCHEMA_CHECK(result==f.status);
        if(f.kind==2){
            request.require_canonical=1;
            std::array<uint8_t,9216> receipt{};uint64_t receipt_size=999;
            const auto bound=pixaura_agent_tools_bind(1,1,&context,&base,&request,
                reinterpret_cast<const uint8_t*>(f.canonical),std::strlen(f.canonical),receipt.data(),receipt.size(),&receipt_size);
            SCHEMA_CHECK(bound==f.status);
            if(!bound){
                const std::string expected=std::string("{\"approval\":\"not_approved\",\"binding_version\":1,\"plan\":")+
                    std::string(f.canonical,std::strlen(f.canonical)-1)+",\"registry_version\":1}\n";
                SCHEMA_CHECK(receipt_size==expected.size() && std::memcmp(receipt.data(),expected.data(),receipt_size)==0);
            }else SCHEMA_CHECK(receipt_size==999);
            request.require_canonical=0;
        }
        if(result){SCHEMA_CHECK(size==999 && out[0]==0x5a);continue;}
        SCHEMA_CHECK(size==std::strlen(f.canonical) && std::memcmp(out.data(),f.canonical,size)==0 && out[size]==0x5a);
        request.require_canonical=1;
        std::array<uint8_t,8192> again{};uint64_t n=0;
        SCHEMA_CHECK(pixaura_agent_schema_canonicalize(1,&context,&base,&request,out.data(),size,again.data(),again.size(),&n)==0);
        SCHEMA_CHECK(n==size && std::memcmp(out.data(),again.data(),size)==0);
    }
    for(const auto& f:pixaura::reference_planner_fixtures::cases){
        pixaura_reference_planner_request r{};r.api_version=1;r.struct_size=sizeof(r);r.planner_version=1;r.registry_version=1;
        std::array<uint8_t,2048> canonical{};uint64_t count=0;
        SCHEMA_CHECK(pixaura_structured_intent_canonicalize(1,&context,&base,&r,
            reinterpret_cast<const uint8_t*>(f.input),std::strlen(f.input),canonical.data(),canonical.size(),&count)==0);
        SCHEMA_CHECK(count==std::strlen(f.input) && std::memcmp(canonical.data(),f.input,count)==0);
        for(unsigned repeat=0;repeat<3;++repeat){
            std::array<uint8_t,8192> plan{};uint64_t size=0;
            SCHEMA_CHECK(pixaura_reference_planner_plan(1,&context,&base,&r,canonical.data(),count,plan.data(),plan.size(),&size)==0);
            SCHEMA_CHECK(size==std::strlen(f.plan) && std::memcmp(plan.data(),f.plan,size)==0);
            pixaura_agent_schema_request check{};check.api_version=1;check.struct_size=sizeof(check);check.kind=2;check.require_canonical=1;
            std::array<uint8_t,9216> receipt{};uint64_t receipt_size=0;
            SCHEMA_CHECK(pixaura_agent_tools_bind(1,1,&context,&base,&check,plan.data(),size,receipt.data(),receipt.size(),&receipt_size)==0);
        }
    }
    std::puts("Reference planner v1 goldens=39 repeats=3 failures=0; structured/plan bytes exact; Step 3 binding PASS");
    std::puts("Agent schema v1 goldens=7 valid=6 failures=0 canonical_bytes=exact; no execution/approval");
    std::puts("Agent tools v1 registry=11 exact; plan binding goldens=6 failures=0; NOT_APPROVED; no effects");
    return 0;
#undef SCHEMA_CHECK
}
#endif
