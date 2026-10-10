#include "preset_boundary.h"
#include <stdio.h>
int main(int argc,char** argv) {
    uint8_t bytes[8192]; FILE* file=NULL; size_t count; int status;
    if(argc!=2)return 1;
#ifdef _MSC_VER
    if(fopen_s(&file,argv[1],"rb")!=0)return 1;
#else
    file=fopen(argv[1],"rb");
#endif
    if(!file)return 1;
    count=fread(bytes,1,sizeof(bytes),file);
    if(ferror(file)||!feof(file)){fclose(file);return 1;}
    fclose(file);
    status=preset_boundary_check(bytes,count,(const uint8_t*)"00000000000000000000000000000900");
    if(!status)puts("C preset canonical/proposal/stale boundary PASS");
    return status;
}
