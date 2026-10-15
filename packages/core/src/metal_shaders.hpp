// Generated from fixed repository Metal shaders. No model code.
#ifndef PIXAURA_METAL_SHADERS_HPP
#define PIXAURA_METAL_SHADERS_HPP
static constexpr const char* exposure_msl=R"PXMSL(#include <metal_stdlib>
using namespace metal;
struct Parameters { uint count; float gain; uint identity; };
kernel void pixaura_exposure(device const uint4* source [[buffer(0)]],
    device uint4* result [[buffer(1)]], constant Parameters& p [[buffer(2)]],
    uint i [[thread_position_in_grid]]) {
    if(i>=p.count) return;
    uint4 v=source[i];
    if(p.identity==0) v.xyz=as_type<uint3>(as_type<float3>(v.xyz)*p.gain);
    result[i]=v;
}
)PXMSL";
static constexpr const char* segmentation_msl=R"PXMSL(#include <metal_stdlib>
using namespace metal;
struct ConvParameters {uint count;float r,g,b;};
kernel void pixaura_reference_conv(device const float4* source [[buffer(0)]],
    device float4* result [[buffer(1)]],constant ConvParameters& p [[buffer(2)]],
    uint i [[thread_position_in_grid]]) {
    if(i>=p.count)return;
    // Explicit rounded operations: fma(x,w,0) is a product, fma(x,1,y) an add.
    // Safe math forbids replacing this chain with a different result.
    float r=fma(source[i].r,p.r,0.0f);
    float g=fma(source[i].g,p.g,0.0f);
    float b=fma(source[i].b,p.b,0.0f);
    float rg=fma(r,1.0f,g);
    float probability=fma(rg,1.0f,b);
    result[i]=float4(probability,0.0f,0.0f,0.0f);
}
)PXMSL";
#endif
