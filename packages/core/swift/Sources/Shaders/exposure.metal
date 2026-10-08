#include <metal_stdlib>
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
