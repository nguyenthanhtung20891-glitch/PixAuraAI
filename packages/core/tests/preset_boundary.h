#ifndef PIXAURA_PRESET_BOUNDARY_H
#define PIXAURA_PRESET_BOUNDARY_H
#include "pixaura/preset.h"
#include <string.h>
static const char preset_reference_input[] = "{\"schema_version\":1,\"recipe_version\":1,\"preset_id\":\"reference.batch\",\"name\":\"Reference only\",\"operations\":[{\"id\":\"00000000000000000000000000000001\",\"type\":\"pixaura.exposure\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":1000}},{\"id\":\"00000000000000000000000000000002\",\"type\":\"pixaura.brightness\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_linear\":100}}]}";
static const char preset_reference_canonical[] = "{\"name\":\"Reference only\",\"operations\":[{\"id\":\"00000000000000000000000000000001\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":1000},\"type\":\"pixaura.exposure\"},{\"id\":\"00000000000000000000000000000002\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_linear\":100},\"type\":\"pixaura.brightness\"}],\"preset_id\":\"reference.batch\",\"recipe_version\":1,\"schema_version\":1}\n";
static int preset_boundary_check(const uint8_t* manifest,uint64_t bytes,const uint8_t* identity) {
    pixaura_document_context context;pixaura_document_handle base,proposal;
    uint8_t output[16384];uint64_t needed=99;uint32_t changed=99;int status=14;
    memset(&context,0,sizeof(context));memset(&base,0,sizeof(base));memset(&proposal,0,sizeof(proposal));
    const char* binding="{\"operation_ids\":[\"00000000000000000000000000000701\",\"00000000000000000000000000000702\"]}";
    const char* revision="00000000000000000000000000000801";
    if(pixaura_preset_canonical(2,(const uint8_t*)preset_reference_input,sizeof(preset_reference_input)-1,output,sizeof(output),&needed)!=2||needed!=99)return 14;
    if(pixaura_preset_canonical(1,(const uint8_t*)preset_reference_input,sizeof(preset_reference_input)-1,output,sizeof(output),&needed)!=0||needed!=sizeof(preset_reference_canonical)-1||memcmp(output,preset_reference_canonical,(size_t)needed)!=0)return 14;
    if(pixaura_document_context_init(1,&context,sizeof(context),identity,32,0)!=0)return 14;
    if(pixaura_document_open(1,&context,manifest,bytes,identity,32,&base,0)!=0)goto done;
    if(pixaura_preset_propose(1,&context,&base,&base,(const uint8_t*)preset_reference_input,sizeof(preset_reference_input)-1,(const uint8_t*)binding,strlen(binding),(const uint8_t*)revision,32,&proposal,&changed)!=0||changed!=1||proposal.serial==base.serial)goto done;
    needed=0;if(pixaura_document_serialize(&context,&proposal,0,0,&needed,0)!=0||needed>sizeof(output))goto done;
    if(pixaura_document_serialize(&context,&proposal,output,sizeof(output),&needed,0)!=0)goto done;
    {pixaura_document_handle sentinel=base;changed=99;
    if(pixaura_preset_propose(1,&context,&base,&proposal,(const uint8_t*)preset_reference_input,sizeof(preset_reference_input)-1,(const uint8_t*)binding,strlen(binding),(const uint8_t*)revision,32,&sentinel,&changed)!=9||sentinel.serial!=base.serial||changed!=99)goto done;}
    if(pixaura_document_release(&context,&proposal)!=0)goto done;
    status=0;
done: (void)pixaura_document_context_destroy(&context);return status;
}
#endif
