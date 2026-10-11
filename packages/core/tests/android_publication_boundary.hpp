#ifndef PIXAURA_ANDROID_PUBLICATION_BOUNDARY_HPP
#define PIXAURA_ANDROID_PUBLICATION_BOUNDARY_HPP
#include "../src/android_asset_publication.hpp"
#include "../src/storage.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <future>
#include <atomic>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <sys/stat.h>
#include <cstdio>
namespace pixaura::testing {
inline unsigned android_publication_boundary(const char* root) {
    namespace fs=std::filesystem;using namespace storage;unsigned checks=0;
    auto check=[&](bool ok){if(!ok)throw document::Failure{14};++checks;};
    auto rejects=[&](int code,auto work){bool failed=false;try{work();}catch(const document::Failure& e){check(e.code==code);failed=true;}check(failed);};
    struct Input:Reader {unsigned left=1024;uint8_t value;bool fail=false;explicit Input(uint8_t v):value(v){}
        std::size_t read(uint8_t* p,std::size_t n)override{if(fail)throw document::Failure{12};n=std::min<std::size_t>(n,left);std::fill_n(p,n,value);left-=static_cast<unsigned>(n);return n;}};
    const fs::path base(root);AssetStore store(base.generic_string());Input input(31);const auto asset=store.ingest(input,1024);store.verify(asset);check(asset.bytes==1024);
    const auto object=base/"assets"/"sha256"/asset.digest.c_str();struct stat before{},after{};
    check(stat(object.c_str(),&before)==0&&(before.st_mode&0222)==0&&before.st_nlink==1);
    Input duplicate(31);check(store.ingest(duplicate,1024,asset.digest).digest==asset.digest);
    check(stat(object.c_str(),&after)==0&&before.st_ino==after.st_ino);store.verify(asset);
    // Only the owned synthetic fixture is made corrupt, never user content.
    check(chmod(object.c_str(),0600)==0);{std::ofstream f(object,std::ios::binary|std::ios::trunc);f<<"corrupt";}
    Input corrupt(31);rejects(6,[&]{store.ingest(corrupt,1024,asset.digest);});check(fs::file_size(object)==7);check(chmod(object.c_str(),0444)==0);
    Input failed(99);failed.fail=true;rejects(12,[&]{store.ingest(failed,1024);});
    check(std::distance(fs::directory_iterator(base/"assets"/"sha256"),fs::directory_iterator())==1);
    std::atomic<unsigned> ready{0};
    auto publisher=[&](uint8_t value){AssetStore other(base.generic_string());Input bytes(value);++ready;
        const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(5);
        while(ready.load()<2){if(std::chrono::steady_clock::now()>deadline){throw document::Failure{14};}std::this_thread::yield();}
        return other.ingest(bytes,1024);};
    auto a=std::async(std::launch::async,publisher,41),b=std::async(std::launch::async,publisher,41);
    const auto same=a.get(),same2=b.get();check(same.digest==same2.digest);store.verify(same);
    ready=0;
    auto c=std::async(std::launch::async,publisher,42),d=std::async(std::launch::async,publisher,43);
    const auto different=c.get(),different2=d.get();check(different.digest!=different2.digest);store.verify(different);store.verify(different2);
    struct FD {int value;explicit FD(int fd):value(fd){if(fd<0)throw document::Failure{12};}~FD(){close(value);}};
    FD source(open((base/"staging").c_str(),O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC));
    FD destination(open((base/"assets"/"sha256").c_str(),O_RDONLY|O_DIRECTORY|O_NOFOLLOW|O_CLOEXEC));
    struct Observed:AndroidPublicationSyscalls {unsigned calls=0,syncs=0;int failRename=0,failSync=0;
        int rename_no_replace(int s,const char* n,int t,const char* digest){++calls;if(failRename){errno=failRename;return -1;}return AndroidPublicationSyscalls::rename_no_replace(s,n,t,digest);}
        int sync_directory(int fd){++syncs;if(failSync&&syncs==static_cast<unsigned>(failSync)){errno=EIO;return -1;}return fsync(fd);}};
    unsigned actualRenames=0;
    for(int scenario=0;scenario<7;++scenario){
        const auto name="boundary-"+std::to_string(scenario),digest="owned-final-"+std::to_string(scenario);
        FD staged(openat(source.value,name.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600));
        const uint8_t bytes[]={1,2,3,4};check(write(staged.value,bytes,4)==4);check(fchmod(staged.value,0444)==0);check(fsync(staged.value)==0);
        Observed os;if(scenario==0){os.failRename=ENOSYS;}if(scenario==1){os.failRename=EOPNOTSUPP;}if(scenario==2){os.failRename=EIO;}
        if(scenario==3){os.failSync=1;}if(scenario==4){os.failSync=2;}
        if(scenario<5)rejects(12,[&]{android_asset_publish(source.value,name.c_str(),destination.value,digest.c_str(),os);});
        else{check(android_asset_publish(source.value,name.c_str(),destination.value,digest.c_str(),os));
            FD extra(openat(source.value,name.c_str(),O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600));check(write(extra.value,"bad",3)==3);
            check(!android_asset_publish(source.value,name.c_str(),destination.value,digest.c_str(),os));}
        if(scenario<3){check(fs::exists(base/"staging"/name));check(!fs::exists(base/"assets"/"sha256"/digest));check(os.syncs==0);}
        else{++actualRenames;check(fs::file_size(base/"assets"/"sha256"/digest)==4);}
        check(os.calls==static_cast<unsigned>(scenario<5?1:2));
    }
    std::printf("ADR0021 Android RENAME_NOREPLACE actual publications=%u; immutable/dedup/corrupt/concurrent/staging/unsupported/sync checks=%u PASS\n",actualRenames,checks);
    return checks;
}
}
#endif
