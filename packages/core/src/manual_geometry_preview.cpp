#include "pixaura/manual.h"
#include "pixaura/geometry.h"
#include <vector>
#include <new>

extern "C" int32_t pixaura_manual_geometry_render(pixaura_document_context* document,
    const pixaura_manual_gesture* gesture, const pixaura_document_handle* live,
    pixaura_decode_context* raster, const pixaura_decode_handle* source,
    const pixaura_working_limits* limits, const pixaura_decode_handle* cancellation,
    const pixaura_preview_ticket* ticket, const pixaura_preview_request* request,
    pixaura_decode_handle* output, uint64_t* sequence) {
    if (!output || !sequence) return PIXAURA_INVALID_ARGUMENT;
    try {
        uint64_t required = 0, captured = 0;
        int32_t status = pixaura_manual_geometry_projection(document, gesture, live, nullptr, 0, &required, &captured);
        if (status != PIXAURA_DOCUMENT_BUFFER_TOO_SMALL) return status;
        if (required > PIXAURA_EVALUATION_MAX_REQUEST_BYTES) return PIXAURA_DOCUMENT_RESOURCE_LIMIT;
        std::vector<uint8_t> operations(static_cast<std::size_t>(required));
        status = pixaura_manual_geometry_projection(document, gesture, live, operations.data(), operations.size(), &required, &captured);
        if (status != 0) return status;
        status = pixaura_preview_current(raster, ticket, nullptr);
        if (status != 0) return status;
        pixaura_decode_handle evaluated{}, preview{};
        status = cancellation
            ? pixaura_working_evaluate_cancel(raster, source, 1, operations.data(), required, limits, cancellation, &evaluated)
            : pixaura_working_evaluate(raster, source, 1, operations.data(), required, limits, &evaluated);
        if (status != 0) return status;
        status = pixaura_preview_render_interactive(raster, &evaluated, ticket, request, &preview);
        (void)pixaura_decode_release(raster, &evaluated);
        if (status != 0) return status;
        status = pixaura_manual_geometry_current(document, gesture, live, captured);
        if (status == 0) status = pixaura_preview_current(raster, ticket, &preview);
        if (status != 0) { (void)pixaura_preview_release(raster, &preview); return status; }
        *output = preview; *sequence = captured; return 0;
    } catch (const std::bad_alloc&) { return PIXAURA_DOCUMENT_RESOURCE_LIMIT; }
      catch (...) { return PIXAURA_DOCUMENT_INTERNAL_ERROR; }
}
