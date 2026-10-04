#include "pixaura/decode.h"
#include "pixaura/working.h"
#include "../src/storage.hpp"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <new>
#include <filesystem>
static thread_local int fail_after=-1;
void* operator new(std::size_t n){if(fail_after==0)throw std::bad_alloc();if(fail_after>0)--fail_after;if(auto p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p)noexcept{std::free(p);}
void operator delete[](void* p)noexcept{std::free(p);}
void operator delete(void* p,std::size_t)noexcept{std::free(p);}
void operator delete[](void* p,std::size_t)noexcept{std::free(p);}
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"decode allocation check %d\n",__LINE__);std::_Exit(1);}}while(false)
struct Reader:pixaura::storage::Reader{std::size_t at=0;std::size_t read(uint8_t* p,std::size_t n)override{const auto bytes=std::min(n,sizeof(decode_png)-at);std::memcpy(p,decode_png+at,bytes);at+=bytes;return bytes;}};
int main(int argc,char** argv){
    CHECK(argc==2);const auto root=std::filesystem::absolute(std::filesystem::path(argv[1])/"decode-allocation-root");std::filesystem::create_directories(root);
    const auto path=root.generic_string();pixaura::storage::AssetStore store(path);Reader reader;const auto asset=store.ingest(reader,sizeof(decode_png));
    pixaura_decode_limits l{};CHECK(pixaura_decode_default_limits(1,&l)==0);const uint8_t id[]="dddddddddddddddddddddddddddddddd";unsigned failures=0;
    for(int phase=0;phase<5;++phase){bool success=false;
        for(int n=0;n<512;++n){pixaura_decode_context c{};pixaura_decode_handle source{},image{},working{},out{},sentinel;std::memset(&out,0x5a,sizeof(out));sentinel=out;
            if(phase>0)CHECK(pixaura_decode_context_init(1,&c,sizeof(c),id,32,&l)==0);
            if(phase>1)CHECK(pixaura_decode_open(&c,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(asset.digest.data()),64,asset.bytes,&source)==0);
            pixaura_working_limits wl{};CHECK(pixaura_working_default_limits(1,&wl)==0);
            if(phase>2)CHECK(pixaura_decode_image(&c,&source,&image)==0);
            if(phase>3)CHECK(pixaura_working_normalize(&c,&image,&wl,&working)==0);
            fail_after=n;int32_t status;
            if(phase==0)status=pixaura_decode_context_init(1,&c,sizeof(c),id,32,&l);
            else if(phase==1)status=pixaura_decode_open(&c,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(asset.digest.data()),64,asset.bytes,&out);
            else if(phase==2)status=pixaura_decode_image(&c,&source,&out);
            else if(phase==3)status=pixaura_working_normalize(&c,&image,&wl,&out);
            else status=pixaura_working_identity(&c,&working,&wl,&out);
            fail_after=-1;
            if(status==0)success=true;else{CHECK(status==8);++failures;if(phase==0){const pixaura_decode_context zero{};CHECK(std::memcmp(&c,&zero,sizeof(c))==0);}else CHECK(std::memcmp(&out,&sentinel,sizeof(out))==0);}
            if(phase>0||status==0){if(status!=0&&phase>0){pixaura_decode_handle valid{};if(phase==1)CHECK(pixaura_decode_open(&c,reinterpret_cast<const uint8_t*>(path.data()),path.size(),reinterpret_cast<const uint8_t*>(asset.digest.data()),64,asset.bytes,&valid)==0);else if(phase==2)CHECK(pixaura_decode_image(&c,&source,&valid)==0);else if(phase==3)CHECK(pixaura_working_normalize(&c,&image,&wl,&valid)==0);else CHECK(pixaura_working_identity(&c,&working,&wl,&valid)==0);}CHECK(pixaura_decode_context_destroy(&c)==0);}
            if(success)break;
        }CHECK(success);
    }
    CHECK(failures>0);std::printf("decode C++ allocation sweep failures recovered=%u; phases=5\n",failures);
}
