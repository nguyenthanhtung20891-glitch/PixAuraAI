# Intelligence Runtime Resource Admission v1

Phase 7 Step 7 remediation round 1. Additive foundation, not production admission.
Existing Phase 4 Budget Admission v1 and Phase 5 segmentation Inference Admission
v1, including all 256 MiB ceilings, are unchanged. No guessed production ceiling.

## Authority and ownership

One application/process-scoped shared intelligence Owner owns immutable model
mapping, runtime/session, tokenizer/preprocessor state and bounded shared caches.
Editors, UI, documents and requests never independently own heavyweight sessions.
The process owner is separate from every editing/document/resource context.
Its singleton is explicit architecture, not a global editing scheduler. This
round provides an internal mechanism and tests only: no application constructs
an embedding runtime, no production device policy, no platform/C ABI addition.

The trusted application supplies a reviewed adapter factory and truthful bounded
Costs/Observation. Providers never supply resource estimates or capability facts.
No runtime receives an editor, document, approval issuer, candidate or repository.
Output is untrusted data; only the existing Envelope/Plan/Registry/human approval
chain may eventually edit. An embedding dimension is never an editing parameter.

## Accounting

Costs names eight nonoverlapping simultaneously live capacity upper bounds:
mapping (conservative model mapping charge, not on-disk package footprint), session,
tokenizer, cache, preprocessing, scratch, output and device. An adapter must account
for its initialization peak as well as running peak in those bounds. Mapping bytes
are conservatively reserved; sharing/reclaimability cannot be assumed without
reviewed evidence. No absent cost may be replaced with a guessed RSS-derived value.

Observation is application-owned trusted evidence: available intelligence-plus-
protected headroom, positive Manual reserve, positive platform/UI/OS reserve,
nonzero monotonic observation sequence, known facts, foreground and pressure.
Checked arithmetic requires sum(Costs) <= available - Manual - platform reserve.
Overflow, unknown observations, zero reserves, insufficient room, background or
pressure rejects before factory/load. RSS is diagnostic evidence only and never
participates in lease arithmetic. No numeric production thresholds ship here;
synthetic test amounts confer no production authority.

Persistent charges/factory identity cannot change while a runtime is retained.
All request resources are scoped to the synchronous request and must be reclaimed
before return; adapters may not retain request output/scratch as an uncharged cache.
No retained output object is exposed in this foundation. A future transport must
respect these ownership rules or obtain separate review. READY retains persistent
resources until explicit application unload. No existing context cap is raised.

## Lifecycle and concurrency

Owner construction is lightweight. UNLOADED -> LOADING -> RUNNING -> READY;
READY -> RUNNING -> READY. Exactly one active heavy call process-wide, including
load; competitors return BUSY without queue/retry. A second editor shares that
same owner/session. Runtime failure destroys the session and returns FAILED.
stop is terminal until explicit unload; reject new work, cooperatively cancel,
join all calls, reclaim request resources, destroy session, mappings and caches.
unload follows that order and permits a later explicit lazy request. Cancellation
alone is nonsticky for the next admitted request. It does not promise immediate
memory reclamation from an executing adapter. No thread kill or callback API.

A client epoch is ephemeral application interaction ownership, not document
freshness/approval. Client abandonment (editor destruction, source/mode change)
suppresses its late result; final native source/revision/serial validation remains
mandatory downstream. Client stays alive through its call. Epoch exhaustion
fails closed. stop/unload may not be called synchronously from one's own active
request: application owns join ordering under ADR 0009. Locks protect short
admission/state transitions; load/run hold no owner mutex. A stopped caller must
join before process-owner destruction. No runtime/provider handles persist in
Schema 3 or application state restoration.

## Device policy still pending

Physical Android evidence must measure RAM/low-RAM class, process/native headroom,
pressure/trim/background, CPU latency, thermal behavior and package footprint.
Physical Apple evidence must measure footprint/VM, pressure/background, CPU,
thermal and package effects. Process RSS/private dirty/mapped pages/device memory
are distinct evidence, not interchangeable accounting. Hosted macOS/Simulator
cannot certify physical memory or Metal. Future trusted observations must be
connected to pressure-driven stop/join/unload; this round adds no OS adapter.

If admission fails, optional intelligence is unavailable; Manual/history/UI retain
their protected resources and existing independent operation. No OOM-and-OS-kill
strategy, hidden model load, network download or automatic retry is authorized.
DH-APPLE-METAL-01 DEFERRED / NOT WAIVED / NOT SATISFIED.

### Observation publication clarification

The additive owner accepts trusted observation updates separately from request
admission. Sequence regression and same-sequence conflicting facts fail closed;
RSS-only diagnostic changes grant no authority. A newer rejection/pressure fact
remains recorded, preventing reuse of older permissive facts. If active costs no
longer fit, cooperative cancellation suppresses that result. Application teardown
must still join and unload before memory reclamation. No OS policy/threshold is
invented by this mechanism. Adapter allocation failure returns RESOURCE and
releases its runtime/session; Manual owns separate protected resources.

Process shutdown is distinct from optional cache unload: shutdown permanently
rejects new calls and joins/releases resources; a later unload cannot revive the
owner. Applications must invoke shutdown and quiesce all callers before process
owner destruction. Explicit ordinary unload allows later lazy use only before
that terminal process shutdown.
