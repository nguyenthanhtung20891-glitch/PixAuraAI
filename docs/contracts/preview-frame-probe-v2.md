# Preview frame currency probe v2

Implementation record under approved CR18-02. This additive read-only interface is
not independent acceptance, editing authority, a revision, a commercial receipt,
or provider/GPU admission. Existing v1 headers/handles and the legacy v2
`frame_current` boolean behavior remain unchanged.

`pixaura_v2_frame_probe(version, editor, frame, worker_wait, output)` accepts only
versioned v2 handles. `worker_wait=0` does not wait on the registry, context metadata
or policy mutex; `worker_wait=1` is for command workers. A successful query returns
API status zero and writes CURRENT=0, BUSY=1 or STALE=2. A failed query leaves the
output untouched. In particular, resource-policy denial remains an API error,
not BUSY; it cannot cause an endless transient-lock retry.

CURRENT checks the same stopped/cancelled/context/budget-owner/generation/epoch and
repository-epoch predicates as the original native check, then the current preview
edge policy. Registry entries and frame storage stay owned while checked. No new
raster, execution lease, source mutation, quota use or approval is created by the
query. Existing frame/consumer reservations remain accounted for until their real
consumers retire; no budget or RSS ceiling is enlarged.

The old boolean probe conflated a busy metadata mutex with an obsolete frame.
Discarding that sole worker result could leave a valid preview undisplayed. The
new Swift facade reports installation as installed/busy/discarded. Its existing
`install` API still returns a boolean. On the UI lane the application retains one
pending frame, yields before a BUSY retry, and allows a newer result to replace it.
Every retry revalidates owner, newest display serial and native currency; STALE or
failed queries never regain authority. Cancel/close clears pending work. This is a
bounded mailbox, not additional render requests or a test-timeout extension.

Android performs installation on its serial command worker and uses the waiting
probe there. The original public `install` and old JNI opcodes remain available;
new internal worker installation and probe opcodes are additive. It does not wait
on a native metadata lock from the shipping UI thread.

Evidence under `build/phase-11-final/` includes native metadata-contention tests,
C consumer CURRENT/STALE/invalid-handle/invalid-wait checks, resource denial with
unchanged output, and Swift tests using an actual held facade lock. The Swift test
requires BUSY not to install pixels, then installs after release and rejects an
older display serial without changing history. See the chronological checkpoint
for exact run results and still-running sanitizer/platform regressions. These
checks are software evidence, never physical-device qualification.
