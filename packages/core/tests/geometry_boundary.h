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
    if(pixaura_geometry_preflight(1,2,3,request,sizeof(request)-1,limits,&plan)!=0||plan.width!=1||plan.height!=3||plan.executable!=0)return 14;
    if(pixaura_geometry_tile_at(1,129,129,3,&tile)!=0||tile.x0!=128||tile.y0!=128||tile.x1!=129||tile.y1!=129)return 14;
    if(pixaura_geometry_preflight(1,2,3,(const uint8_t*)"{}",2,limits,&plan)==0)return 14;
    if(pixaura_cancel_create(c,1,&token)!=0)return 14;
    if(pixaura_working_evaluate_cancel(c,source,1,request,sizeof(request)-1,limits,&token,&output)!=5||memcmp(&output,&unchanged,sizeof(output))!=0)return 14;
    if(pixaura_working_evaluate_cancel(c,source,1,identity,sizeof(identity)-1,limits,&token,&output)!=0)return 14;
    if(pixaura_decode_release(c,&output)!=0)return 14;
    output=unchanged;
    if(pixaura_cancel_signal(c,&token)!=0||pixaura_cancel_signal(c,&token)!=0)return 14;
    if(pixaura_working_evaluate_cancel(c,source,1,identity,sizeof(identity)-1,limits,&token,&output)!=13||memcmp(&output,&unchanged,sizeof(output))!=0)return 14;
    if(pixaura_cancel_release(c,&token)!=0||pixaura_cancel_signal(c,&token)!=3||pixaura_cancel_release(c,&token)!=3)return 14;
    return 0;
}
#endif
