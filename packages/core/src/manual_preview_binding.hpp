#ifndef PIXAURA_MANUAL_PREVIEW_BINDING_HPP
#define PIXAURA_MANUAL_PREVIEW_BINDING_HPP
#include "pixaura/manual.h"
// Internal preflight for the existing worker adapter; no publication authority.
PIXAURA_API int32_t manual_render_binding(pixaura_document_context*,
    const pixaura_manual_gesture*,const pixaura_document_handle*,
    pixaura_decode_context*,const pixaura_decode_handle*,const pixaura_preview_ticket*);
#endif
