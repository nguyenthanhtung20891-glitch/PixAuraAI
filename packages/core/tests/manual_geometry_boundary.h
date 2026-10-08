#ifndef PIXAURA_MANUAL_GEOMETRY_BOUNDARY_TEST_H
#define PIXAURA_MANUAL_GEOMETRY_BOUNDARY_TEST_H
#include "pixaura/manual.h"
#include <string.h>
/* Consumer scenario only; all domain rules run through the shared core. */
static int manual_geometry_boundary_check(const uint8_t* manifest, uint64_t bytes, const uint8_t* context_id) {
    const uint8_t session[] = "00000000000000000000000000000702";
    const uint8_t gesture_id[] = "00000000000000000000000000000703";
    const uint8_t revision_id[] = "00000000000000000000000000000704";
    const uint8_t tool[] = "pixaura.rotate";
    const uint8_t operation[] = "{\"operations\":[{\"id\":\"00000000000000000000000000000705\",\"type\":\"pixaura.rotate\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"quarter_turns\":1}}]}";
    const uint8_t invalid[] = "{\"operations\":[{\"id\":\"00000000000000000000000000000705\",\"type\":\"pixaura.rotate\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"quarter_turns\":4}}]}";
    pixaura_document_context context;
    pixaura_document_handle base, proposal;
    pixaura_manual_gesture gesture;
    uint64_t sequence = 0;
    uint32_t changed = 42;
    int result = 14, status;
    unsigned i;
    memset(&context, 0, sizeof(context)); memset(&base, 0, sizeof(base));
    memset(&proposal, 0, sizeof(proposal)); memset(&gesture, 0, sizeof(gesture));
    if (pixaura_document_context_init(1, &context, sizeof(context), context_id, 32, 0) != 0) return 14;
    if (pixaura_document_open(1, &context, manifest, bytes, session, 32, &base, 0) != 0) goto done;
    if (pixaura_manual_geometry_begin(1, &context, &base, gesture_id, 32, tool, sizeof(tool)-1, 0, 0, &gesture) != 0) goto done;
    for (i = 0; i < 1001; ++i)
        if (pixaura_manual_geometry_update(&context, &gesture, &base, operation, sizeof(operation)-1, &sequence) != 0 || sequence != i+1) goto done;
    status = pixaura_manual_geometry_update(&context, &gesture, &base, invalid, sizeof(invalid)-1, &sequence);
    if (status != 7 || sequence != 1001) goto done;
    if (pixaura_manual_geometry_current(&context, &gesture, &base, 1000) != 13 ||
        pixaura_manual_geometry_current(&context, &gesture, &base, 1001) != 0) goto done;
    if (pixaura_manual_geometry_commit(&context, &gesture, &base, revision_id, 32, &proposal, &changed) != 0 || changed != 1 || proposal.serial == base.serial) goto done;
    if (pixaura_manual_geometry_commit(&context, &gesture, &base, revision_id, 32, &proposal, &changed) != 13) goto done;
    if (pixaura_manual_geometry_release(&context, &gesture) != 0 ||
        pixaura_manual_geometry_release(&context, &gesture) != 3) goto done;
    {
        const uint8_t crop_tool[]="pixaura.crop", crop_gesture[]="00000000000000000000000000000708";
        const uint8_t crop_revision[]="00000000000000000000000000000707";
        const uint8_t crop_operation[]="{\"operations\":[{\"id\":\"00000000000000000000000000000706\",\"type\":\"pixaura.crop\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"x_ppm\":999999,\"y_ppm\":999999,\"width_ppm\":1,\"height_ppm\":1}}]}";
        pixaura_document_handle cropped;
        if(pixaura_manual_geometry_begin(1,&context,&proposal,crop_gesture,32,crop_tool,sizeof(crop_tool)-1,0,0,&gesture)!=0) goto done;
        if(pixaura_manual_geometry_update(&context,&gesture,&proposal,crop_operation,sizeof(crop_operation)-1,&sequence)!=0||sequence!=1) goto done;
        if(pixaura_manual_geometry_commit(&context,&gesture,&proposal,crop_revision,32,&cropped,&changed)!=0||changed!=1) goto done;
        if(pixaura_manual_geometry_release(&context,&gesture)!=0) goto done;
    }
    result = 0;
done:
    if (pixaura_document_context_destroy(&context) != 0) result = 14;
    return result;
}
#endif
