#ifndef PIXAURA_STORE_INGRESS_V3_H
#define PIXAURA_STORE_INGRESS_V3_H
#include "editor_v2.h"
#ifdef __cplusplus
extern "C" {
#endif
/* Private application-owned verifier configuration, not deserialized UI/cache
 * input. The callback MUST perform the actual configured store cryptography.
 * Fresh Apple evidence also requires the private verified StoreKit SDK fence.
 * Cached verification MUST NOT fabricate that fence or authenticated current UTC.
 * No callback may reenter native APIs; retain/release must not dispatch/wait.
 * No product catalog, trust anchor, network, checkout or paid flag is supplied
 * by the library. Missing genuine configuration leaves this path unavailable.
 */
typedef struct pixaura_store_v3_product {char identifier[129];uint32_t kind;} pixaura_store_v3_product;
typedef struct pixaura_store_v3_verified {
    uint32_t version,struct_size,kind,status,signed_millisecond,reserved;
    int64_t signed_seconds,expires;
    char product[129],transaction_digest[65],verified_key_identity[65];
} pixaura_store_v3_verified;
typedef int32_t (*pixaura_store_v3_verify)(void*,const uint8_t*,uint64_t,
    uint32_t origin,const char* native_nonce,uint64_t,pixaura_store_v3_verified*);
typedef struct pixaura_store_v3_configuration {
    uint32_t version,struct_size,store,environment,product_count,reserved;
    char application[256],key_identity[65];
    pixaura_store_v3_product products[8];
    void* context;
    int32_t (*retain)(void*);
    void (*release)(void*);
    pixaura_store_v3_verify verify;
} pixaura_store_v3_configuration;
typedef struct pixaura_store_v3_acknowledgment {
    uint32_t version,struct_size,replay,reserved;
    char product[129],identity[65],proof_digest[65];
} pixaura_store_v3_acknowledgment;
/* Bind once to an existing v2 runtime's SOLE commercial owner. These version3
 * calls neither reinterpret v1 handles nor create another editing authority.
 * All calls run on workers; retained callback owner outlives all native users.
 * Failed calls leave outputs untouched. A matching nonce attempt consumes it,
 * including verification failure. An ack is returned only after native COMMIT.
 */
PIXAURA_API int32_t pixaura_store_v3_bind(uint32_t,pixaura_v2_handle,const pixaura_store_v3_configuration*);
PIXAURA_API int32_t pixaura_store_v3_begin(uint32_t,pixaura_v2_handle,char*,uint64_t);
PIXAURA_API int32_t pixaura_store_v3_cancel(uint32_t,pixaura_v2_handle,const char*,uint64_t);
PIXAURA_API int32_t pixaura_store_v3_accept(uint32_t,pixaura_v2_handle,const uint8_t*,uint64_t,const char*,uint64_t,pixaura_store_v3_acknowledgment*);
#ifdef __cplusplus
}
#endif
#endif
