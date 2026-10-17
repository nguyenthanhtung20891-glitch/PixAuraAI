#ifndef PIXAURA_MODEL_PACKAGE_INTERNAL_HPP
#define PIXAURA_MODEL_PACKAGE_INTERNAL_HPP
#include "pixaura/model_package.h"
#include <array>
#include <string>
#include <vector>
namespace pixaura::model {
struct Failure { uint32_t reason, ordinal; };
inline void need(bool b, uint32_t r, uint32_t i=0) { if(!b) throw Failure{r,i}; }
struct File { std::string name; std::vector<uint8_t> bytes; };
struct Key { uint32_t id, purpose, activation, minimum_model_version; std::array<uint8_t,32> bytes; };
struct Trust { uint32_t version, generation, purpose; const Key* keys; size_t count; };
struct Binding {
    std::array<uint32_t,4> input{},output{};
    std::array<uint32_t,6> normalization{};
    std::string cpu_payload;
    uint32_t runtime_floor=0,api_floor=0,operator_set=0;
    uint32_t minimum_tier=0,cpu_threads=1,budget_class=0,accelerator_preference=0,required_validation=0;
    std::vector<uint32_t> operators;
};
std::vector<File> read_directory(const char*, uint64_t, uint64_t);
bool leaf(const std::string&);
#if defined(__GNUC__) || defined(__clang__)
__attribute__((visibility("hidden")))
#endif
int32_t verify_owned(uint32_t, std::vector<File>, const Trust&, uint64_t,
    pixaura_model_package**, pixaura_model_diagnostic*);
}
// Internal shared immutable ownership after admission; never a caller-constructible ABI.
struct pixaura_model_package {
    pixaura_model_info info{};
    std::vector<pixaura::model::File> files;
    pixaura::model::Binding binding;
};
#endif
