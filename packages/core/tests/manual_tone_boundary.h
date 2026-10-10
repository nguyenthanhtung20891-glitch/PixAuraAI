#ifndef PIXAURA_MANUAL_TONE_BOUNDARY_H
#define PIXAURA_MANUAL_TONE_BOUNDARY_H
#include "pixaura/manual.h"
#include <stdio.h>
#include <string.h>
/* Shared C boundary test data, not platform business rules. */
static int manual_tone_boundary_check(const uint8_t* manifest,uint64_t bytes,const uint8_t* identity){
    const char* tools[]={"exposure","brightness","contrast","highlights","shadows","saturation","temperature","blur","sharpen"};
    const char* parameters[]={"milli_ev","milli_linear","milli_stops","milli_ev","milli_ev","milli_ratio","kelvin","milli_strength","milli_amount"};
    const int changed_values[]={1000,500,1000,1000,-1000,0,4000,1000,1000};
    const int neutral_values[]={0,0,0,0,0,1000,6504,0,0};
    const uint8_t session[]="00000000000000000000000000000800";
    pixaura_document_context context;pixaura_document_handle base;unsigned tool;int status=14;
    memset(&context,0,sizeof(context));memset(&base,0,sizeof(base));
    if(pixaura_document_context_init(1,&context,sizeof(context),identity,32,0)!=0)return 14;
    if(pixaura_document_open(1,&context,manifest,bytes,session,32,&base,0)!=0)goto done;
    for(tool=0;tool<9;++tool){
        char name[64],gesture_id[33],operation_id[33],revision_id[33],request[512];
        pixaura_manual_gesture gesture;pixaura_document_handle proposal;uint64_t sequence=0;uint32_t changed=99;unsigned update;int n;
        memset(&gesture,0,sizeof(gesture));memset(&proposal,0,sizeof(proposal));
        (void)snprintf(name,sizeof(name),"pixaura.%s",tools[tool]);
        (void)snprintf(gesture_id,sizeof(gesture_id),"%032x",900+tool);
        (void)snprintf(operation_id,sizeof(operation_id),"%032x",920+tool);
        (void)snprintf(revision_id,sizeof(revision_id),"%032x",940+tool);
        if(pixaura_manual_begin(1,&context,&base,(const uint8_t*)gesture_id,32,(const uint8_t*)name,strlen(name),0,0,&gesture)!=0)goto done;
        n=snprintf(request,sizeof(request),"{\"operations\":[{\"id\":\"%s\",\"type\":\"%s\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"%s\":%d}}]}",operation_id,name,parameters[tool],changed_values[tool]);
        if(n<=0||(size_t)n>=sizeof(request))goto done;
        for(update=0;update<1001;++update)if(pixaura_manual_update(&context,&gesture,&base,(const uint8_t*)request,(uint64_t)n,&sequence)!=0||sequence!=update+1)goto done;
        if(pixaura_manual_current(&context,&gesture,&base,1000)!=13||pixaura_manual_current(&context,&gesture,&base,1001)!=0)goto done;
        if(pixaura_manual_commit(&context,&gesture,&base,(const uint8_t*)revision_id,32,&proposal,&changed)!=0||changed!=1||proposal.serial==base.serial)goto done;
        if(pixaura_manual_commit(&context,&gesture,&base,(const uint8_t*)revision_id,32,&proposal,&changed)!=13)goto done;
        if(pixaura_manual_release(&context,&gesture)!=0||pixaura_document_release(&context,&proposal)!=0)goto done;
        if(pixaura_manual_begin(1,&context,&base,(const uint8_t*)gesture_id,32,(const uint8_t*)name,strlen(name),0,0,&gesture)!=0)goto done;
        n=snprintf(request,sizeof(request),"{\"operations\":[{\"id\":\"%s\",\"type\":\"%s\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"%s\":%d}}]}",operation_id,name,parameters[tool],neutral_values[tool]);
        if(n<=0||(size_t)n>=sizeof(request))goto done;
        if(pixaura_manual_update(&context,&gesture,&base,(const uint8_t*)request,(uint64_t)n,&sequence)!=0)goto done;
        if(pixaura_manual_commit(&context,&gesture,&base,(const uint8_t*)revision_id,32,&proposal,&changed)!=0||changed!=0||proposal.serial!=base.serial)goto done;
        if(pixaura_manual_release(&context,&gesture)!=0)goto done;
    }
    status=0;
done:
    (void)pixaura_document_context_destroy(&context);return status;
}
#endif
