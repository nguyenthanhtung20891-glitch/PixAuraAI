# Export and file lifecycle v1

Status: Phase 9 Step 1 contract. EXPORT IS NOT A REVISION.

A user export reads one ordinary committed revision and produces encoded bytes for an explicit platform destination. It does not issue approval, create a candidate, call the Assistance Provider, or change document history.

## Source binding

An export request names the owning document, the repository generation, the editor epoch, and the exact committed revision. The revision must already exist in ordinary history. A preview buffer, preview ticket, provider response, inspection record, or UI bitmap is not a source.

The selected revision is not silently replaced by the live revision. Exporting an older committed revision is allowed only when that revision is named and still present. Undo and redo change which revision is current; they do not rewrite the request. A mismatch is STALE and writes nothing.

## States

Export state is an operation state:

- IDLE
- VALIDATING
- RENDERING
- ENCODING
- READY_TO_SAVE
- SAVING
- SUCCEEDED
- FAILED
- CANCELLED
- STALE

SUCCEEDED means the encoded bytes were produced and, when a destination was requested, the platform observed the destination write complete. Presenting a share sheet is not SUCCEEDED delivery. CANCELLED and FAILED leave the document unchanged.

## Output

Supported formats are JPEG and PNG only.

Dimensions are the existing preview request: EXACT display size, or FIT inside a positive max width and max height that does not exceed the existing working limits. FIT never upscales. Unsupported sizes fail.

JPEG quality is an integer from 1 through 100. The default is 90. PNG has no quality parameter.

Orientation is the display orientation already applied by working normalization. The encoded file does not add a second EXIF orientation. Color samples are the existing 8-bit sRGB preview raster. ICC, GPS, EXIF, XMP, and IPTC are not copied. ICC preservation is not claimed. JPEG flattens straight alpha over opaque white. PNG writes straight RGBA and no ancillary text chunks. Other formats, CMYK, HDR, and animation fail clearly.

## Ownership and resources

One export is in flight for an owner. There is no queue, no hidden retry, and no background export. Encoded output is caller-owned and released on every failure path. Evaluation and preview buffers use the existing admission limits. Export adds no new process-RSS ceiling and no second raster engine.

Cancellation is export-only. Stop joins the export call and releases its buffers. A late callback after destroy cannot report success.

## Destination

Native code does not choose the user's photo library or a share target. Android save uses MediaStore or a user-selected document. Android share uses the system sharesheet with a narrowly granted URI. Apple save uses the Photos or document destination the user requested. Apple share uses the system share sheet. Permission denial fails without reading unrelated photos. A partial destination is not finalized. Failure does not delete the source and does not create a revision.

## Privacy

Save and share happen only after an explicit user action. There is no upload, telemetry image, or provider network call.

## Threats

| ID | Threat |
| --- | --- |
| A | Preview exported as the committed result |
| B | Stale revision exported under another identity |
| C | Cross-document export |
| D | Export mutates revision or history |
| E | Location or metadata leakage |
| F | Oversized allocation |
| G | Partial destination left as success |
| H | Double save |
| I | Callback after owner destruction |
| J | Provider or network dependency |
| K | Malformed encoder input |
| L | Android and Apple semantic mismatch |
| M | Test hook shipped in release |
| N | Share sheet treated as confirmed delivery |

## Tests required by later steps

Original revision, manual revision, approved ordinary revision, current revision, explicitly selected older revision, undo and redo, crop, rotation, tone and color, multiple operations, dimensions, orientation, alpha, JPEG quality, PNG samples, malformed input, unsupported format, resource denial, allocation failure, cancellation, and stop or destroy. JPEG equality is within the lossy codec. PNG samples match the evaluated raster in this color model.
