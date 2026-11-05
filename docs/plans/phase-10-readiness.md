# Phase 10 readiness proposal

Status: NOT STARTED. This document is not an authorization. Phase 10 implementation is not started. Phase 11 physical-device qualification stays
deferred until compliant hardware evidence exists. Phase 12 learned
intelligence stays deferred.

The proposal follows the observed Phase 9 closure
`85ec8988a88f482318a3f78b97dd6e5287bd9eda`. It does not change export, history,
approval, Schema 3, or the admitted provider.

## Proposed scope

1. Onboarding and empty states. Explain the first screen when no photo is
   open, and keep import as the only way a photo enters private storage.
2. Permissions UX. Explain a denied photo-library add or a cancelled document
   picker without requesting broad storage or photo-read access.
3. Accessibility. Keep export, save, share, and cancellation controls named,
   reachable, and distinguishable from Apply.
4. Localization. Replace inline strings with a reviewed catalog only after the
   current English copy is frozen. Do not change the meaning of export errors.
5. Responsive UI. Keep the export sheet from reducing the review viewport on
   narrow and wide layouts.
6. Error recovery. A failed or cancelled export stays failed or cancelled.
   Recovery is a new explicit export. There is no hidden retry.
7. Memory-pressure handling. Keep the existing admission limits. Do not add a
   process-RSS ceiling in this proposal.
8. Startup, load, and export performance. Collect hosted measurements
   separately from physical battery and thermal claims.
9. Logging and privacy. Logs must not contain image bytes, location metadata,
   or a destination URI that grants access. There is no upload.
10. Release readiness. Keep the existing release scanners. No certification-only
    export hook, test image, or research model asset ships.

None of these items is authorized by this document.
