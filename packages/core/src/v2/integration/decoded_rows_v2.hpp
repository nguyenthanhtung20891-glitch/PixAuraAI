#ifndef PIXAURA_DECODED_ROWS_V2_HPP
#define PIXAURA_DECODED_ROWS_V2_HPP
#include "../../tiled_cpu.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <mutex>
#include <cstring>
#include <algorithm>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cerrno>
#endif
namespace pixaura::integration {
// Anonymous/delete-on-close scratch only. No source/revision/approval authority.
// Encoded originals stay immutable in the existing managed asset store.
class DecodedRowsV2 final {
    static void need(bool ok,int code=12){if(!ok)throw document::Failure{code};}
#ifdef _WIN32
    HANDLE file_=INVALID_HANDLE_VALUE;
#else
    int file_=-1;
#endif
    uint32_t width_,height_;uint64_t bytes_;
    tiled::Budget& budget_;mutable std::mutex cache_gate_,file_gate_;
    bool sealed_=false;
    struct Tile {uint32_t x=0,y=0;uint64_t age=0;bool valid=false;std::vector<uint8_t> bytes;std::unique_ptr<tiled::Budget::Lease> lease;};
    mutable std::array<Tile,16> tiles_{};mutable size_t hot_=0;mutable uint64_t age_=0;
    void transfer(uint64_t offset,uint8_t* data,size_t count,bool writing)const{
        need(offset<=bytes_&&count<=bytes_-offset,19);std::lock_guard<std::mutex> lock(file_gate_);
        while(count){
#ifdef _WIN32
            LARGE_INTEGER position;position.QuadPart=static_cast<LONGLONG>(offset);need(SetFilePointerEx(file_,position,nullptr,FILE_BEGIN)!=0);
            DWORD done=0;const auto chunk=static_cast<DWORD>(std::min<size_t>(count,65536));
            need((writing?WriteFile(file_,data,chunk,&done,nullptr):ReadFile(file_,data,chunk,&done,nullptr))!=0&&done!=0);
            const auto n=static_cast<size_t>(done);
#else
            const auto chunk=std::min<size_t>(count,65536);
            const auto done=writing?::pwrite(file_,data,chunk,static_cast<off_t>(offset)) : ::pread(file_,data,chunk,static_cast<off_t>(offset));
            if(done<0&&errno==EINTR)continue;need(done>0);const auto n=static_cast<size_t>(done);
#endif
            data+=n;count-=n;offset+=n;
        }
    }
    void discard()noexcept{
#ifdef _WIN32
        if(file_!=INVALID_HANDLE_VALUE){CloseHandle(file_);file_=INVALID_HANDLE_VALUE;}
#else
        if(file_>=0){::close(file_);file_=-1;}
#endif
    }
public:
    DecodedRowsV2(const std::string& root,uint32_t width,uint32_t height,tiled::Budget& budget):width_(width),height_(height),bytes_(uint64_t(width)*height*4),budget_(budget){
        need(width&&height&&width<=16384&&height<=16384&&bytes_<=decode::defaults().decoded_bytes,8);
        static std::atomic<uint64_t> serial{0};
#ifdef _WIN32
        const auto root_path=std::filesystem::u8path(root);const auto attributes=GetFileAttributesW(root_path.c_str());
        need(attributes!=INVALID_FILE_ATTRIBUTES&&(attributes&FILE_ATTRIBUTE_DIRECTORY)&&!(attributes&FILE_ATTRIBUTE_REPARSE_POINT));
#endif
#ifndef _WIN32
        const int directory=::open(root.c_str(),O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);need(directory>=0);
#endif
        try{
            for(unsigned attempt=0;attempt<16;++attempt){
                const auto sequence=serial.fetch_add(1);need(sequence!=UINT64_MAX,8);
                const auto token=std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())+"-"+std::to_string(sequence);
                const auto name=".pixaura-row-v2-"+token;
#ifdef _WIN32
                const auto path=std::filesystem::u8path(root)/std::filesystem::u8path(name);
                file_=CreateFileW(path.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_TEMPORARY|FILE_FLAG_DELETE_ON_CLOSE|FILE_FLAG_RANDOM_ACCESS,nullptr);
                if(file_==INVALID_HANDLE_VALUE){need(GetLastError()==ERROR_FILE_EXISTS||GetLastError()==ERROR_ALREADY_EXISTS);continue;}
                LARGE_INTEGER length;length.QuadPart=static_cast<LONGLONG>(bytes_);need(SetFilePointerEx(file_,length,nullptr,FILE_BEGIN)!=0&&SetEndOfFile(file_)!=0);
#else
                file_=::openat(directory,name.c_str(),O_RDWR|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
                if(file_<0){need(errno==EEXIST);continue;}
                // No crash-visible cache and no later path-based access.
                if(::unlinkat(directory,name.c_str(),0)!=0){discard();throw document::Failure{12};}
                need(::ftruncate(file_,static_cast<off_t>(bytes_))==0);
#endif
                break;
            }
#ifdef _WIN32
            need(file_!=INVALID_HANDLE_VALUE);
#else
            need(file_>=0);::close(directory);
#endif
        }catch(...){discard();
#ifndef _WIN32
            ::close(directory);
#endif
            throw;
        }
    }
    ~DecodedRowsV2(){discard();}
    DecodedRowsV2(const DecodedRowsV2&)=delete;
    void row(uint32_t y,uint8_t* data,size_t count,bool writing){
        need(!sealed_&&y<height_&&count==size_t(width_)*4,19);transfer(uint64_t(y)*count,data,count,writing);
    }
    void seal(){
        std::lock_guard<std::mutex> lock(cache_gate_);need(!sealed_,19);
#ifdef _WIN32
        LARGE_INTEGER size;need(GetFileSizeEx(file_,&size)!=0&&size.QuadPart==static_cast<LONGLONG>(bytes_),19);
#else
        struct stat value{};need(::fstat(file_,&value)==0&&S_ISREG(value.st_mode)&&value.st_nlink==0&&static_cast<uint64_t>(value.st_size)==bytes_,19);
#endif
        sealed_=true;
    }
    void pixel(uint32_t x,uint32_t y,uint8_t* output)const{
        std::lock_guard<std::mutex> lock(cache_gate_);need(sealed_&&x<width_&&y<height_&&output,19);
        const auto tx=x/128,ty=y/128;
        size_t at=hot_;
        if(!tiles_[at].valid||tiles_[at].x!=tx||tiles_[at].y!=ty){
            at=tiles_.size();for(size_t i=0;i<tiles_.size();++i)if(tiles_[i].valid&&tiles_[i].x==tx&&tiles_[i].y==ty){at=i;break;}
            if(at==tiles_.size()){
                at=0;for(size_t i=0;i<tiles_.size();++i){if(!tiles_[i].valid){at=i;break;}if(tiles_[i].age<tiles_[at].age)at=i;}
                auto& tile=tiles_[at];tile.valid=false;
                if(tile.bytes.empty()){
                    tile.lease.reset(new tiled::Budget::Lease(budget_.reserve(128u*128*4)));
                    try{tile.bytes.resize(128u*128*4);need(tile.bytes.capacity()==128u*128*4,8);}
                    catch(...){std::vector<uint8_t>().swap(tile.bytes);tile.lease.reset();throw;}
                }
                std::fill(tile.bytes.begin(),tile.bytes.end(),static_cast<unsigned char>(0));
                const auto start_x=tx*128,start_y=ty*128,count_x=std::min(128u,width_-start_x),count_y=std::min(128u,height_-start_y);
                for(uint32_t row=0;row<count_y;++row)transfer((uint64_t(start_y+row)*width_+start_x)*4,tile.bytes.data()+size_t(row)*128*4,size_t(count_x)*4,false);
                tile.x=tx;tile.y=ty;tile.valid=true;
            }
            hot_=at;
        }
        if(age_==UINT64_MAX){for(auto& tile:tiles_)tile.age=0;age_=0;}
        tiles_[at].age=++age_;std::memcpy(output,tiles_[at].bytes.data()+(size_t(y%128)*128+x%128)*4,4);
    }
};
}
#endif
