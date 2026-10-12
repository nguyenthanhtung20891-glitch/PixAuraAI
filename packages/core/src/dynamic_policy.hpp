#ifndef PIXAURA_DYNAMIC_POLICY_HPP
#define PIXAURA_DYNAMIC_POLICY_HPP
#include "pixaura/dynamic_policy.h"
namespace pixaura::policy {
struct State {
    bool enabled=false;
    pixaura_capability_profile profile{};
    pixaura_policy_observation observation{};
    pixaura_execution_strategy strategy{}, recovery{};
};
static_assert(sizeof(State)<=512,"fixed bounded runtime policy control");
static_assert(sizeof(pixaura_policy_observation)==48,"observation ABI family 1");
static_assert(sizeof(pixaura_execution_strategy)==56,"strategy ABI family 1");
int32_t configure(State&, const pixaura_capability_profile*, const pixaura_policy_observation*, uint64_t);
int32_t update(State&, const pixaura_policy_observation*, uint64_t);
int32_t admit(const State&, const pixaura_budget_request*, pixaura_budget_result*);
#ifdef PIXAURA_PREVIEW_TESTING
void test_epoch(pixaura_decode_context*, uint64_t);
#endif
}
#endif
