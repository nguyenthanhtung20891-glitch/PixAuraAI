#include "pixaura/decode.h"
#include "pixaura/working.h"
#include "../../../tests/fixtures/decode/fixtures.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <direct.h>
#define MKDIR(p) _mkdir(p)
#else
#include <sys/stat.h>
#define MKDIR(p) mkdir(p,0700)
#endif
#define CHECK(x) do{if(!(x)){fprintf(stderr,"C decode check %d\n",__LINE__);return 1;}}while(0)
int main(int argc,char** argv){
    char root[2048],assets[2048],directory[2048],file[2048];FILE* f;
    const char hash[]="d7c6cc1e89b21542bd9d00e376c89135d5f5e313975c4a27363abe9cbc39945a";
    const uint8_t identity[]="cccccccccccccccccccccccccccccccc";
    pixaura_decode_context c={0};pixaura_decode_handle source={0},image={0},sentinel;
    pixaura_decode_metadata m={0};pixaura_decode_limits l;uint8_t pixels[24]={0};
    CHECK(argc==2);CHECK(strlen(argv[1])<1000);
    CHECK(snprintf(root,sizeof(root),"%s/decode-c-root",argv[1])>0);(void)MKDIR(root);
    CHECK(snprintf(assets,sizeof(assets),"%s/assets",root)>0);(void)MKDIR(assets);
    CHECK(snprintf(directory,sizeof(directory),"%s/sha256",assets)>0);(void)MKDIR(directory);
    CHECK(snprintf(file,sizeof(file),"%s/%s",directory,hash)>0);
#ifdef _MSC_VER
    CHECK(fopen_s(&f,file,"wb")==0);
#else
    f=fopen(file,"wb");
#endif
    CHECK(f!=NULL);CHECK(fwrite(decode_png,1,sizeof(decode_png),f)==sizeof(decode_png));CHECK(fclose(f)==0);
    CHECK(pixaura_decode_default_limits(1,&l)==0);CHECK(pixaura_decode_context_init(1,&c,sizeof(c),identity,32,&l)==0);
    CHECK(pixaura_decode_open(&c,(const uint8_t*)root,strlen(root),(const uint8_t*)hash,64,sizeof(decode_png),&source)==0);
    CHECK(pixaura_decode_query(&c,&source,&m)==0&&m.width==2&&m.height==3&&m.format==2&&m.row_stride==8);
    CHECK(pixaura_decode_image(&c,&source,&image)==0);CHECK(pixaura_decode_copy_pixels(&c,&image,0,pixels,24)==0&&pixels[0]==255&&pixels[23]==128);
    CHECK(pixaura_decode_copy_pixels(&c,&image,23,pixels,2)==1&&pixels[0]==255);
    sentinel=image;CHECK(pixaura_decode_image(&c,&image,&sentinel)==3&&memcmp(&sentinel,&image,sizeof(image))==0);
    {
        pixaura_working_limits wl;pixaura_working_metadata wm;pixaura_decode_handle working={0},identity_image={0};float values[24]={0};
        CHECK(pixaura_working_default_limits(1,&wl)==0);
        CHECK(pixaura_working_normalize(&c,&image,&wl,&working)==0);
        CHECK(pixaura_working_query(&c,&working,&wm)==0&&wm.width==2&&wm.height==3&&wm.orientation==1&&wm.row_stride==32);
        CHECK(pixaura_working_copy(&c,&working,0,values,24)==0&&values[0]==1.0f);
        CHECK(pixaura_working_identity(&c,&working,&wl,&identity_image)==0);
        CHECK(pixaura_decode_release(&c,&working)==0);
        CHECK(pixaura_working_query(&c,&working,&wm)==3);
        CHECK(pixaura_working_copy(&c,&identity_image,0,values,24)==0&&values[0]==1.0f);
        CHECK(pixaura_decode_release(&c,&identity_image)==0);
    }
    CHECK(pixaura_decode_release(&c,&source)==0);CHECK(pixaura_decode_query(&c,&image,&m)==0);
    CHECK(pixaura_decode_release(&c,&image)==0);CHECK(pixaura_decode_release(&c,&image)==3);
    CHECK(pixaura_decode_context_destroy(&c)==0);CHECK(pixaura_decode_context_destroy(&c)==3);
    CHECK(pixaura_decode_query(&c,&image,&m)==3);
    puts("independent C decode boundary passed");return 0;
}
