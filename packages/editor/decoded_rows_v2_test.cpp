#include "../core/src/v2/integration/decoded_rows_v2.hpp"
#include <filesystem>
#include <future>
#include <cstdio>
#include <stdexcept>
using namespace pixaura;
namespace fs=std::filesystem;
#define CHECK(x) do{if(!(x))throw std::runtime_error("decoded rows line "+std::to_string(__LINE__));}while(false)
template<class F>void fails(int code,F call){bool caught=false;try{call();}catch(const document::Failure& e){CHECK(e.code==code);caught=true;}CHECK(caught);}
static std::array<uint8_t,4> expected(uint32_t x,uint32_t y){return {static_cast<uint8_t>(x*13+y*7),static_cast<uint8_t>(x*3+y*11),static_cast<uint8_t>(x+y*5),static_cast<uint8_t>(x^y)};}
int main(){fs::path root;try{
    root=fs::current_path()/("decoded-rows-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));CHECK(fs::create_directory(root));fs::permissions(root,fs::perms::owner_all,fs::perm_options::replace);
    tiled::Budget budget(64u<<20,0);constexpr uint32_t width=2049,height=257;
    // Invalid roots/geometry and reservation failure never expose scratch files.
    fails(12,[&]{integration::DecodedRowsV2 missing((root/"missing").string(),1,1,budget);});
    fails(8,[&]{integration::DecodedRowsV2 oversized(root.string(),16385,1,budget);});
    CHECK(fs::directory_iterator(root)==fs::directory_iterator());
#ifndef _WIN32
    const auto link=root.parent_path()/(root.filename().string()+"-link");fs::create_directory_symlink(root,link);
    fails(12,[&]{integration::DecodedRowsV2 escaped(link.string(),1,1,budget);});CHECK(fs::remove(link));
#endif
    {
        tiled::Budget tiny(1,0);integration::DecodedRowsV2 limited(root.string(),1,1,tiny);
        uint8_t raw[4]={1,2,3,4};limited.row(0,raw,4,true);limited.seal();
        fails(8,[&]{limited.pixel(0,0,raw);});CHECK(tiny.inflight()==0);
        CHECK(raw[0]==1&&raw[1]==2&&raw[2]==3&&raw[3]==4);
    }
    CHECK(fs::directory_iterator(root)==fs::directory_iterator());

    {
        auto metadata=budget.reserve(65536);integration::DecodedRowsV2 store(root.string(),width,height,budget);
        std::array<uint8_t,4> pixel{};fails(19,[&]{store.pixel(0,0,pixel.data());});
        std::vector<uint8_t> row(size_t(width)*4);
        for(uint32_t y=0;y<height;++y){for(uint32_t x=0;x<width;++x){const auto p=expected(x,y);std::memcpy(row.data()+size_t(x)*4,p.data(),4);}store.row(y,row.data(),row.size(),true);}
        store.seal();fails(19,[&]{store.row(0,row.data(),row.size(),true);});fails(19,[&]{store.pixel(width,0,pixel.data());});
#ifndef _WIN32
        CHECK(fs::directory_iterator(root)==fs::directory_iterator()); // unlinked BEFORE any pixel is written
#endif
        auto check=[&](unsigned seed){for(uint32_t i=0;i<10000;++i){const auto x=(i*131u+seed*47u)%width,y=(i*53u+seed*19u)%height;std::array<uint8_t,4> actual{};store.pixel(x,y,actual.data());CHECK(actual==expected(x,y));}};
        std::vector<std::future<void>> threads;for(unsigned i=0;i<4;++i)threads.push_back(std::async(std::launch::async,check,i));for(auto& work:threads)work.get();
        CHECK(budget.inflight()<=17u*65536&&budget.peak()<=17u*65536);
    }
    CHECK(budget.inflight()==0&&fs::directory_iterator(root)==fs::directory_iterator());CHECK(fs::remove(root));
    std::puts("decoded row store v2: exact random/edge/concurrent RGBA;16-tile bound; anonymous/delete-on-close cleanup; no leases left PASS");return 0;
}catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}catch(const document::Failure& e){std::fprintf(stderr,"decoded rows native%d\n",e.code);return 1;}}
