#include <metal_stdlib>
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
