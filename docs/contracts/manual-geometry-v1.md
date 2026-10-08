# Manual geometry integration v1

Scope: authorized Phase 3 Step 2, crop and orthogonal rotation only. This connects
the [manual descriptor/gesture contract](manual-tools-v1.md) to existing document,
evaluation, persistence and PRV1 boundaries. No geometry model or scheduler is
added. Resize remains **PROPOSED / unimplemented**. No flip, perspective,
free-angle rotation, keystone, AI geometry or final editor UI is authorized.

## Frozen parameters and execution

| Tool / operation tuple | Integer parameters | Neutral | Validation |
| --- | --- | --- | --- |
| `pixaura.crop / 1 / 1` | `x_ppm,y_ppm`: 0..999999; `width_ppm,height_ppm`: 1..1000000 | (0,0,1000000,1000000) | x+width and y+height <=1000000 |
| `pixaura.rotate / 1 / 1` | `quarter_turns`: 0..3, clockwise, one turn = 90 degrees | 0 | Reject all other values |

PPM means millionths of the **current stage's** extent, not source pixels.
Crop uses the accepted outward rasterization: floor(W*x/1000000) through
ceil(W*(x+width)/1000000), with the corresponding Y rule. Positive valid crops
remain nonempty, including one-millionth and 1x1 inputs. Rotation copies all four
float components exactly using the accepted orthogonal mappings; odd turns swap
width/height. CPU semantics and GPU eligibility remain unchanged.

No clamp, locale parsing, platform rounding, float/exponent notation, negative
zero, string numbers, unknown keys or unknown operation/parameter versions.
The existing strict document parser validates every update; no platform repeats
these rules. Canonical UTF-8 JSON uses sorted keys, exact integer notation and
one trailing LF. Full-frame crop and zero-turn append are neutral.

The complete ordered stack remains inspectable. Crop->rotate differs from
rotate->crop. Crop->rotate->crop uses each intermediate extent. Four separate
90-degree commits restore pixels while retaining four operations/revisions;
there is no fusion, normalization, reordering or history collapse. Existing
historical neutral operations remain replayable; new neutral appends are omitted.

## Shared checked boundary and ownership

`manual.h` exposes geometry begin/update/projection/current/commit/cancel/release
and worker render. Family version 1 is additive; existing document context and
handle layouts are unchanged. A gesture token is 56 bytes with a distinct GEO1
kind, context identity and nonreused serial. Caller identities must be fresh
32-byte lowercase hex across their required lifetimes. No public native pointers,
network, discovery, plugins or hidden singleton is introduced.

Gestures and snapshots share the existing **64 owned handles per document
context**, never 64 additional handles. Closed gestures still consume ownership
until released. One active gesture per document within a context is admitted;
owners must route switching/interruption/background cancellation through cancel
and release. Destroy invalidates both registries and remains quiescent. Context
calls serialize metadata changes; heavy pixel work occurs outside that mutex.
Operation stack 256, retained operations/revisions 4096, request 64 KiB,
operation 1024 bytes and manifest 8 MiB limits remain unchanged, as do raster,
memory, dimension, tile and decode-context ceilings.

BEGIN -> zero or more UPDATE/PREVIEW -> COMMIT once; or CANCEL with no revision.
Updates replace a single pending operation and increment a nonwrapping sequence.
1,001 updates do not produce history. Invalid/allocation-failed updates retain the
last valid pending operation and sequence. A changed commit produces one detached
immutable proposal. No-update, neutral append or identical replacement produces
the same live snapshot and no history growth. Neutral replacement removes that
stack reference according to Step 1. Commit does not approve, persist or publish.

Changed commit returns a new owned document handle and `changed=1`. Unchanged
commit returns the borrowed live handle and `changed=0`: do not release it twice.
Allocation/admission failures preserve pending state for retry, including failure
to allocate the returned handle. The adapter trials the existing gesture on a
bounded metadata copy and closes the original only after ownership succeeds.
Stale project/document/source/session/generation/revision binding returns
STALE_BASE (9) and cancels. Closed/cancelled or old sequence returns CANCELLED
(13); released/foreign/wrong-kind tokens return INVALID_HANDLE (3). Other error
codes remain the document family codes. Outputs remain unchanged on failure,
except a projection size query/short buffer writes only `required` and returns 11.

## Preview and persistence integration

Projection is the existing complete ordered evaluation envelope, never a
durable edit. Worker render composes existing cancellable CPU evaluation and
`preview_render_interactive`, releases its private working candidate on every
path and returns an independently owned preview plus captured sequence. Supply
the **verified immutable working original matching the document source hash**,
not the last evaluated image. This binding remains the application service's
responsibility; a raster handle itself is not proof of a source digest.

PRV1 ticket checks precede evaluation and follow rendering. The captured manual
sequence and live binding are checked again after rendering. Optional sticky
cancellation uses the existing evaluation token; PRV1 alone fences publication
but does not promise preemption during evaluation. No new scheduler or latency
guarantee is introduced. For display installation the existing owner must check
**both** PRV1 eligibility and `geometry_current` under its lifecycle/publication
lock, serializing manual updates, cancel, commit and live revision changes with
check+display replacement. Failed/newest requests retain the last valid display.
Stop/join before context destruction and existing detached-copy ownership apply.

Application approval sends the proposal's fresh operation/revision and full stack
through the existing repository's expected-session/generation/revision/epoch
commit. Preview and detached proposals never write SQLite. Reopen trusts SQLite,
verifies immutable assets and restores canonical revision/stack state. Same
verified source + ordered parameters + versions reconstructs the same logical
state and CPU pixels. Fresh editing sessions reject old gestures/proposals.

Native integration tests cover real verified PNG import, PRV1 previews, ordered
pixels, durable commit/reopen, canonical replay, original hash and retry. C, JNI
and Swift consumers exercise the same shared boundary, with no platform-domain
rules. Full UI editing/lifecycle integration remains frozen later Steps 6/7.
