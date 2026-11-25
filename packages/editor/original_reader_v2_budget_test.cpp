#include "../core/src/v2/integration/original_reader_v2.hpp"
#include "../core/src/storage_files.hpp"
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <cstdio>
#include <cstring>
using namespace pixaura;
#define CHECK(x) do{if(!(x)){std::fprintf(stderr,"source budget check %d\n",__LINE__);return 1;}}while(false)
struct Input final:storage::Reader {
    std::ifstream file;
    explicit Input(const char* path):file(path,std::ios::binary){}
    size_t read(uint8_t* out,size_t count)override{file.read(reinterpret_cast<char*>(out),static_cast<std::streamsize>(count));return static_cast<size_t>(file.gcount());}
};
int main(int argc,char** argv){try{
    CHECK(argc==4);const bool oracle=std::strcmp(argv[1],"--oracle")==0;
    const bool low=std::strcmp(argv[1],"--verify-low")==0;
    CHECK(oracle||low||std::strcmp(argv[1],"--verify")==0);
    storage::Sha256 digest;
    if(oracle){
        // A separate reference PROCESS, not production admission or RSS evidence.
        // Frozen codec + bounded v1 normalization windows produce expected bits.
        std::ifstream input(argv[2],std::ios::binary|std::ios::ate);CHECK(input);const auto size=input.tellg();CHECK(size>0);input.seekg(0);
        decode::Vector<uint8_t> encoded(static_cast<size_t>(size));CHECK(input.read(reinterpret_cast<char*>(encoded.data()),size));
        auto metadata=decode::admit(encoded.data(),encoded.size(),decode::defaults());decode::Source source(encoded,std::move(metadata));
        auto pixels=decode::execute(source,decode::defaults());CHECK(source.metadata().value.width==4000&&source.metadata().value.height==3000&&pixels->size()==48000000);
        for(uint32_t y=0;y<3000;y+=128)for(uint32_t x=0;x<4000;x+=128){
            const auto width=std::min(128u,4000-x),height=std::min(128u,3000-y);std::vector<uint8_t> tile(size_t(width)*height*4);
            for(uint32_t row=0;row<height;++row)std::memcpy(tile.data()+size_t(row)*width*4,pixels->data()+(size_t(y+row)*4000+x)*4,size_t(width)*4);
            auto m=source.metadata();auto& v=m.value;v.width=v.display_width=width;v.height=v.display_height=height;v.row_stride=uint64_t(width)*4;v.decoded_bytes=v.row_stride*height;
            auto normalized=working::normalize(m,tile.data(),tile.size(),working::defaults());
            digest.update(reinterpret_cast<const uint8_t*>(normalized.pixels->data()),normalized.pixels->size()*sizeof(float));
        }
        std::ofstream output(argv[3],std::ios::binary|std::ios::trunc);output<<digest.finish()<<'\n';CHECK(output);
        std::puts("Independent frozen decoder/normalizer source oracle PASS; not a production memory observation");return 0;
    }
    const auto root=(std::filesystem::current_path()/("source-v2-budget-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()))).string();CHECK(std::filesystem::create_directory(root));
    struct Cleanup{std::string root;~Cleanup(){std::error_code error;std::filesystem::remove_all(root,error);}} cleanup{root};
    Input input(argv[2]);CHECK(input.file);storage::AssetStore assets(root);const auto asset=assets.ingest(input,std::filesystem::file_size(argv[2]));
    const uint64_t cap=low?67108864:134217728;
    tiled::Budget budget(cap,0);uint64_t scratch=0;
    {
        auto context=budget.reserve(9ULL<<20);storage::Files files(root);
        auto source=integration::OriginalReaderV2::from_verified(files,asset,budget);CHECK(source->width()==4000&&source->height()==3000&&source->digest()==asset.digest);
        CHECK(source->uses_row_store()==low);
        scratch=source->decode_scratch_bytes();CHECK(scratch>0&&scratch<=decode::defaults().scratch_bytes);
        auto comparison=budget.reserve(1048576);
        for(uint32_t y=0;y<3000;y+=128)for(uint32_t x=0;x<4000;x+=128){
            const auto width=std::min(128u,4000-x),height=std::min(128u,3000-y);std::vector<float> tile(size_t(width)*height*4);
            for(uint32_t row=0;row<height;++row)for(uint32_t column=0;column<width;++column)source->read(x+column,y+row,tile.data()+(size_t(row)*width+column)*4);
            digest.update(reinterpret_cast<const uint8_t*>(tile.data()),tile.size()*sizeof(float));
        }
        std::ifstream expected(argv[3]);std::string hash;CHECK(std::getline(expected,hash));CHECK(hash.size()==64&&digest.finish()==hash);
    }
    CHECK(budget.inflight()==0&&budget.peak()<=cap);
    std::printf("Managed high-entropy 12MP source all48000000 normalized float values bitwise PASS; cap=%llu scratch=%llu peak_reserved=%llu leaked_leases=0; host only\n",static_cast<unsigned long long>(cap),static_cast<unsigned long long>(scratch),static_cast<unsigned long long>(budget.peak()));return 0;
}catch(const document::Failure& failure){std::fprintf(stderr,"source budget failure=%d\n",failure.code);return 1;}}
