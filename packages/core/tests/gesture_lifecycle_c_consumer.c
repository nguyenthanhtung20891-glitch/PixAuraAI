#include "gesture_lifecycle_boundary.h"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <errno.h>
#ifdef _WIN32
#include <direct.h>
#define MAKE_DIRECTORY(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MAKE_DIRECTORY(p) mkdir(p,0700)
#endif
int main(int argc,char** argv){
    const uint8_t digest[]="d7c6cc1e89b21542bd9d00e376c89135d5f5e313975c4a27363abe9cbc39945a";
    const uint8_t identity[]="00000000000000000000000000006010";
    char root[1024],path[1200];FILE* file=0;int n,result;
    if(argc!=2)return 1;
    n=snprintf(root,sizeof(root),"%s/gesture-c-fixture",argv[1]);if(n<=0||(size_t)n>=sizeof(root))return 1;
    if(MAKE_DIRECTORY(root)!=0&&errno!=EEXIST)return 1;
    n=snprintf(path,sizeof(path),"%s/assets",root);if(n<=0||(size_t)n>=sizeof(path))return 1;
    if(MAKE_DIRECTORY(path)!=0&&errno!=EEXIST)return 1;
    n=snprintf(path,sizeof(path),"%s/assets/sha256",root);if(n<=0||(size_t)n>=sizeof(path))return 1;
    if(MAKE_DIRECTORY(path)!=0&&errno!=EEXIST)return 1;
    n=snprintf(path,sizeof(path),"%s/assets/sha256/%s",root,(const char*)digest);if(n<=0||(size_t)n>=sizeof(path))return 1;
#ifdef _MSC_VER
    if(fopen_s(&file,path,"wb")!=0)return 1;
#else
    file=fopen(path,"wb");if(!file)return 1;
#endif
    if(fwrite(decode_png,1,sizeof(decode_png),file)!=sizeof(decode_png)){fclose(file);return 1;}
    if(fclose(file)!=0)return 1;
    result=gesture_lifecycle_boundary_check((const uint8_t*)root,strlen(root),digest,sizeof(decode_png),identity);
    if(remove(path)!=0)return 1;
    return result;
}
