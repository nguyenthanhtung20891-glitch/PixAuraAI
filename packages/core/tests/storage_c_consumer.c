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
    puts("C storage schema/reopen boundary PASS");return 0;
}
