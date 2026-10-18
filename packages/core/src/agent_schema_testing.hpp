#ifndef PIXAURA_AGENT_SCHEMA_TESTING_HPP
#define PIXAURA_AGENT_SCHEMA_TESTING_HPP
#ifdef PIXAURA_CERTIFICATION_TESTING
#include "pixaura/agent_schema.h"
extern "C" PIXAURA_API int32_t pixaura_agent_schema_test_checkpoint(
    pixaura_document_context*, void (*checkpoint)(void*), void* state);
extern "C" PIXAURA_API int32_t pixaura_agent_schema_test_boundary();
#endif
#endif
