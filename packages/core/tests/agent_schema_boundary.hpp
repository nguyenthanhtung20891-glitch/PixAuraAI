#ifndef PIXAURA_AGENT_SCHEMA_BOUNDARY_HPP
#define PIXAURA_AGENT_SCHEMA_BOUNDARY_HPP
#include "agent_schema_fixtures.hpp"
#include "pixaura/agent_schema.h"
#include <array>
#include <cstdio>
#include <cstring>
extern "C" int32_t pixaura_agent_schema_test_boundary(){
    using namespace pixaura::agent_schema_fixtures;
    pixaura_document_context context{};pixaura_document_handle base{};
    const uint8_t identity[]="000000000000000000000000000000a1",session[]="00000000000000000000000000000500";
    if(pixaura_document_context_init(1,&context,sizeof(context),identity,32,nullptr))return 1;
    struct Cleanup{pixaura_document_context* c;~Cleanup(){(void)pixaura_document_context_destroy(c);}} cleanup{&context};
#define SCHEMA_CHECK(x) do{if(!(x)){std::printf("Agent schema fixture failure line %d\n",__LINE__);return 1;}}while(false)
    SCHEMA_CHECK(pixaura_document_open(1,&context,reinterpret_cast<const uint8_t*>(document),std::strlen(document),session,32,&base,nullptr)==0);
    for(const auto& f:cases){
        pixaura_agent_schema_request request{};request.api_version=1;request.struct_size=sizeof(request);request.kind=f.kind;
        std::array<uint8_t,8192> out{};out.fill(0x5a);uint64_t size=999;
        const auto result=pixaura_agent_schema_canonicalize(1,&context,&base,&request,
            reinterpret_cast<const uint8_t*>(f.input),std::strlen(f.input),out.data(),out.size(),&size);
        SCHEMA_CHECK(result==f.status);
        if(result){SCHEMA_CHECK(size==999 && out[0]==0x5a);continue;}
        SCHEMA_CHECK(size==std::strlen(f.canonical) && std::memcmp(out.data(),f.canonical,size)==0 && out[size]==0x5a);
        request.require_canonical=1;
        std::array<uint8_t,8192> again{};uint64_t n=0;
        SCHEMA_CHECK(pixaura_agent_schema_canonicalize(1,&context,&base,&request,out.data(),size,again.data(),again.size(),&n)==0);
        SCHEMA_CHECK(n==size && std::memcmp(out.data(),again.data(),size)==0);
    }
    std::puts("Agent schema v1 goldens=7 valid=6 failures=0 canonical_bytes=exact; no execution/approval");
    return 0;
#undef SCHEMA_CHECK
}
#endif
