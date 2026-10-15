#ifndef PIXAURA_VULKAN_WORKER_HPP
#define PIXAURA_VULKAN_WORKER_HPP
#include <vulkan/vulkan.h>
#include "pixaura/gpu.h"
#include <array>
#include <cstring>
#include <chrono>
namespace pixaura::android_gpu {
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
    uint64_t upload_ns=0,dispatch_ns=0,readback_ns=0;
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
    void probe(bool physical_only=true){
        stage=1;auto app=info<VkApplicationInfo>(VK_STRUCTURE_TYPE_APPLICATION_INFO);app.pApplicationName="PixAura GPU validation";app.apiVersion=VK_API_VERSION_1_0;
        auto inst=info<VkInstanceCreateInfo>(VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);inst.pApplicationInfo=&app;vkcheck(vkCreateInstance(&inst,nullptr,&instance),5);
        uint32_t count=0;vkcheck(vkEnumeratePhysicalDevices(instance,&count,nullptr),5);require(count>0&&count<=32,5);
        std::array<VkPhysicalDevice,32> devices{};vkcheck(vkEnumeratePhysicalDevices(instance,&count,devices.data()),5);
        for(uint32_t i=0;i<count;++i){VkPhysicalDeviceProperties p{};vkGetPhysicalDeviceProperties(devices[i],&p);
            // Record first rejected device too, but never dispatch on software.
            if(!physical)properties=p;
            if((physical_only&&pixaura_gpu_hardware(p.deviceType,p.vendorID,p.deviceName,static_cast<uint32_t>(std::strlen(p.deviceName)))!=0)||p.apiVersion<VK_API_VERSION_1_0)continue;
            uint32_t n=0;vkGetPhysicalDeviceQueueFamilyProperties(devices[i],&n,nullptr);if(n==0||n>64)continue;
            std::array<VkQueueFamilyProperties,64> q{};vkGetPhysicalDeviceQueueFamilyProperties(devices[i],&n,q.data());
            const auto& limits=p.limits;
            for(uint32_t j=0;j<n;++j){const pixaura_vulkan_capabilities capabilities{p.apiVersion,q[j].queueCount,q[j].queueFlags,
                limits.maxComputeWorkGroupInvocations,limits.maxComputeWorkGroupSize[0],limits.maxComputeWorkGroupCount[0],
                limits.maxStorageBufferRange,limits.maxPushConstantsSize,limits.maxPerStageDescriptorStorageBuffers};
                if(pixaura_gpu_vulkan_capable(1,&capabilities)==0&&limits.maxPushConstantsSize>=16){physical=devices[i];properties=p;family=j;break;}}
            if(physical)break;
        }
        require(physical!=VK_NULL_HANDLE,5);
        hardware=pixaura_gpu_hardware(properties.deviceType,properties.vendorID,properties.deviceName,static_cast<uint32_t>(std::strlen(properties.deviceName)))==0;
    }
    void initialize(const uint32_t* code,size_t code_bytes,uint32_t push_bytes,bool physical_only=true){
        require(code&&code_bytes&&code_bytes%4==0&&push_bytes>=12&&push_bytes<=16);
        probe(physical_only);
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
        stage=4;auto s=info<VkShaderModuleCreateInfo>(VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);s.codeSize=code_bytes;s.pCode=code;vkcheck(vkCreateShaderModule(device,&s,nullptr,&shader));
        VkDescriptorSetLayoutBinding bindings[2]{};for(unsigned i=0;i<2;++i)bindings[i]={i,VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,1,VK_SHADER_STAGE_COMPUTE_BIT,nullptr};
        auto dl=info<VkDescriptorSetLayoutCreateInfo>(VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);dl.bindingCount=2;dl.pBindings=bindings;vkcheck(vkCreateDescriptorSetLayout(device,&dl,nullptr,&descriptor_layout));
        VkPushConstantRange push{VK_SHADER_STAGE_COMPUTE_BIT,0,push_bytes};auto pl=info<VkPipelineLayoutCreateInfo>(VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);pl.setLayoutCount=1;pl.pSetLayouts=&descriptor_layout;pl.pushConstantRangeCount=1;pl.pPushConstantRanges=&push;vkcheck(vkCreatePipelineLayout(device,&pl,nullptr,&layout));
        auto pi=info<VkComputePipelineCreateInfo>(VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO);pi.stage=info<VkPipelineShaderStageCreateInfo>(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);pi.stage.stage=VK_SHADER_STAGE_COMPUTE_BIT;pi.stage.module=shader;pi.stage.pName="main";pi.layout=layout;vkcheck(vkCreateComputePipelines(device,VK_NULL_HANDLE,1,&pi,nullptr,&pipeline));pipeline_created=true;
        VkDescriptorPoolSize pool_size{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,2};auto dp=info<VkDescriptorPoolCreateInfo>(VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);dp.maxSets=1;dp.poolSizeCount=1;dp.pPoolSizes=&pool_size;vkcheck(vkCreateDescriptorPool(device,&dp,nullptr,&descriptors));
        auto da=info<VkDescriptorSetAllocateInfo>(VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO);da.descriptorPool=descriptors;da.descriptorSetCount=1;da.pSetLayouts=&descriptor_layout;vkcheck(vkAllocateDescriptorSets(device,&da,&set));
        VkWriteDescriptorSet writes[2]{};for(unsigned i=0;i<2;++i){writes[i]=info<VkWriteDescriptorSet>(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET);writes[i].dstSet=set;writes[i].dstBinding=i;writes[i].descriptorCount=1;writes[i].descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;writes[i].pBufferInfo=&buffer_info[i];}vkUpdateDescriptorSets(device,2,writes,0,nullptr);
        auto cp=info<VkCommandPoolCreateInfo>(VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);cp.queueFamilyIndex=family;cp.flags=VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;vkcheck(vkCreateCommandPool(device,&cp,nullptr,&commands));
        auto ca=info<VkCommandBufferAllocateInfo>(VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO);ca.commandPool=commands;ca.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY;ca.commandBufferCount=1;vkcheck(vkAllocateCommandBuffers(device,&ca,&command));auto fi=info<VkFenceCreateInfo>(VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);vkcheck(vkCreateFence(device,&fi,nullptr,&fence));stage=5;
    }
    int32_t run_kernel(const float* input,float* output,uint32_t count,const void* params,uint32_t parameter_bytes,uint32_t fault=0)noexcept{
        try{require(input&&output&&params&&parameter_bytes>=12&&parameter_bytes<=16);require(count>0&&count<=PIXAURA_GPU_MAX_PIXELS,8);require(stage==5,5);
            const auto start=std::chrono::steady_clock::now();
            const auto n=static_cast<std::size_t>(count)*16;std::memcpy(mapped[0],input,n);std::memset(mapped[1],0xcd,n);
            const auto uploaded=std::chrono::steady_clock::now();
            vkcheck(vkResetFences(device,1,&fence));vkcheck(vkResetCommandBuffer(command,0));auto begin=info<VkCommandBufferBeginInfo>(VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);begin.flags=VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;vkcheck(vkBeginCommandBuffer(command,&begin));
            auto barrier=info<VkMemoryBarrier>(VK_STRUCTURE_TYPE_MEMORY_BARRIER);barrier.srcAccessMask=VK_ACCESS_HOST_WRITE_BIT;barrier.dstAccessMask=VK_ACCESS_SHADER_READ_BIT|VK_ACCESS_SHADER_WRITE_BIT;
            vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_HOST_BIT,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,0,1,&barrier,0,nullptr,0,nullptr);
            vkCmdBindPipeline(command,VK_PIPELINE_BIND_POINT_COMPUTE,pipeline);vkCmdBindDescriptorSets(command,VK_PIPELINE_BIND_POINT_COMPUTE,layout,0,1,&set,0,nullptr);
            vkCmdPushConstants(command,layout,VK_SHADER_STAGE_COMPUTE_BIT,0,parameter_bytes,params);vkCmdDispatch(command,(count+63)/64,1,1);
            barrier.srcAccessMask=VK_ACCESS_SHADER_WRITE_BIT;barrier.dstAccessMask=VK_ACCESS_HOST_READ_BIT;vkCmdPipelineBarrier(command,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_HOST_BIT,0,1,&barrier,0,nullptr,0,nullptr);vkcheck(vkEndCommandBuffer(command));
            auto submit=info<VkSubmitInfo>(VK_STRUCTURE_TYPE_SUBMIT_INFO);submit.commandBufferCount=1;submit.pCommandBuffers=&command;vkcheck(vkQueueSubmit(queue,1,&submit,fence));submitted=true;
#ifdef PIXAURA_CERTIFICATION_TESTING
            // Real submitted work remains owned through terminal cleanup.
            // Injected failure is protocol evidence, never an actual driver-loss claim.
            if(fault==3)throw int32_t{14};
#else
            (void)fault;
#endif
            vkcheck(vkWaitForFences(device,1,&fence,VK_TRUE,30000000000ULL));submitted=false;dispatched=true;
            const auto completed=std::chrono::steady_clock::now();std::memcpy(output,mapped[1],n);
            const auto read=std::chrono::steady_clock::now();
            upload_ns+=static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(uploaded-start).count());
            dispatch_ns+=static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(completed-uploaded).count());
            readback_ns+=static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(read-completed).count());return 0;
        }catch(int32_t status){stage=6;return status;}catch(...){stage=6;return 14;}
    }
    int32_t run(const float* input,float* output,uint32_t count,float gain,uint32_t identity)noexcept{
        struct Parameters{uint32_t count;float gain;uint32_t identity;}params{count,gain,identity};
        static_assert(sizeof(params)==12,"exposure shader push layout");
        return run_kernel(input,output,count,&params,sizeof(params));
    }
};
}
}
#endif
