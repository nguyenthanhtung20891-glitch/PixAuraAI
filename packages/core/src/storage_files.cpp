#include "storage_files.hpp"
#include "sha256.hpp"
#include <array>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#else
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef __APPLE__
#include <sys/types.h>
#endif
#endif
namespace pixaura::storage {
namespace {
void require_at(int line, bool ok, int32_t code=12) {
    (void)line;
    if (!ok) {
#ifdef PIXAURA_STORAGE_TESTING
#ifdef _WIN32
        std::fprintf(stderr,"storage filesystem failure line=%d status=%d os=%lu\n",line,code,GetLastError());
#else
        std::fprintf(stderr,"storage filesystem failure line=%d status=%d os=%d\n",line,code,errno);
#endif
#endif
        throw document::Failure{code};
    }
}
#define require(...) require_at(__LINE__, __VA_ARGS__)
#ifdef _WIN32
using Native = HANDLE;
const Native invalid = INVALID_HANDLE_VALUE;
document::Vector<wchar_t> wide(std::string_view text) {
    require(text.size()<=4096,8);
    const auto n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text.data(),static_cast<int>(text.size()),nullptr,0);
    require(n>0,1);document::Vector<wchar_t> result(static_cast<std::size_t>(n)+1);
    require(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,text.data(),static_cast<int>(text.size()),result.data(),n)==n,1);
    return result;
}
void close_native(Native h) noexcept {if(h!=invalid) CloseHandle(h);}
#else
using Native = int;
constexpr Native invalid = -1;
void close_native(Native h) noexcept {if(h!=invalid) close(h);}
#endif
struct File {
    Native handle=invalid;
    explicit File(Native h=invalid):handle(h) {}
    ~File(){close_native(handle);}
    File(const File&)=delete;File& operator=(const File&)=delete;
    File(File&& other) noexcept:handle(other.handle){other.handle=invalid;}
    File& operator=(File&& other) noexcept {if(this!=&other){close_native(handle);handle=other.handle;other.handle=invalid;}return *this;}
};
uint64_t length(const File& f) {
#ifdef _WIN32
    BY_HANDLE_FILE_INFORMATION info{};require(GetFileInformationByHandle(f.handle,&info)!=0);
    require((info.dwFileAttributes&(FILE_ATTRIBUTE_REPARSE_POINT|FILE_ATTRIBUTE_DIRECTORY))==0 && info.nNumberOfLinks==1,6);
    return (uint64_t(info.nFileSizeHigh)<<32)|info.nFileSizeLow;
#else
    struct stat info{};require(fstat(f.handle,&info)==0);
    require(S_ISREG(info.st_mode)&&info.st_nlink==1&&info.st_size>=0,6);return static_cast<uint64_t>(info.st_size);
#endif
}
void sync(const File& f) {
#ifdef _WIN32
    require(FlushFileBuffers(f.handle)!=0);
#else
#ifdef __APPLE__
    require(fcntl(f.handle,F_FULLFSYNC)==0);
#else
    require(fsync(f.handle)==0);
#endif
#endif
}
std::size_t read(const File& f,uint8_t* p,std::size_t n) {
#ifdef _WIN32
    DWORD got=0;require(ReadFile(f.handle,p,static_cast<DWORD>(n),&got,nullptr)!=0);return got;
#else
    ssize_t got;do{got=::read(f.handle,p,n);}while(got<0&&errno==EINTR);require(got>=0);return static_cast<std::size_t>(got);
#endif
}
void write(const File& f,const uint8_t* p,std::size_t n) {
    while(n) {
#ifdef _WIN32
        DWORD got=0;require(WriteFile(f.handle,p,static_cast<DWORD>(n),&got,nullptr)!=0&&got>0);
#else
        ssize_t got;do{got=::write(f.handle,p,n);}while(got<0&&errno==EINTR);require(got>0);
#endif
        const auto bytes=static_cast<std::size_t>(got);p+=bytes;n-=bytes;
    }
}
bool digest_valid(std::string_view value){return value.size()==64&&value.find_first_not_of("0123456789abcdef")==std::string_view::npos;}
}
#ifdef PIXAURA_STORAGE_TESTING
thread_local int fault_point=-1;
thread_local bool crash_fault=false;
thread_local void (*point_hook)(Point)=nullptr;
thread_local const char* temp_name_override=nullptr;
#endif
void fault(Point point) {
#ifdef PIXAURA_STORAGE_TESTING
    if(point_hook)point_hook(point);
    if(fault_point==static_cast<int>(point)) {if(crash_fault) std::_Exit(73);throw document::Failure{12};}
#else
    (void)point;
#endif
}
struct Files::Impl {
    String root;
    File root_dir,assets_parent,assets_dir,stage_dir;
#ifdef _WIN32
    std::array<File,512> parents;
    std::size_t parent_count=0;
#endif
    explicit Impl(std::string_view value):root(value) {
        require(value.size()>1&&value.size()<=1024&&value.find('\0')==std::string_view::npos&&value.back()!='/'&&value.back()!='\\',1);
#ifdef _WIN32
        require(value.size()>3&&value[1]==':'&&(value[2]=='/'||value[2]=='\\'),1);
        require(value.substr(2).find(':')==value.npos,1);
        for(std::size_t i=3;i<=value.size();++i) if(i==value.size()||value[i]=='/'||value[i]=='\\') {
            const auto prefix=value.substr(0,i);const auto end=prefix.find_last_of("/\\");const auto name=prefix.substr(end+1);
            require(!name.empty()&&name!="."&&name!="..",1);
            auto path=wide(prefix);File f(CreateFileW(path.data(),FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT,nullptr));
            require(f.handle!=invalid);BY_HANDLE_FILE_INFORMATION info{};require(GetFileInformationByHandle(f.handle,&info)!=0);
            require((info.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0&&(info.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)==0,6);
            require(parent_count<parents.size(),8);parents[parent_count++]=std::move(f);
        }
        root_dir=directory("",false);assets_parent=directory("assets",true);
        auto sha=wide(path("assets/sha256"));require(CreateDirectoryW(sha.data(),nullptr)!=0||GetLastError()==ERROR_ALREADY_EXISTS);
        assets_dir=directory("assets/sha256",false);stage_dir=directory("staging",true);
#else
        require(value.front()=='/',1);
#ifdef __APPLE__
        // Atomic all-component no-follow lookup avoids requesting read access to
        // sandbox-external ancestor directories. Caller supplies canonical root.
        std::size_t start=1;
        while(start<value.size()) {
            const auto end=value.find('/',start);const auto part=value.substr(start,end==value.npos?value.size()-start:end-start);
            require(!part.empty()&&part!="."&&part!="..",1);if(end==value.npos)break;start=end+1;
        }
        File current(open(root.c_str(),O_RDONLY|O_DIRECTORY|O_NOFOLLOW_ANY|O_CLOEXEC));require(current.handle!=invalid,6);
#else
        // O_PATH requires traversal, not permission to enumerate /data or other
        // protected OS ancestors. The owned leaf is opened readably for fsync.
        File current(open("/",O_PATH|O_DIRECTORY|O_CLOEXEC));require(current.handle!=invalid);
        std::size_t start=1;
        while(start<value.size()) {
            const auto end=value.find('/',start);const auto part=value.substr(start,end==value.npos?value.size()-start:end-start);
            require(!part.empty()&&part!="."&&part!="..",1);String name(part);
            const int access=end==value.npos?O_RDONLY:O_PATH;
            File next(openat(current.handle,name.c_str(),access|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC));require(next.handle!=invalid,6);
            current=std::move(next);if(end==value.npos) break;start=end+1;
        }
#endif
        root_dir=std::move(current);File assets=directory_at(root_dir,"assets");assets_dir=directory_at(assets,"sha256");stage_dir=directory_at(root_dir,"staging");
        require(fsync(root_dir.handle)==0);
#endif
    }
    String path(std::string_view name) const {String result(root);if(!name.empty()){result+='/';result.append(name);}return result;}
#ifdef _WIN32
    File directory(std::string_view name,bool create) {
        auto p=wide(path(name));if(create) require(CreateDirectoryW(p.data(),nullptr)!=0||GetLastError()==ERROR_ALREADY_EXISTS);
        File f(CreateFileW(p.data(),FILE_READ_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT,nullptr));require(f.handle!=invalid);
        BY_HANDLE_FILE_INFORMATION info{};require(GetFileInformationByHandle(f.handle,&info)!=0);
        require((info.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0&&(info.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)==0,6);return f;
    }
#else
    static File directory_at(const File& parent,const char* name) {
        require(mkdirat(parent.handle,name,0700)==0||errno==EEXIST);
        File f(openat(parent.handle,name,O_RDONLY|O_DIRECTORY|O_CLOEXEC|O_NOFOLLOW));require(f.handle!=invalid,6);require(fsync(parent.handle)==0);return f;
    }
#endif
    File open_file(std::string_view dir,std::string_view name,bool create) const {
        require(name.find_first_of("/\\") == name.npos && name!="."&&name!=".."&&!name.empty(),1);
#ifdef _WIN32
        String relative(dir);if(!dir.empty()) relative+='/';relative.append(name);auto p=wide(path(relative));
        const DWORD sharing=dir.empty()?FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE:FILE_SHARE_READ;
        return File(CreateFileW(p.data(),create?GENERIC_WRITE:(GENERIC_READ|FILE_READ_ATTRIBUTES),sharing,nullptr,create?CREATE_NEW:OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT,nullptr));
#else
        const auto fd=dir=="staging"?stage_dir.handle:dir=="assets/sha256"?assets_dir.handle:root_dir.handle;String text(name);
        return File(openat(fd,text.c_str(),(create?(O_WRONLY|O_CREAT|O_EXCL):O_RDONLY)|O_NOFOLLOW|O_CLOEXEC,0600));
#endif
    }
    String temporary() const {
#ifdef PIXAURA_STORAGE_TESTING
        if(temp_name_override)return String(temp_name_override);
#endif
        // Exclusive creation decides uniqueness; random spelling is not an identity.
        std::array<uint8_t,16> random{};
#ifdef _WIN32
        require(BCryptGenRandom(nullptr,random.data(),static_cast<ULONG>(random.size()),BCRYPT_USE_SYSTEM_PREFERRED_RNG)==0);
#else
        File f(open("/dev/urandom",O_RDONLY|O_CLOEXEC));require(f.handle!=invalid);
        std::size_t got=0;while(got<random.size()){const auto n=read(f,random.data()+got,random.size()-got);require(n>0);got+=n;}
#endif
        String name("temp-");constexpr char hex[]="0123456789abcdef";for(auto b:random){name+=hex[b>>4];name+=hex[b&15];}return name;
    }
    void remove_temp(std::string_view name) const noexcept {
#ifdef _WIN32
        try{String relative("staging/");relative.append(name);auto p=wide(path(relative));DeleteFileW(p.data());}catch(...){}
#else
        // Temp names are already owned, null-terminated String values.
        (void)unlinkat(stage_dir.handle,name.data(),0);
#endif
    }
    bool publish(std::string_view name,std::string_view digest) const {
#ifdef _WIN32
        String a("staging/");a.append(name);String b("assets/sha256/");b.append(digest);auto from=wide(path(a)),to=wide(path(b));
        if(MoveFileExW(from.data(),to.data(),MOVEFILE_WRITE_THROUGH)!=0) return true;
        require(GetLastError()==ERROR_ALREADY_EXISTS||GetLastError()==ERROR_FILE_EXISTS);return false;
#else
        String a(name),b(digest);if(linkat(stage_dir.handle,a.c_str(),assets_dir.handle,b.c_str(),0)==0) {
            require(unlinkat(stage_dir.handle,a.c_str(),0)==0);require(fsync(assets_dir.handle)==0&&fsync(stage_dir.handle)==0);return true;
        }
        require(errno==EEXIST);return false;
#endif
    }
};
// Move assignment closes the previous OS handle without allocation.
// Declared separately above constructors to keep every failure cleanup scoped.
Files::Files(std::string_view root):impl_(std::make_unique<Impl>(root)){}
Files::~Files()=default;
Asset Files::inspect(std::string_view digest) const {
    require(digest_valid(digest),1);auto f=impl_->open_file("assets/sha256",digest,false);require(f.handle!=invalid,6);
    Asset result{String(digest),length(f)};verify(result);return result;
}
void Files::verify(const Asset& asset) const {
    require(digest_valid(asset.digest)&&asset.bytes>0&&asset.bytes<=asset_limit,1);
    auto file=impl_->open_file("assets/sha256",asset.digest,false);require(file.handle!=invalid,6);require(length(file)==asset.bytes,6);
    Sha256 hash;std::array<uint8_t,stream_buffer> buffer{};uint64_t total=0;
    while(total<asset.bytes) {const auto n=read(file,buffer.data(),static_cast<std::size_t>(std::min<uint64_t>(buffer.size(),asset.bytes-total)));require(n>0,6);hash.update(buffer.data(),n);total+=n;}
    require(read(file,buffer.data(),1)==0&&length(file)==asset.bytes&&hash.finish()==asset.digest,6);
}
Asset Files::ingest(Reader& reader,uint64_t expected,std::string_view digest) {
    require(expected>0&&expected<=asset_limit,8);require(digest.empty()||digest_valid(digest),1);fault(Point::before_temp);
    const auto name=impl_->temporary();
    struct Cleanup{Impl& files;const String& name;bool owned=false;~Cleanup(){if(owned)files.remove_temp(name);}} cleanup{*impl_,name};
    File file=impl_->open_file("staging",name,true);require(file.handle!=invalid);cleanup.owned=true;Sha256 hash;std::array<uint8_t,stream_buffer> buffer{};uint64_t total=0;
    for(;;){const auto n=reader.read(buffer.data(),buffer.size());require(n<=buffer.size(),1);if(n==0)break;require(n<=expected-total,6);write(file,buffer.data(),n);hash.update(buffer.data(),n);total+=n;fault(Point::asset_write);}
    require(total==expected,6);Asset result{hash.finish(),total};require(digest.empty()||result.digest==digest,6);
#ifndef _WIN32
    require(fchmod(file.handle,0444)==0);
#endif
    sync(file);file=File();fault(Point::before_publish);const bool published=impl_->publish(name,result.digest);
    // Even deduplication rehashes bytes; a mismatched destination is never replaced.
    verify(result);
#ifdef _WIN32
    if(published){String p("assets/sha256/");p+=result.digest;auto path=wide(impl_->path(p));require(SetFileAttributesW(path.data(),FILE_ATTRIBUTE_READONLY)!=0);}
#else
    (void)published;
#endif
    fault(Point::after_publish);return result;
}
String Files::database_path() {
    // SQLite's fixed sidecars are checked too; never accept a link as a database.
    for(const auto* name:{"catalog.sqlite","catalog.sqlite-wal","catalog.sqlite-shm","catalog.sqlite-journal"}) {
        auto f=impl_->open_file("",name,false);
        if(f.handle!=invalid) require(length(f)<=268435456,8);
        else {
#ifdef _WIN32
            require(GetLastError()==ERROR_FILE_NOT_FOUND,6);
#else
            require(errno==ENOENT,6);
#endif
        }
    }
    return impl_->path("catalog.sqlite");
}
uint64_t Files::wal_bytes() const {
    auto file=impl_->open_file("","catalog.sqlite-wal",false);
    if(file.handle!=invalid)return length(file);
#ifdef _WIN32
    require(GetLastError()==ERROR_FILE_NOT_FOUND,6);
#else
    require(errno==ENOENT,6);
#endif
    return 0;
}
bool Files::checkpoint(std::string_view bytes) {
    require(bytes.size()<=document::manifest_limit,8);const auto name=impl_->temporary();
    struct Cleanup{Impl& files;const String& name;bool owned=false;~Cleanup(){if(owned)files.remove_temp(name);}}cleanup{*impl_,name};
    File file=impl_->open_file("staging",name,true);require(file.handle!=invalid);cleanup.owned=true;
    const auto half=bytes.size()/2;write(file,reinterpret_cast<const uint8_t*>(bytes.data()),half);fault(Point::checkpoint_write);
    write(file,reinterpret_cast<const uint8_t*>(bytes.data()+half),bytes.size()-half);sync(file);file=File();fault(Point::checkpoint_publish);
#ifdef _WIN32
    String relative("staging/");relative+=name;auto from=wide(impl_->path(relative)),to=wide(impl_->path("checkpoint.json"));
    require(MoveFileExW(from.data(),to.data(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0);
    fault(Point::checkpoint_renamed);
#else
    require(renameat(impl_->stage_dir.handle,name.c_str(),impl_->root_dir.handle,"checkpoint.json")==0);
    fault(Point::checkpoint_renamed);
    require(fsync(impl_->root_dir.handle)==0&&fsync(impl_->stage_dir.handle)==0);
#endif
    return true;
}
AssetStore::AssetStore(std::string_view root):files_(std::make_shared<Files>(root)){}
Asset AssetStore::ingest(Reader& r,uint64_t expected,std::string_view digest,std::string_view filename) {
    require(filename.size()<=256,8);return files_->ingest(r,expected,digest);
}
void AssetStore::verify(const Asset& asset) const{files_->verify(asset);}
}
