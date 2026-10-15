#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <TargetConditionals.h>
#include "../metal_worker.hpp"
#include "../metal_shaders.hpp"
#include "../onnx_profile.hpp"
#include "pixaura/metal_exposure.h"
#include "pixaura/gpu.h"
#include <chrono>
#include <cstring>
#if !__has_feature(objc_arc)
#error Metal worker requires ARC ownership
#endif
namespace pixaura::apple {
namespace {
using pixaura::inference::require;
constexpr size_t tile_bytes=PIXAURA_GPU_MAX_PIXELS*16;
struct Worker final:MetalWorker {
    id<MTLDevice> device;id<MTLCommandQueue> queue;id<MTLComputePipelineState> pipeline;
    id<MTLBuffer> input,output;bool reference;
    explicit Worker(bool ref):reference(ref){
        device=MTLCreateSystemDefaultDevice();require(device!=nil,4);
        if(!ref){
#if TARGET_OS_SIMULATOR
            require(false,4);
#endif
            NSString* name=[device.name lowercaseString];
            require(([device supportsFamily:MTLGPUFamilyApple1]||[device supportsFamily:MTLGPUFamilyMac2])&&
                ![name containsString:@"software"]&&![name containsString:@"virtual"],4);
        }
        require(device.maxBufferLength>=tile_bytes,4);
        queue=[device newCommandQueue];input=[device newBufferWithLength:tile_bytes options:MTLResourceStorageModeShared];
        output=[device newBufferWithLength:tile_bytes options:MTLResourceStorageModeShared];require(queue&&input&&output,11);
        MTLCompileOptions* options=[MTLCompileOptions new];options.languageVersion=MTLLanguageVersion2_4;
        if(@available(iOS 18.0,macOS 15.0,*)){options.mathMode=MTLMathModeSafe;options.mathFloatingPointFunctions=MTLMathFloatingPointFunctionsPrecise;}
        else {options.fastMathEnabled=NO;}
        NSError* error=nil;
        NSString* source=[NSString stringWithUTF8String:(ref?segmentation_msl:exposure_msl)];
        id<MTLLibrary> library=[device newLibraryWithSource:source options:options error:&error];require(library&&error==nil,15);
        id<MTLFunction> function=[library newFunctionWithName:(ref?@"pixaura_reference_conv":@"pixaura_exposure")];require(function!=nil,15);
        pipeline=[device newComputePipelineStateWithFunction:function error:&error];require(pipeline&&error==nil,15);
        require(pipeline.maxTotalThreadsPerThreadgroup>=64,4);
    }
    int32_t dispatch(const float* source,float* destination,uint32_t count,const uint32_t* parameters,uint32_t fault,MetalTiming& timing)noexcept override{
        @autoreleasepool { @try {try{
            require(source&&destination&&parameters&&count&&count<=PIXAURA_GPU_MAX_PIXELS,1);
            require(parameters[0]==count,1);const auto bytes=static_cast<size_t>(count)*16;
            const auto start=std::chrono::steady_clock::now();std::memcpy(input.contents,source,bytes);std::memset(output.contents,0xcd,bytes);
            const auto uploaded=std::chrono::steady_clock::now();
            id<MTLCommandBuffer> command=[queue commandBuffer];id<MTLComputeCommandEncoder> encoder=[command computeCommandEncoder];require(command&&encoder,15);
            [encoder setComputePipelineState:pipeline];[encoder setBuffer:input offset:0 atIndex:0];[encoder setBuffer:output offset:0 atIndex:1];
            [encoder setBytes:parameters length:(reference?16u:12u) atIndex:2];
            [encoder dispatchThreadgroups:MTLSizeMake((count+63)/64,1,1) threadsPerThreadgroup:MTLSizeMake(64,1,1)];[encoder endEncoding];
            [command commit];
            // Even injected failures retain command/buffers through completion.
            [command waitUntilCompleted];
            require(command.status==MTLCommandBufferStatusCompleted&&command.error==nil,15);
#ifdef PIXAURA_CERTIFICATION_TESTING
            require(fault!=3,15);
#else
            (void)fault;
#endif
            const auto completed=std::chrono::steady_clock::now();std::memcpy(destination,output.contents,bytes);
            const auto read=std::chrono::steady_clock::now();
            timing.upload_ns+=static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(uploaded-start).count());
            timing.dispatch_ns+=static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(completed-uploaded).count());
            timing.readback_ns+=static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(read-completed).count());return 0;
        }catch(const pixaura::inference::Error& e){return e.code;}catch(...){return 15;}}
        @catch(NSException* failure){(void)failure;return 15;} }
    }
};
}
bool metal_available()noexcept{@autoreleasepool {@try{return MTLCreateSystemDefaultDevice()!=nil;}@catch(NSException* failure){(void)failure;return false;}}}
std::unique_ptr<MetalWorker> make_metal_worker(bool reference){@autoreleasepool {@try{return std::make_unique<Worker>(reference);}@catch(NSException* failure){(void)failure;throw pixaura::inference::Error{15};}}}
}
namespace {
struct Header{uint64_t tag; pixaura::apple::MetalWorker* worker;};
static_assert(sizeof(Header)<=sizeof(pixaura_metal_exposure_context::opaque),"Metal exposure owner ABI");
constexpr uint64_t live_tag=0x50584d4554414c31ULL,dead_tag=0x50584d4554414c30ULL;
Header live(pixaura_metal_exposure_context* c){pixaura::inference::require(c,1);Header h{};std::memcpy(&h,c->opaque,sizeof(h));
    pixaura::inference::require(c->api_version==1&&c->struct_size==sizeof(*c)&&!c->reserved&&h.tag==live_tag&&h.worker,1);return h;}
template<class F>int32_t boundary(F f)noexcept{@autoreleasepool {@try {try{return f();}
    catch(const pixaura::inference::Error& e){return e.code==4?5:e.code==11?8:e.code==1?1:14;}
    catch(const std::bad_alloc&){return 8;}catch(...){return 14;}}@catch(NSException* failure){(void)failure;return 14;}}}
}
extern "C" int32_t pixaura_metal_exposure_init(uint32_t version,pixaura_metal_exposure_context* c,uint32_t size){return boundary([&]{
    pixaura::inference::require(version==1&&c&&size==sizeof(*c),1);const auto* bytes=reinterpret_cast<const uint8_t*>(c);
    for(size_t i=0;i<sizeof(*c);++i)pixaura::inference::require(bytes[i]==0,1);
    auto worker=pixaura::apple::make_metal_worker(false);Header h{live_tag,worker.release()};c->api_version=1;c->struct_size=sizeof(*c);std::memcpy(c->opaque,&h,sizeof(h));return 0;
});}
extern "C" int32_t pixaura_metal_exposure_dispatch(pixaura_metal_exposure_context* c,const float* in,float* out,uint32_t count,float gain,uint32_t identity){return boundary([&]{
    auto h=live(c);pixaura::inference::require(in&&out&&count&&count<=PIXAURA_GPU_MAX_PIXELS&&identity<=1,1);
    const uint64_t bytes=static_cast<uint64_t>(count)*16;
    const auto source=reinterpret_cast<uintptr_t>(in),destination=reinterpret_cast<uintptr_t>(out),context=reinterpret_cast<uintptr_t>(c);
    pixaura::inference::require(bytes<=UINTPTR_MAX-source&&bytes<=UINTPTR_MAX-destination&&sizeof(*c)<=UINTPTR_MAX-context,1);
    pixaura::inference::require(!(source<destination+bytes&&destination<source+bytes)&&
        !(source<context+sizeof(*c)&&context<source+bytes)&&!(destination<context+sizeof(*c)&&context<destination+bytes),1);
    uint32_t parameters[3]{count,0,identity};std::memcpy(&parameters[1],&gain,4);pixaura::apple::MetalTiming timing;
    const auto status=h.worker->dispatch(in,out,count,parameters,0,timing);return status==4?5:status==11?8:status?14:0;
});}
extern "C" int32_t pixaura_metal_exposure_destroy(pixaura_metal_exposure_context* c){return boundary([&]{auto h=live(c);delete h.worker;h={dead_tag,nullptr};std::memcpy(c->opaque,&h,sizeof(h));return 0;});}
