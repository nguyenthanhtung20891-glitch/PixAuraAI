#ifndef PIXAURA_GESTURE_LIFECYCLE_BOUNDARY_TEST_H
#define PIXAURA_GESTURE_LIFECYCLE_BOUNDARY_TEST_H
#include "pixaura/manual.h"
#include <stdio.h>
#include <string.h>
/* Synchronous consumer scenario, never application domain semantics. */
static int gesture_lifecycle_boundary_check(const uint8_t* root,uint64_t root_bytes,
    const uint8_t* digest,uint64_t encoded_bytes,const uint8_t* identity) {
    pixaura_document_context document; pixaura_decode_context raster;
    pixaura_document_handle base,proposal; pixaura_decode_handle encoded,decoded,original,preview;
    pixaura_decode_limits dl; pixaura_working_limits wl; pixaura_manual_gesture gesture;
    pixaura_preview_ticket first,last; pixaura_preview_request request;
    char manifest[1024];uint64_t sequence=0;uint32_t state=0,changed=99;
    unsigned i;int n,result=14,document_live=0,raster_live=0;
    const uint8_t session[]="00000000000000000000000000006001",gid[]="00000000000000000000000000006002",next_gid[]="00000000000000000000000000006007",revision[]="00000000000000000000000000006003";
    const uint8_t tool[]="pixaura.exposure";
    const uint8_t operation[]="{\"operations\":[{\"id\":\"00000000000000000000000000006004\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":1000}}]}";
    uint8_t copy[24],after[24];
    memset(&document,0,sizeof(document));memset(&raster,0,sizeof(raster));
    memset(&base,0,sizeof(base));memset(&proposal,0,sizeof(proposal));memset(&gesture,0,sizeof(gesture));
    memset(&encoded,0,sizeof(encoded));memset(&decoded,0,sizeof(decoded));memset(&original,0,sizeof(original));memset(&preview,0,sizeof(preview));
    memset(&request,0,sizeof(request));request.version=1;request.struct_size=sizeof(request);request.mode=1;request.max_width=2;request.max_height=3;
    n=snprintf(manifest,sizeof(manifest),"{\"current_revision_id\":\"00000000000000000000000000006000\",\"document_id\":\"00000000000000000000000000006005\",\"operations\":[],\"project_id\":\"00000000000000000000000000006006\",\"redo\":[],\"revisions\":[{\"actor\":\"import\",\"id\":\"00000000000000000000000000006000\",\"parent_id\":null,\"plan_id\":null,\"stack\":[]}],\"schema_version\":1,\"source\":{\"byte_length\":%llu,\"metadata\":{\"codec\":\"png\",\"has_alpha\":true,\"height\":3,\"icc_sha256\":null,\"orientation\":1,\"width\":2},\"sha256\":\"%.*s\"}}",(unsigned long long)encoded_bytes,64,(const char*)digest);
    if(n<=0||(size_t)n>=sizeof(manifest))return 14;
    if(pixaura_document_context_init(1,&document,sizeof(document),identity,32,0)!=0)goto done;
    document_live=1;
    if(pixaura_document_create(1,&document,(const uint8_t*)manifest,(uint64_t)n,session,32,&base,0)!=0)goto done;
    if(pixaura_decode_default_limits(1,&dl)!=0||pixaura_working_default_limits(1,&wl)!=0)goto done;
    if(pixaura_decode_context_init(1,&raster,sizeof(raster),identity,32,&dl)!=0)goto done;
    raster_live=1;
    if(pixaura_decode_open(&raster,root,root_bytes,digest,64,encoded_bytes,&encoded)!=0||pixaura_decode_image(&raster,&encoded,&decoded)!=0||pixaura_working_normalize(&raster,&decoded,&wl,&original)!=0)goto done;
    if(pixaura_decode_release(&raster,&encoded)!=0||pixaura_decode_release(&raster,&decoded)!=0)goto done;
    if(pixaura_manual_edit_begin(1,&document,&base,gid,32,tool,sizeof(tool)-1,0,0,&raster,&original,&gesture)!=0)goto done;
    if(pixaura_manual_state(&document,&gesture,&state,&sequence)!=0||state!=PIXAURA_MANUAL_ACTIVE||sequence!=0)goto done;
    if(pixaura_manual_update(&document,&gesture,&base,operation,sizeof(operation)-1,&sequence)!=0)goto done;
    if(pixaura_manual_preview_ticket(&document,&gesture,&base,&first,&sequence)!=0)goto done;
    if(pixaura_manual_render(&document,&gesture,&base,&raster,&original,&wl,0,&first,&request,&preview,&sequence)!=0||sequence!=1)goto done;
    if(pixaura_preview_copy(&raster,&preview,0,copy,24)!=0)goto done;
    for(i=0;i<10000;++i)if(pixaura_manual_update(&document,&gesture,&base,operation,sizeof(operation)-1,&sequence)!=0||sequence!=i+2)goto done;
    if(pixaura_manual_preview_ticket(&document,&gesture,&base,&last,&sequence)!=0||sequence!=10001||last.generation!=first.generation+10000)goto done;
    if(pixaura_manual_preview_current(&document,&gesture,&base,1,&first,&preview)!=13)goto done;
    if(pixaura_preview_copy(&raster,&preview,0,after,24)!=0||memcmp(copy,after,24)!=0)goto done;
    if(pixaura_preview_release(&raster,&preview)!=0)goto done;
    if(pixaura_manual_commit(&document,&gesture,&base,revision,32,&proposal,&changed)!=0||changed!=1)goto done;
    if(pixaura_manual_state(&document,&gesture,&state,&sequence)!=0||state!=PIXAURA_MANUAL_COMPLETED)goto done;
    if(pixaura_preview_current(&raster,&last,0)!=13||pixaura_manual_commit(&document,&gesture,&base,revision,32,&proposal,&changed)!=13)goto done;
    if(pixaura_manual_release(&document,&gesture)!=0)goto done;
    if(pixaura_manual_edit_begin(1,&document,&proposal,next_gid,32,tool,sizeof(tool)-1,0,0,&raster,&original,&gesture)!=0)goto done;
    if(pixaura_manual_interrupt(&document)!=0||pixaura_manual_interrupt(&document)!=0||pixaura_manual_state(&document,&gesture,&state,&sequence)!=3)goto done;
    if(copy[3]!=255)goto done;
    result=0;
done:
    if(document_live&&pixaura_document_context_destroy(&document)!=0)result=14;
    if(raster_live&&pixaura_decode_context_destroy(&raster)!=0)result=14;
    return result;
}
#endif
