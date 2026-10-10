#include "pixaura/storage.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#ifdef _WIN32
#include <direct.h>
#include <process.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif
int main(int argc,char** argv) {
    assert(argc==2);
    pixaura_storage_info info={87,99,55,44}, before=info;
    assert(sizeof(info)==16);
    assert(pixaura_storage_check(99,NULL,0,&info)==2);
    assert(memcmp(&info,&before,sizeof(info))==0);
    assert(pixaura_storage_check(1,NULL,0,&info)==1);
    assert(pixaura_storage_check(1,(const uint8_t*)"../escape",9,&info)==1);
    assert(pixaura_storage_check(1,(const uint8_t*)"/",1,&info)==1);
    assert(memcmp(&info,&before,sizeof(info))==0);
    assert(pixaura_storage_check(1,(const uint8_t*)argv[1],strlen(argv[1]),&info)==0);
    assert(info.api_version==1 && info.struct_size==16 && info.storage_version==1 && info.sqlite_version==3053004);
    assert(pixaura_storage_check(1,(const uint8_t*)argv[1],strlen(argv[1]),&info)==0);
    {
        char path[1100];
        unsigned char header[16];
        FILE* db;
        int n=snprintf(path,sizeof(path),"%s/catalog.sqlite",argv[1]);
        assert(n>0 && (size_t)n<sizeof(path));
#ifdef _MSC_VER
        assert(fopen_s(&db,path,"rb")==0);
#else
        db=fopen(path,"rb");
#endif
        assert(db!=NULL);
        assert(fread(header,1,sizeof(header),db)==sizeof(header));assert(fclose(db)==0);
        assert(memcmp(header,"SQLite format 3",16)==0);
        info=before;
        /* A file is not the directory required by this API. */
        assert(pixaura_storage_check(1,(const uint8_t*)path,strlen(path),&info)==6);
        assert(memcmp(&info,&before,sizeof(info))==0);
        n=snprintf(path,sizeof(path),"%s/storage-c-nonexistent",argv[1]);
        assert(n>0 && (size_t)n<sizeof(path));
#ifdef _WIN32
        assert(pixaura_storage_check(1,(const uint8_t*)path,strlen(path),&info)==12);
#else
        assert(pixaura_storage_check(1,(const uint8_t*)path,strlen(path),&info)==6);
#endif
        assert(memcmp(&info,&before,sizeof(info))==0);
        n=snprintf(path,sizeof(path),"%s/../escape",argv[1]);
        assert(n>0 && (size_t)n<sizeof(path));
        assert(pixaura_storage_check(1,(const uint8_t*)path,strlen(path),&info)==1);
        assert(memcmp(&info,&before,sizeof(info))==0);
    }
    {
        char path[1100];int n;
#ifdef _WIN32
        n=snprintf(path,sizeof(path),"%s/storage-migration-%ld-%d",argv[1],(long)time(NULL),_getpid());
        assert(n>0&&(size_t)n<sizeof(path));assert(_mkdir(path)==0);
#else
        n=snprintf(path,sizeof(path),"%s/storage-migration-%ld-%ld",argv[1],(long)time(NULL),(long)getpid());
        assert(n>0&&(size_t)n<sizeof(path));assert(mkdir(path,0700)==0);
#endif
        assert(pixaura_storage_check(1,(const uint8_t*)path,strlen(path),&info)==0&&info.storage_version==1);
        assert(pixaura_storage_migrate(1,(const uint8_t*)path,strlen(path),1,2,&info)==0&&info.storage_version==2);
        assert(pixaura_storage_check(1,(const uint8_t*)path,strlen(path),&info)==0&&info.storage_version==2);
        info=before;assert(pixaura_storage_migrate(1,(const uint8_t*)path,strlen(path),1,2,&info)==4&&memcmp(&info,&before,sizeof(info))==0);
        assert(pixaura_storage_migrate(1,(const uint8_t*)path,strlen(path),2,2,&info)==0&&info.storage_version==2);
    }
    puts("C storage schema/reopen/explicit migration boundary PASS");return 0;
}
