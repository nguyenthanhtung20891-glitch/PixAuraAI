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
std::vector<File> read_directory(const char*, uint64_t, uint64_t);
bool leaf(const std::string&);
#if defined(__GNUC__) || defined(__clang__)
__attribute__((visibility("hidden")))
#endif
int32_t verify_owned(uint32_t, std::vector<File>, const Trust&, uint64_t,
    pixaura_model_package**, pixaura_model_diagnostic*);
}
#endif
