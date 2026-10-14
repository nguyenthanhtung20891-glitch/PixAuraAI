#include <chrono>
#include <cstdio>
#include <cstdint>
extern "C" int32_t pixaura_inference_test_boundary();
int main(){const auto start=std::chrono::steady_clock::now();const auto result=pixaura_inference_test_boundary();
    const auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count();
    std::printf("Inference shared corpus: 32 signed ONNX rows, 4 independent golden inputs, validation/lifetime/isolation/cancellation/resource cases; status=%d; total_ms=%lld\n",result,static_cast<long long>(ms));return result?1:0;}
