#include "pixaura/model_package.h"
#include <stdio.h>
int main(void){
    pixaura_model_package* package=NULL;
    pixaura_model_diagnostic diagnostic={0,0,0,0};
    pixaura_model_info info={0};
    if(pixaura_model_package_verify(99,NULL,0,65536,&package,&diagnostic)!=PIXAURA_MODEL_VERSION||package||diagnostic.reason!=PIXAURA_MODEL_VERSION)return 1;
    if(pixaura_model_package_verify(1,NULL,0,65536,&package,&diagnostic)!=PIXAURA_MODEL_ARGUMENT||package)return 2;
    if(pixaura_model_package_info(1,NULL,&info)!=PIXAURA_MODEL_ARGUMENT||info.version)return 3;
    pixaura_model_package_release(NULL);
    puts("Model C boundary: 4 checks PASS");return 0;
}
