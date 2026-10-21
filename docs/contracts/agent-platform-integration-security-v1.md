# Agent platform integration and security v1

Phase 6 Step 7. Additive adapter contract; Steps 1-6 remain authoritative and
unchanged. Android/Kotlin/JNI and Apple/Swift/C are transport/application owners,
never separate semantic authorities. Intent != Plan != Approval != Candidate !=
Execution. Manual and approved Agent edits use the same ordinary native history.

## Authority and transport

An application-owned ManualEditor may inspect native binding, submit canonical
Structured Intent v1 to reference.deterministic.v1, validate/inspect Plan v1,
explicitly approve from human application confirmation, revoke/release approval,
execute with matching approval, or request cancellation. Planning/validation never
call approval. No payload asserts approval. Application code must not pass the
editor or approval-issuing capability to untrusted planner/model code. As already
frozen, the native host is trusted; this is not a sandbox against malicious native
application code or reflection within that host.

The sole additive C entrypoint `pixaura_editor_agent_plan` delegates to the existing
reference planner using the editor's hidden trusted document/live handle under
its owner gate. Same versioned request, canonical input, bounded caller-owned
disjoint output/required buffers, error/status/exception rules. No internal
pointer is exposed. Existing C structures/layouts and ABI compatibility remain.
All other Agent calls reuse approval/execution v1 without semantic changes.

Plans are copied immutable bytes (Swift value Data); approval is a distinct typed
owner object retaining its editor, containing only the existing opaque native
record. Native authority checks context, kind, serial, exact canonical bytes,
source serial, live state, registry and parameters. Cross-context use rejects;
no wrapper translates or upgrades authority. Approval release is idempotent at
the wrapper ownership layer, while native stale/double-release checks remain.
Revoke is explicit. No automatic retry, reapproval, fallback or replan.

JNI bounds copies to 8192 bytes and requires exact aligned direct-buffer storage.
Swift uses checked native structures and scoped byte buffers. No pointer escapes,
semantic JSON parsing or editing parameter math is introduced in either adapter.
Native version/size/reserved/enum/UTF-8/tool/parameter checks remain authoritative.
Receipt carries only plan SHA-256, source and resulting revision. No token/address
or user-content logging, callback, network, shell or generic plugin facility.

## Lifecycle and concurrency

Lock order: wrapper lifecycle gate -> short native owner/admission operations;
Swift condition count is accessed under its condition lock while gate admission
is held. Execution increments bounded in-flight count, releases wrapper gate,
then invokes native execution. Completion decrements/signals after all native
use ends. No wrapper gate spans private pixel work. Core Service gate,
agent_publication fence and repository epoch CAS remain unchanged Step 6 authority.
No reverse callback acquires a wrapper lock.

Close closes admission, requests native stop, joins all render and Agent calls,
then destroys once. Every concurrent close caller waits through destruction.
No internal unmanaged worker or forced interruption. Approvals closed after owner
shutdown perform no native access; context teardown owns remaining native leases.
Platform backgrounding behavior is not redesigned. ADR 0009 caller quiescence
continues to cover all raw C callers.

At most eight concurrent Agent transport calls are admitted per owner; the core
still admits ONE candidate, rejects BUSY as RESOURCE, never queues and never
consumes a competing approval. Existing limits are not increased. Cancellation
is nonsticky; source serial/ABA and publication fencing remain core authority.
Once execution begins approval remains consumed even on failure/cancel/stale.
Manual edit/undo/redo that wins source authority makes private work stale.

## Status parity and evidence

Both adapters preserve native status integers. 0 alone is success; 1 invalid
argument, 2 version, 3 invalid handle, 5 unknown tool, 6 invalid document/schema,
7 invalid parameter, 8 RESOURCE/BUSY, 9 STALE, 11 short buffer, 12 IO,
13 CANCELLED/stopped, 14 INTERNAL, 15 NOT_APPROVED remain failures. Unknown nonzero
values also fail. No mapping treats approval rejection as success or retry.
Swift's human-readable NOT_APPROVED text explicitly identifies missing approval.

Real Android instrumentation and Swift hosted/simulator tests exercise these
owners and native authority. Existing native malformed/forgery/rollback/CAS/race
corpora remain retained. Debug-only fixed native checkpoint controls have bounded
30-second waits, atomics and no platform pointer/callback. Instrumentation owns
resume/join ordering. Production scans reject these symbols and fixture markers;
release native objects contain no control or injected authority. No test state is
runtime trust or Schema 3 persistence.

Both complete exact implementation-SHA workflows and both exact closure-SHA
workflows must succeed independently. Apple hosted/simulator results are software
only. DH-APPLE-METAL-01 stays DEFERRED / NOT WAIVED / NOT SATISFIED. Phase 5 CPU
VALIDATED; Vulkan/Metal UNKNOWN. Production model UNRESOLVED. No model/NLP/runtime,
weights, AI UX, new tools or persistence. Step 8 and Phase 7 NOT STARTED.
