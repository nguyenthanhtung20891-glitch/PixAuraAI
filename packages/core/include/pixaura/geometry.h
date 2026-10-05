#ifndef PIXAURA_GEOMETRY_H
#define PIXAURA_GEOMETRY_H
#include "evaluation.h"
#ifdef __cplusplus
extern "C" {
#endif
#define PIXAURA_GEOMETRY_API_VERSION 1u
#define PIXAURA_TILE_EDGE 128u
#define PIXAURA_MAX_TILES 4096u
#define PIXAURA_CANCELLED 13
typedef struct pixaura_geometry_plan {
    uint32_t api_version, struct_size, width, height;
    uint32_t operation_count, tile_count, tile_edge, executable;
    uint64_t raster_bytes, metadata_bytes, pixel_visits;
} pixaura_geometry_plan;
typedef struct pixaura_geometry_tile {
    uint32_t api_version, struct_size, index, x0, y0, x1, y1;
} pixaura_geometry_tile;
/* Exact schema-1 stack, unchanged order. Step 8 executes registered geometry
 * through existing evaluation APIs; this function itself only plans.
 * Output untouched on failure. Dimensions obey existing working limits. */
PIXAURA_API int32_t pixaura_geometry_preflight(uint32_t version, uint32_t width,
    uint32_t height, const uint8_t* request, uint64_t bytes,
    const pixaura_working_limits* limits, pixaura_geometry_plan* output);
PIXAURA_API int32_t pixaura_geometry_tile_at(uint32_t version, uint32_t width,
    uint32_t height, uint32_t index, pixaura_geometry_tile* output);
/* Tokens use context-scoped monotonic handles but a distinct registry/kind.
 * Signal/release can run during evaluation; context destroy requires quiescence.
 * Release invalidates lookup; an in-flight call retains its token until completion.
 * Tokens are one-shot, sticky, idempotently signalled, never reset. */
PIXAURA_API int32_t pixaura_cancel_create(pixaura_decode_context*, uint32_t version, pixaura_decode_handle*);
PIXAURA_API int32_t pixaura_cancel_signal(pixaura_decode_context*, const pixaura_decode_handle*);
PIXAURA_API int32_t pixaura_cancel_release(pixaura_decode_context*, const pixaura_decode_handle*);
PIXAURA_API int32_t pixaura_working_evaluate_cancel(pixaura_decode_context*,
    const pixaura_decode_handle* source, uint32_t version, const uint8_t* request,
    uint64_t bytes, const pixaura_working_limits*, const pixaura_decode_handle* cancellation,
    pixaura_decode_handle* output);
#ifdef __cplusplus
}
#endif
#endif
