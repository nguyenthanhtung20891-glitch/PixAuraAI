#include "preset_boundary.h"
#include <stdio.h>
int main(int argc,char** argv){uint8_t bytes[8192];FILE* file;size_t count;int status;if(argc!=2)return 1;file=fopen(argv[1],"rb");if(!file)return 1;count=fread(bytes,1,sizeof(bytes),file);fclose(file);status=preset_boundary_check(bytes,count,(const uint8_t*)"00000000000000000000000000000900");if(!status)puts("C preset canonical/proposal/stale boundary PASS");return status;}
