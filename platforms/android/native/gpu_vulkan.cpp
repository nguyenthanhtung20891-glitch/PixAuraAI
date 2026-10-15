#include <jni.h>
#include "vulkan_worker.hpp"
#include "pixaura/gpu.h"
#include "exposure_spirv.h"
#include <array>
#include <cstring>
#include <sstream>
#include <string>
namespace {
using pixaura::android_gpu::Vulkan;
int32_t dispatch(void* state,const float* in,float* out,uint32_t count,float gain,uint32_t identity){return static_cast<Vulkan*>(state)->run(in,out,count,gain,identity);}
std::string safe_name(const char* s){std::string out;for(unsigned i=0;i<256&&s[i];++i){const char c=s[i];out+=c>=32&&c<127&&c!='"'&&c!='\\'?c:'_';}return out;}
}
extern "C" JNIEXPORT jstring JNICALL Java_ai_pixaura_bridge_GpuValidation_nativeEvidence(JNIEnv* env,jobject){
    try{Vulkan gpu;int32_t status=0;uint32_t passed=0,attempted=0;try{gpu.initialize(exposure_spirv,sizeof(exposure_spirv),12);
        std::array<float,PIXAURA_GPU_CASE_PIXELS*4> input{},output{};
        for(uint32_t i=0;i<pixaura_gpu_case_count();++i){int32_t ev=0;status=pixaura_gpu_case(i,input.data(),PIXAURA_GPU_CASE_PIXELS,&ev);if(status)break;
            pixaura_gpu_result result{};++attempted;status=pixaura_gpu_tile(1,input.data(),PIXAURA_GPU_CASE_PIXELS,ev,dispatch,&gpu,nullptr,nullptr,0,output.data(),&result);if(status||!result.used_gpu)break;++passed;}
        }catch(int32_t e){status=e;}
        const char* conclusion=status==5?"UNSUPPORTED":status==0&&passed==pixaura_gpu_case_count()?"PASS":"FAIL";
        std::ostringstream s;s<<"{\"platform\":\"Android\",\"backend\":\"Vulkan compute\",\"gpu\":\""<<safe_name(gpu.properties.deviceName)<<"\",\"vendor\":"<<gpu.properties.vendorID<<",\"device_type\":"<<gpu.properties.deviceType<<",\"api_version\":"<<gpu.properties.apiVersion<<",\"driver_version\":"<<gpu.properties.driverVersion<<",\"hardware\":"<<(gpu.hardware?"true":"false")<<",\"pipeline\":"<<(gpu.pipeline_created?"true":"false")<<",\"dispatch\":"<<(gpu.dispatched?"true":"false")<<",\"parity_passed\":"<<passed<<",\"test_count\":"<<attempted<<",\"expected_test_count\":"<<pixaura_gpu_case_count()<<",\"status_code\":"<<status<<",\"stage\":"<<gpu.stage<<",\"status\":\""<<conclusion<<"\"}";
        return env->NewStringUTF(s.str().c_str());
    }catch(...){return env->NewStringUTF("{\"platform\":\"Android\",\"backend\":\"Vulkan compute\",\"gpu\":\"unavailable\",\"api_version\":0,\"driver_version\":0,\"vendor\":0,\"device_type\":0,\"hardware\":false,\"pipeline\":false,\"dispatch\":false,\"parity_passed\":0,\"test_count\":0,\"expected_test_count\":14,\"status\":\"FAIL\"}");}
}
