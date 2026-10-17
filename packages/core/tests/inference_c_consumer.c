#include "pixaura/inference.h"
#include "pixaura/inference_admission.h"
#include <string.h>
int main(void){pixaura_inference_context c={0};uint8_t identity[32];memset(identity,'a',32);
    pixaura_inference_descriptor d={0};if(pixaura_inference_descriptor_query(1,"cpu.reference.v1",16,&d,sizeof(d)))return 1;
    if(pixaura_inference_context_init(1,&c,sizeof(c),identity,32,1048576))return 2;
    pixaura_inference_resources r={0};if(pixaura_inference_resources_query(&c,&r,sizeof(r))||r.handles||r.active)return 3;
    pixaura_execution_strategy strategy={0};if(pixaura_inference_policy_query(1,&c,&strategy)!=1)return 5;
    if(pixaura_inference_context_destroy(&c)||pixaura_inference_context_destroy(&c)!=PIXAURA_INFERENCE_STALE_HANDLE){return 4;}return 0;}
