#include "pixaura/auto_proposal.h"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"phase11 security line %d: %s\n",__LINE__,#x);std::abort();}}while(false)
static int64_t reader(void* state,uint8_t* out,uint64_t capacity){
 auto& at=*static_cast<std::size_t*>(state);const auto n=std::min<uint64_t>(capacity,sizeof(decode_png)-at);
 std::memcpy(out,decode_png+at,static_cast<std::size_t>(n));at+=static_cast<std::size_t>(n);return static_cast<int64_t>(n);
}
static std::string state(pixaura_editor_context& owner){
 uint64_t n=0;CHECK(pixaura_editor_state(&owner,nullptr,0,&n)==0);std::string out(static_cast<std::size_t>(n),' ');
 CHECK(pixaura_editor_state(&owner,reinterpret_cast<uint8_t*>(out.data()),n,&n)==0);return out;
}
int main(int argc,char** argv){
 CHECK(argc==2);const auto root=std::filesystem::path(argv[1])/("phase11-fuzz-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
 CHECK(std::filesystem::create_directory(root));const auto path=root.generic_string();const std::string identity(32,'1');
 pixaura_editor_context owner{};std::size_t at=0;
 CHECK(pixaura_editor_open(1,&owner,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(identity.data()),identity.size(),reader,&at,sizeof(decode_png))==0);
 const auto original=state(owner);const pixaura_auto_request structured{1,sizeof(pixaura_auto_request),2,2};pixaura_auto_proposal seed{};
 CHECK(pixaura_editor_auto_request(1,&owner,&structured,nullptr,0,&seed)==0);uint64_t size=0;
 CHECK(pixaura_editor_auto_plan(1,&owner,&seed,nullptr,0,&size)==11);std::vector<uint8_t> plan(static_cast<std::size_t>(size));
 CHECK(pixaura_editor_auto_plan(1,&owner,&seed,plan.data(),size,&size)==0);CHECK(pixaura_editor_auto_cancel(1,&owner)==0);
 const pixaura_auto_request request{1,sizeof(pixaura_auto_request),0,3};unsigned accepted=0,rejected=0;uint32_t rng=0x50483131;
 auto next=[&](){rng=rng*1664525u+1013904223u;return rng;};
 auto probe=[&](const std::vector<uint8_t>& input){
  pixaura_auto_proposal output{};std::memset(&output,0xa5,sizeof(output));const auto sentinel=output;
  const auto code=pixaura_editor_auto_request(1,&owner,&request,input.empty()?nullptr:input.data(),input.size(),&output);
  if(code==0){++accepted;CHECK(output.action_count>=1&&output.action_count<=8);CHECK(pixaura_editor_auto_current(1,&owner,&output)==0);}
  else {++rejected;CHECK(std::memcmp(&output,&sentinel,sizeof(output))==0);}
  CHECK(state(owner)==original);CHECK(pixaura_editor_auto_cancel(1,&owner)==0);CHECK(state(owner)==original);
 };
 probe(plan);
 for(unsigned i=0;i<4096;++i){auto input=plan;const auto pos=static_cast<std::size_t>(next())%input.size();
  switch(i%4){case 0:input[pos]^=static_cast<uint8_t>((next()>>16)|1u);break;case 1:input.resize(pos);break;
   case 2:input.insert(input.begin()+static_cast<std::ptrdiff_t>(pos),static_cast<uint8_t>(next()>>24));break;
   default:input[pos]=static_cast<uint8_t>(next()>>24);break;}
  probe(input);
 }
 for(unsigned i=0;i<64;++i){std::vector<uint8_t> input(8192);for(auto& byte:input)byte=static_cast<uint8_t>(next()>>24);probe(input);}
 probe(std::vector<uint8_t>(8193,'x'));probe({});CHECK(accepted>0&&rejected>0);
 // Invalid requests never acquire approval. Manual must remain usable afterwards.
 CHECK(pixaura_editor_select(&owner,4)==0);const int32_t value=100;CHECK(pixaura_editor_update(&owner,&value,1)==0);
 CHECK(pixaura_editor_action(&owner,0)==0);CHECK(state(owner)==original);
 CHECK(pixaura_editor_stop(&owner)==0);CHECK(pixaura_editor_destroy(&owner)==0);std::filesystem::remove_all(root);
 std::printf("phase11 seed=0x50483131 mutations=4096 random_8192=64 boundary_cases=2 accepted=%u rejected=%u checks=%u failures=0; no revision/approval/publication\n",accepted,rejected,checks);
}
