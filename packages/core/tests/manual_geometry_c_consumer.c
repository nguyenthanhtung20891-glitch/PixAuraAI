#include "manual_geometry_boundary.h"
#include "manual_tone_boundary.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char** argv) {
    uint8_t bytes[8192]; size_t count; FILE* file; int status;
    if (argc != 2) return 1;
#ifdef _MSC_VER
    if (fopen_s(&file, argv[1], "rb") != 0) return 2;
#else
    file = fopen(argv[1], "rb"); if (!file) return 2;
#endif
    count = fread(bytes, 1, sizeof(bytes), file); fclose(file);
    status = manual_geometry_boundary_check(bytes, count, (const uint8_t*)"00000000000000000000000000000701");
    if (status != 0) fprintf(stderr, "geometry C boundary failure %d\n", status);
    if(status==0)status=manual_tone_boundary_check(bytes,count,(const uint8_t*)"00000000000000000000000000000801");
    if(status!=0)fprintf(stderr,"tone C boundary failure %d\n",status);
    return status;
}
