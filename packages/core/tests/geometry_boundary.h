#ifndef PIXAURA_GEOMETRY_BOUNDARY_TEST_H
#define PIXAURA_GEOMETRY_BOUNDARY_TEST_H
#include "pixaura/geometry.h"
#include <string.h>
/* Shared native consumer scenario, not production semantics or pixel math. */
static int geometry_boundary_check(pixaura_decode_context* c,const pixaura_decode_handle* source,
    const pixaura_working_limits* limits) {
    const uint8_t request[]="{\"operations\":[{\"id\":\"00000000000000000000000000000009\",\"type\":\"pixaura.crop\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"x_ppm\":999999,\"y_ppm\":0,\"width_ppm\":1,\"height_ppm\":1000000}}]}";
    const uint8_t identity[]="{\"operations\":[]}";
    pixaura_geometry_plan plan; pixaura_geometry_tile tile;
    pixaura_decode_handle token,output,unchanged;
    memset(&output,0x5a,sizeof(output));unchanged=output;
    if(pixaura_geometry_preflight(1,2,3,request,sizeof(request)-1,limits,&plan)!=0||plan.width!=1||plan.height!=3||plan.executable!=1)return 14;
    if(pixaura_geometry_tile_at(1,129,129,3,&tile)!=0||tile.x0!=128||tile.y0!=128||tile.x1!=129||tile.y1!=129)return 14;
    if(pixaura_geometry_preflight(1,2,3,(const uint8_t*)"{}",2,limits,&plan)==0)return 14;
    if(pixaura_cancel_create(c,1,&token)!=0)return 14;
    {
        const uint8_t invalid[]="{\"operations\":[{\"id\":\"00000000000000000000000000000001\",\"type\":\"pixaura.rotate\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"quarter_turns\":4}}]}";
        if(pixaura_working_evaluate_cancel(c,source,1,invalid,sizeof(invalid)-1,limits,&token,&output)!=7||memcmp(&output,&unchanged,sizeof(output))!=0)return 14;
    }
    {
        float original[24],cropped[12];pixaura_working_metadata metadata;
        if(pixaura_working_copy(c,source,0,original,24)!=0)return 14;
        if(pixaura_working_evaluate_cancel(c,source,1,request,sizeof(request)-1,limits,&token,&output)!=0)return 14;
        if(pixaura_working_query(c,&output,&metadata)!=0||metadata.width!=1||metadata.height!=3)return 14;
        if(pixaura_working_copy(c,&output,0,cropped,12)!=0)return 14;
        if(memcmp(cropped,original+4,16)||memcmp(cropped+4,original+12,16)||memcmp(cropped+8,original+20,16))return 14;
        if(pixaura_decode_release(c,&output)!=0)return 14;
        output=unchanged;
    }
    {
        const uint8_t mixed[]="{\"operations\":[{\"id\":\"00000000000000000000000000000001\",\"type\":\"pixaura.rotate\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"quarter_turns\":1}},{\"id\":\"00000000000000000000000000000002\",\"type\":\"pixaura.crop\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"x_ppm\":0,\"y_ppm\":0,\"width_ppm\":1,\"height_ppm\":1}},{\"id\":\"00000000000000000000000000000003\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":1000}}]}";
        float original[24],pixel[4];pixaura_working_metadata metadata;
        if(pixaura_working_copy(c,source,0,original,24)!=0)return 14;
        if(pixaura_working_evaluate_cancel(c,source,1,mixed,sizeof(mixed)-1,limits,&token,&output)!=0)return 14;
        if(pixaura_working_query(c,&output,&metadata)!=0||metadata.width!=1||metadata.height!=1)return 14;
        if(pixaura_working_copy(c,&output,0,pixel,4)!=0||pixel[0]!=original[16]*2||pixel[1]!=original[17]*2||pixel[2]!=original[18]*2||memcmp(pixel+3,original+19,4)!=0)return 14;
        if(pixaura_decode_release(c,&output)!=0)return 14;
        output=unchanged;
    }
    if(pixaura_working_evaluate_cancel(c,source,1,identity,sizeof(identity)-1,limits,&token,&output)!=0)return 14;
    if(pixaura_decode_release(c,&output)!=0)return 14;
    output=unchanged;
    if(pixaura_cancel_signal(c,&token)!=0||pixaura_cancel_signal(c,&token)!=0)return 14;
    if(pixaura_working_evaluate_cancel(c,source,1,identity,sizeof(identity)-1,limits,&token,&output)!=13||memcmp(&output,&unchanged,sizeof(output))!=0)return 14;
    if(pixaura_cancel_release(c,&token)!=0||pixaura_cancel_signal(c,&token)!=3||pixaura_cancel_release(c,&token)!=3)return 14;
    return 0;
}
#endif
