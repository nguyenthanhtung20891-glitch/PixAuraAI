#include "pixaura/storage.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
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
        db=fopen(path,"rb");assert(db!=NULL);
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
    puts("C storage schema/reopen boundary PASS");return 0;
}
