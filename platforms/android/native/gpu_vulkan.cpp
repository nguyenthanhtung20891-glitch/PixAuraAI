#include <jni.h>
#include <vulkan/vulkan.h>
#include "pixaura/gpu.h"
#include "exposure_spirv.h"
#include <array>
#include <cstring>
#include <sstream>
#include <string>
namespace {
void require(bool ok,int32_t status=14){if(!ok)throw status;}
void vkcheck(VkResult r,int32_t status=14){require(r==VK_SUCCESS,r==VK_ERROR_OUT_OF_HOST_MEMORY||r==VK_ERROR_OUT_OF_DEVICE_MEMORY?8:status);}
template<class T> T info(VkStructureType s){T t{};t.sType=s;return t;}
struct Vulkan {
    VkInstance instance{};VkDevice device{};VkPhysicalDevice physical{};VkQueue queue{};
    VkBuffer buffers[2]{};VkDeviceMemory memory[2]{};void* mapped[2]{};
    VkShaderModule shader{};VkDescriptorSetLayout descriptor_layout{};
    VkPipelineLayout layout{};VkPipeline pipeline{};VkDescriptorPool descriptors{};
    VkDescriptorSet set{};VkCommandPool commands{};VkCommandBuffer command{};VkFence fence{};
    VkPhysicalDeviceProperties properties{};uint32_t family=0;bool submitted=false;
    bool hardware=false,pipeline_created=false,dispatched=false;int32_t stage=0;
    static constexpr VkDeviceSize bytes=PIXAURA_GPU_MAX_PIXELS*16;
    ~Vulkan(){
        // Never release resources still referenced by submitted work, including
        // timeout/failure paths. Device loss is terminal; no candidate publishes.
        if(device){if(submitted)(void)vkDeviceWaitIdle(device);
            if(fence)vkDestroyFence(device,fence,nullptr);
            if(commands)vkDestroyCommandPool(device,commands,nullptr);
            if(pipeline)vkDestroyPipeline(device,pipeline,nullptr);
            if(descriptors)vkDestroyDescriptorPool(device,descriptors,nullptr);
            if(layout)vkDestroyPipelineLayout(device,layout,nullptr);
            if(descriptor_layout)vkDestroyDescriptorSetLayout(device,descriptor_layout,nullptr);
            if(shader)vkDestroyShaderModule(device,shader,nullptr);
            for(unsigned i=0;i<2;++i){if(mapped[i])vkUnmapMemory(device,memory[i]);if(buffers[i])vkDestroyBuffer(device,buffers[i],nullptr);if(memory[i])vkFreeMemory(device,memory[i],nullptr);}
            vkDestroyDevice(device,nullptr);
        }if(instance)vkDestroyInstance(instance,nullptr);
    }
    void initialize(){
        stage=1;auto app=info<VkApplicationInfo>(VK_STRUCTURE_TYPE_APPLICATION_INFO);app.pApplicationName="PixAura GPU validation";app.apiVersion=VK_API_VERSION_1_0;
        auto inst=info<VkInstanceCreateInfo>(VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);inst.pApplicationInfo=&app;vkcheck(vkCreateInstance(&inst,nullptr,&instance),5);
        uint32_t count=0;vkcheck(vkEnumeratePhysicalDevices(instance,&count,nullptr),5);require(count>0&&count<=32,5);
        std::array<VkPhysicalDevice,32> devices{};vkcheck(vkEnumeratePhysicalDevices(instance,&count,devices.data()),5);
        for(uint32_t i=0;i<count;++i){VkPhysicalDeviceProperties p{};vkGetPhysicalDeviceProperties(devices[i],&p);
            // Record first rejected device too, but never dispatch on software.
            if(!physical)properties=p;
            if(pixaura_gpu_hardware(p.deviceType,p.vendorID,p.deviceName,static_cast<uint32_t>(std::strlen(p.deviceName)))!=0||p.apiVersion<VK_API_VERSION_1_0)continue;
            uint32_t n=0;vkGetPhysicalDeviceQueueFamilyProperties(devices[i],&n,nullptr);if(n==0||n>64)continue;
            std::array<VkQueueFamilyProperties,64> q{};vkGetPhysicalDeviceQueueFamilyProperties(devices[i],&n,q.data());
            const auto& limits=p.limits;
            for(uint32_t j=0;j<n;++j){const pixaura_vulkan_capabilities capabilities{p.apiVersion,q[j].queueCount,q[j].queueFlags,
                limits.maxComputeWorkGroupInvocations,limits.maxComputeWorkGroupSize[0],limits.maxComputeWorkGroupCount[0],
                limits.maxStorageBufferRange,limits.maxPushConstantsSize,limits.maxPerStageDescriptorStorageBuffers};
                if(pixaura_gpu_vulkan_capable(1,&capabilities)==0){physical=devices[i];properties=p;family=j;break;}}
            if(physical)break;
        }
        require(physical!=VK_NULL_HANDLE,5);hardware=true;
        stage=2;float priority=1;auto q=info<VkDeviceQueueCreateInfo>(VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO);q.queueFamilyIndex=family;q.queueCount=1;q.pQueuePriorities=&priority;
        auto d=info<VkDeviceCreateInfo>(VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);d.queueCreateInfoCount=1;d.pQueueCreateInfos=&q;vkcheck(vkCreateDevice(physical,&d,nullptr,&device),5);vkGetDeviceQueue(device,family,0,&queue);require(queue!=VK_NULL_HANDLE,5);
        stage=3;VkPhysicalDeviceMemoryProperties mp{};vkGetPhysicalDeviceMemoryProperties(physical,&mp);
        VkDescriptorBufferInfo buffer_info[2]{};
        for(unsigned i=0;i<2;++i){auto b=info<VkBufferCreateInfo>(VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);b.size=bytes;b.usage=VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;b.sharingMode=VK_SHARING_MODE_EXCLUSIVE;
            vkcheck(vkCreateBuffer(device,&b,nullptr,&buffers[i]),8);VkMemoryRequirements r{};vkGetBufferMemoryRequirements(device,buffers[i],&r);require(r.size<=524288,8);
            uint32_t type=UINT32_MAX;for(uint32_t j=0;j<mp.memoryTypeCount;++j)if((r.memoryTypeBits&(1u<<j))&&(mp.memoryTypes[j].propertyFlags&(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))==(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)){type=j;break;}
            require(type!=UINT32_MAX,5);auto a=info<VkMemoryAllocateInfo>(VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);a.allocationSize=r.size;a.memoryTypeIndex=type;vkcheck(vkAllocateMemory(device,&a,nullptr,&memory[i]),8);vkcheck(vkBindBufferMemory(device,buffers[i],memory[i],0));vkcheck(vkMapMemory(device,memory[i],0,bytes,0,&mapped[i]),8);
            buffer_info[i]={buffers[i],0,bytes};
        }
        stage=4;auto s=info<VkShaderModuleCreateInfo>(VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);s.codeSize=sizeof(exposure_spirv);s.pCode=exposure_spirv;vkcheck(vkCreateShaderModule(device,&s,nullptr,&shader));
        VkDescriptorSetLayoutBinding bindings[2]{};for(unsigned i=0;i<2;++i)bindings[i]={i,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1,VK_SHADER_STAGE_COMPUTE_BIT,nullptr};
        auto dl=info<VkDescriptorSetLayoutCreateInfo>(VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);dl.bindingCount=2;dl.pBindings=bindings;vkcheck(vkCreateDescriptorSetLayout(device,&dl,nullptr,&descriptor_layout));
        VkPushConstantRange push{VK_SHADER_STAGE_COMPUTE_BIT,0,12};auto pl=info<VkPipelineLayoutCreateInfo>(VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);pl.setLayoutCount=1;pl.pSetLayouts=&descriptor_layout;pl.pushConstantRangeCount=1;pl.pPushConstantRanges=&push;vkcheck(vkCreatePipelineLayout(device,&pl,nullptr,&layout));
        auto pi=info<VkComputePipelineCreateInfo>(VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO);pi.stage=info<VkPipelineShaderStageCreateInfo>(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);pi.stage.stage=VK_SHADER_STAGE_COMPUTE_BIT;pi.stage.module=shader;pi.stage.pName="main";pi.layout=layout;vkcheck(vkCreateComputePipelines(device,VK_NULL_HANDLE,1,&pi,nullptr,&pipeline));pipeline_created=true;
        VkDescriptorPoolSize pool_size{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,2};auto dp=info<VkDescriptorPoolCreateInfo>(VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);dp.maxSets=1;dp.poolSizeCount=1;dp.pPoolSizes=&pool_size;vkcheck(vkCreateDescriptorPool(device,&dp,nullptr,&descriptors));
        auto da=info<VkDescriptorSetAllocateInfo>(VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);da.descriptorPool=descriptors;da.descriptorSetCount=1;da.pSetLayouts=&descriptor_layout;vkcheck(vkAllocateDescriptorSets(device,&da,&set));
        VkWriteDescriptorSet writes[2]{};for(unsigned i=0;i<2;++i){writes[i]=info<VkWriteDescriptorSet>(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);writes[i].dstSet=set;writes[i].dstBinding=i;writes[i].descriptorCount=1;writes[i].descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;writes[i].pBufferInfo=&buffer_info[i];}vkUpdateDescriptorSets(device,2,writes,0,nullptr);
        auto cp=info<VkCommandPoolCreateInfo>(VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);cp.queueFamilyIndex=family;cp.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;vkcheck(vkCreateCommandPool(device,&cp,nullptr,&commands));
        auto ca=info<VkCommandBufferAllocateInfo>(VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);ca.commandPool=commands;ca.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;ca.commandBufferCount=1;vkcheck(vkAllocateCommandBuffers(device,&ca,&command));auto fi=info<VkFenceCreateInfo>(VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);vkcheck(vkCreateFence(device,&fi,nullptr,&fence));stage=5;
    }
    int32_t run(const float* input,float* output,uint32_t count,float gain,uint32_t identity)noexcept{
        try{require(count>0&&count<=PIXAURA_GPU_MAX_PIXELS,8);require(stage==5,5);
            const auto n=static_cast<std::size_t>(count)*16;std::memcpy(mapped[0],input,n);std::memset(mapped[1],0xcd,n);
            vkcheck(vkResetFences(device,1,&fence));vkcheck(vkResetCommandBuffer(command,0));auto begin=info<VkCommandBufferBeginInfo>(VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);begin.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;vkcheck(vkBeginCommandBuffer(command,&begin));
            auto barrier=info<VkMemoryBarrier>(VK_STRUCTURE_TYPE_MEMORY_BARRIER);barrier.srcAccessMask=VK_ACCESS_HOST_WRITE_BIT;barrier.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT;
            vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_HOST_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,1,&barrier,0,nullptr,0,nullptr);
            vkCmdBindPipeline(command,VK_PIPELINE_BIND_POINT_COMPUTE,pipeline);vkCmdBindDescriptorSets(command,VK_PIPELINE_BIND_POINT_COMPUTE,layout,0,1,&set,0,nullptr);
            struct Parameters{uint32_t count;float gain;uint32_t identity;}params{count,gain,identity};static_assert(sizeof(params)==12,"shader push layout");vkCmdPushConstants(command,layout,VK_SHADER_STAGE_COMPUTE_BIT,0,sizeof(params),&params);vkCmdDispatch(command,(count+63)/64,1,1);
            barrier.srcAccessMask=VK_ACCESS_SHADER_WRITE_BIT;barrier.dstAccessMask=VK_ACCESS_HOST_READ_BIT;vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_HOST_BIT,0,1,&barrier,0,nullptr,0,nullptr);vkcheck(vkEndCommandBuffer(command));
            auto submit=info<VkSubmitInfo>(VK_STRUCTURE_TYPE_SUBMIT_INFO);submit.commandBufferCount=1;submit.pCommandBuffers=&command;vkcheck(vkQueueSubmit(queue,1,&submit,fence));submitted=true;
            vkcheck(vkWaitForFences(device,1,&fence,VK_TRUE,30000000000ULL));submitted=false;dispatched=true;std::memcpy(output,mapped[1],n);return 0;
        }catch(int32_t status){stage=6;return status;}catch(...){stage=6;return 14;}
    }
};
int32_t dispatch(void* state,const float* in,float* out,uint32_t count,float gain,uint32_t identity){return static_cast<Vulkan*>(state)->run(in,out,count,gain,identity);}
std::string safe_name(const char* s){std::string out;for(unsigned i=0;i<256&&s[i];++i){const char c=s[i];out+=c>=32&&c<127&&c!='"'&&c!='\\'?c:'_';}return out;}
}
extern "C" JNIEXPORT jstring JNICALL Java_ai_pixaura_bridge_GpuValidation_nativeEvidence(JNIEnv* env,jobject){
    try{Vulkan gpu;int32_t status=0;uint32_t passed=0,attempted=0;try{gpu.initialize();
        std::array<float,PIXAURA_GPU_CASE_PIXELS*4> input{},output{};
        for(uint32_t i=0;i<pixaura_gpu_case_count();++i){int32_t ev=0;status=pixaura_gpu_case(i,input.data(),PIXAURA_GPU_CASE_PIXELS,&ev);if(status)break;
            pixaura_gpu_result result{};++attempted;status=pixaura_gpu_tile(1,input.data(),PIXAURA_GPU_CASE_PIXELS,ev,dispatch,&gpu,nullptr,nullptr,0,output.data(),&result);if(status||!result.used_gpu)break;++passed;}
        }catch(int32_t e){status=e;}
        const char* conclusion=status==5?"UNSUPPORTED":status==0&&passed==pixaura_gpu_case_count()?"PASS":"FAIL";
        std::ostringstream s;s<<"{\"platform\":\"Android\",\"backend\":\"Vulkan compute\",\"gpu\":\""<<safe_name(gpu.properties.deviceName)<<"\",\"vendor\":"<<gpu.properties.vendorID<<",\"device_type\":"<<gpu.properties.deviceType<<",\"api_version\":"<<gpu.properties.apiVersion<<",\"driver_version\":"<<gpu.properties.driverVersion<<",\"hardware\":"<<(gpu.hardware?"true":"false")<<",\"pipeline\":"<<(gpu.pipeline_created?"true":"false")<<",\"dispatch\":"<<(gpu.dispatched?"true":"false")<<",\"parity_passed\":"<<passed<<",\"test_count\":"<<attempted<<",\"expected_test_count\":"<<pixaura_gpu_case_count()<<",\"status_code\":"<<status<<",\"stage\":"<<gpu.stage<<",\"status\":\""<<conclusion<<"\"}";
        return env->NewStringUTF(s.str().c_str());
    }catch(...){return env->NewStringUTF("{\"platform\":\"Android\",\"backend\":\"Vulkan compute\",\"gpu\":\"unavailable\",\"api_version\":0,\"driver_version\":0,\"vendor\":0,\"device_type\":0,\"hardware\":false,\"pipeline\":false,\"dispatch\":false,\"parity_passed\":0,\"test_count\":0,\"expected_test_count\":14,\"status\":\"FAIL\"}");}
}
