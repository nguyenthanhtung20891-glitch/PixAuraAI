#include "model_package_internal.hpp"
#include <algorithm>
#include <cstring>
#include <cerrno>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif
namespace pixaura::model {
bool leaf(const std::string& s) {
    if(s.empty()||s.size()>64||s.front()=='.'||s.back()=='.'||s.back()==' ') return false;
    for(unsigned char c:s) if(!((c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='-'||c=='_'||c=='.')) return false;
    const auto stem=s.substr(0,s.find('.'));
    if(stem=="con"||stem=="prn"||stem=="aux"||stem=="nul") return false;
    if(stem.size()==4&&(stem.substr(0,3)=="com"||stem.substr(0,3)=="lpt")&&stem[3]>='1'&&stem[3]<='9') return false;
    return true;
}
#ifdef _WIN32
struct Handle { HANDLE h=INVALID_HANDLE_VALUE; explicit Handle(HANDLE v):h(v){} ~Handle(){if(h!=INVALID_HANDLE_VALUE)CloseHandle(h);} Handle(const Handle&)=delete; };
std::wstring wide(const std::string& s) {
    const int n=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),static_cast<int>(s.size()),nullptr,0);
    need(n>0,PIXAURA_MODEL_PATH); std::wstring w(static_cast<size_t>(n),L'\0');
    need(MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,s.data(),static_cast<int>(s.size()),w.data(),n)==n,PIXAURA_MODEL_PATH); return w;
}
#else
struct Handle { int h; explicit Handle(int v):h(v){} ~Handle(){if(h>=0)close(h);} Handle(const Handle&)=delete; };
#endif
std::vector<File> read_directory(const char* root,uint64_t root_bytes,uint64_t memory_limit) {
    need(root&&root_bytes&&root_bytes<=4096,PIXAURA_MODEL_ARGUMENT);
    std::string path(root,static_cast<size_t>(root_bytes));
    need(path.find('\0')==std::string::npos,PIXAURA_MODEL_PATH);
    std::vector<File> out; out.reserve(10); uint64_t total=0;
#ifdef _WIN32
    const auto w=wide(path); need(w.size()>=3&&w[1]==L':'&&(w[2]==L'\\'||w[2]==L'/'),PIXAURA_MODEL_PATH);
    std::vector<HANDLE> parents;parents.reserve(129);size_t component_start=3;
    struct Parents { std::vector<HANDLE>& hs; ~Parents(){for(auto h:hs)CloseHandle(h);} } cleanup{parents};
    for(size_t end=3;end<=w.size();++end) if(end==w.size()||w[end]==L'\\'||w[end]==L'/') {
        auto part=w.substr(0,end); if(part.back()==L'\\'||part.back()==L'/') {if(end!=3)continue;}
        if(end>3){const auto component=w.substr(component_start,end-component_start);need(!component.empty()&&component!=L"."&&component!=L".."&&component.find(L':')==std::wstring::npos,PIXAURA_MODEL_PATH);component_start=end+1;}
        need(parents.size()<129,PIXAURA_MODEL_PATH);
        const auto h=CreateFileW(part.c_str(),FILE_READ_ATTRIBUTES,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OPEN_REPARSE_POINT,nullptr);
        need(h!=INVALID_HANDLE_VALUE,PIXAURA_MODEL_UNSAFE_FILE); parents.push_back(h);
        BY_HANDLE_FILE_INFORMATION info{}; need(GetFileInformationByHandle(h,&info)&&!(info.dwFileAttributes&FILE_ATTRIBUTE_REPARSE_POINT)&&(info.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY),PIXAURA_MODEL_UNSAFE_FILE);
    }
    WIN32_FIND_DATAW data{}; const auto finder=FindFirstFileW((w+L"\\*").c_str(),&data);
    need(finder!=INVALID_HANDLE_VALUE,PIXAURA_MODEL_IO);
    struct Find { HANDLE h; ~Find(){FindClose(h);} } find{finder};
    do {
        if(!wcscmp(data.cFileName,L".")||!wcscmp(data.cFileName,L".."))continue;
        std::string name; for(const auto* p=data.cFileName;*p;++p){need(*p<128,PIXAURA_MODEL_PATH);name.push_back(static_cast<char>(*p));}
        need(leaf(name),PIXAURA_MODEL_PATH); need(out.size()<10,PIXAURA_MODEL_COUNT);
        Handle file(CreateFileW((w+L"\\"+data.cFileName).c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT,nullptr));
        need(file.h!=INVALID_HANDLE_VALUE,PIXAURA_MODEL_UNSAFE_FILE); BY_HANDLE_FILE_INFORMATION info{};
        need(GetFileInformationByHandle(file.h,&info)&&!(info.dwFileAttributes&(FILE_ATTRIBUTE_REPARSE_POINT|FILE_ATTRIBUTE_DIRECTORY))&&info.nNumberOfLinks==1,PIXAURA_MODEL_UNSAFE_FILE);
        const uint64_t size=(uint64_t(info.nFileSizeHigh)<<32)|info.nFileSizeLow;
#else
    need(path.front()=='/',PIXAURA_MODEL_PATH); Handle current(open("/",O_RDONLY|O_DIRECTORY|O_CLOEXEC));
    size_t start=1;
    while(start<path.size()) {
        const auto end=path.find('/',start); const auto part=path.substr(start,end==std::string::npos?end:end-start);
        need(!part.empty()&&part!="."&&part!="..",PIXAURA_MODEL_PATH);
        const int next=openat(current.h,part.c_str(),O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC);
        need(next>=0,PIXAURA_MODEL_UNSAFE_FILE); close(current.h);current.h=next;
        if(end==std::string::npos)break;
        start=end+1;
    }
    DIR* dir=fdopendir(dup(current.h));need(dir!=nullptr,PIXAURA_MODEL_IO);
    struct Dir {DIR* d;~Dir(){closedir(d);}} cleanup{dir};
    for(;;) {
        errno=0;const auto* entry=readdir(dir);if(!entry){need(errno==0,PIXAURA_MODEL_IO);break;}
        const std::string name=entry->d_name;if(name=="."||name=="..")continue;
        need(leaf(name),PIXAURA_MODEL_PATH);need(out.size()<10,PIXAURA_MODEL_COUNT);
        Handle file(openat(current.h,name.c_str(),O_RDONLY|O_NOFOLLOW|O_NONBLOCK|O_CLOEXEC));need(file.h>=0,PIXAURA_MODEL_UNSAFE_FILE);
        struct stat info{}; need(fstat(file.h,&info)==0&&S_ISREG(info.st_mode)&&info.st_nlink==1&&info.st_size>=0,PIXAURA_MODEL_UNSAFE_FILE);
        const uint64_t size=static_cast<uint64_t>(info.st_size);
#endif
        const uint64_t limit=name=="manifest.pxm"?PIXAURA_MODEL_MANIFEST_LIMIT:(name=="signature.pxs"?80:PIXAURA_MODEL_FILE_LIMIT);
        need(size<=limit,name=="manifest.pxm"?PIXAURA_MODEL_MANIFEST_LIMIT_REASON:(name=="signature.pxs"?PIXAURA_MODEL_SIGNATURE_SIZE:PIXAURA_MODEL_SIZE));
        need(size<=PIXAURA_MODEL_TOTAL_LIMIT+65616-total&&size<=memory_limit-total,PIXAURA_MODEL_RESOURCE);
        total+=size; File bytes{name,std::vector<uint8_t>(static_cast<size_t>(size))}; size_t done=0;
        while(done<bytes.bytes.size()) {
#ifdef _WIN32
            DWORD got=0;need(ReadFile(file.h,bytes.bytes.data()+done,static_cast<DWORD>(std::min<size_t>(65536,bytes.bytes.size()-done)),&got,nullptr)&&got,PIXAURA_MODEL_IO);
#else
            const auto got=read(file.h,bytes.bytes.data()+done,std::min<size_t>(65536,bytes.bytes.size()-done));need(got>0,PIXAURA_MODEL_IO);
#endif
            done+=static_cast<size_t>(got);
        }
        uint8_t extra=0;
#ifdef _WIN32
        DWORD got=0;need(ReadFile(file.h,&extra,1,&got,nullptr)&&!got,PIXAURA_MODEL_SIZE);
#else
        need(read(file.h,&extra,1)==0,PIXAURA_MODEL_SIZE);
#endif
        out.push_back(std::move(bytes));
#ifdef _WIN32
    } while(FindNextFileW(finder,&data));need(GetLastError()==ERROR_NO_MORE_FILES,PIXAURA_MODEL_IO);
#else
    }
#endif
    return out;
}
}
