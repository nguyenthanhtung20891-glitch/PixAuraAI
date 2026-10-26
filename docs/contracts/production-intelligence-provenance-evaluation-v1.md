# Production intelligence provenance and evaluation v1

Phase 7 Step 7 remediation round 1. Offline engineering gates only. No production
model/runtime/provider is admitted. Existing Envelope/Plan/Registry remain frozen.

## Provenance manifest

scripts/intelligence-admission.mjs validates canonical UTF-8 JSON (sorted keys,
compact integers, terminal LF), version 1, maximum 65536 bytes. Exact root keys:
version, upstream, license, processor, architecture, conversion, artifacts,
runtime, custody, reproduction. Unknown/missing/duplicate/noncanonical fields fail.

upstream: project, revision, artifacts. license: identifier, text_sha256,
redistribution_evidence. processor: tokenizer and preprocessing identities.
conversion: tool, revision, argv, quantization, optimization, environment,
intermediates. architecture/runtime/custody/reproduction are bounded evidence
identities/references, not assertions that automatically pass a legal/quality gate.
Artifact lists contain exact name, positive safe-integer bytes and lowercase
SHA-256. Upstream/final lists require 1..16; intermediates 0..16 (empty only when
not applicable, subject to human review). Names are bounded ASCII leaves; traversal,
duplicate names, unexpected/missing files, size/hash mismatch reject. Other evidence
strings are bounded 512 bytes (custody/reproduction/redistribution 1024), no control
characters; argv is 1..32 bounded strings. Final artifacts are supplied as immutable
local buffers; the checker neither fetches nor executes them. File acquisition and
memory bounding are the trusted offline caller's responsibility.

A complete manifest must identify original checkpoint hashes, license text and
redistribution evidence, tokenizer/processor/config, exact conversion revision and
command/quantization/optimization/environment, intermediate hashes where applicable,
final identity, parity/reproduction and runtime compatibility, and custody. Evidence
references require actual review; arbitrary populated strings cannot certify facts.
The result always says productionAdmitted=false. This is a necessary structural/
byte-integrity gate, not sufficient production approval. EmbeddingGemma's known
final hash proves byte identity only; conversion provenance remains incomplete.

## Candidate-neutral paired measurement

The version-1 offline harness compares an explicit deterministic baseline with a
candidate on identical bounded image/request inputs. Neither adapter receives
expected answers or fixture labels. Golden native Registry metadata supplies the
bounded parameter universe for the eight authorized intelligence tools. It is not
an editor validator or a second execution implementation. Real Envelope/native
validation and model-quality admission remain additional mandatory gates.

Measurement actions are typed tool/value records, never executable Plan/Envelope.
Input fixtures <=128, image representation <=1 MiB, repeats 2..10, actions <=8.
The report records semantic selection, direction relative to native neutral,
abstention, neutral false positives, invalid observations, exposure/brightness
conflict, exact repeat consistency, raw per-case latency, optional separately
labelled memory evidence, and parameter error. Raw aggregate parameter error mixes
units and must not become a quality threshold; evaluate per-tool units before
admission. No scores are probabilities or execution permissions. Provider exceptions,
unsupported tools, noninteger/out-of-bounds/neutral parameters and malformed output
are invalid observations. This synchronous harness assumes bounded trusted adapter
execution; external runtime isolation/timeouts require their own admission owner.

Threshold policy is versioned as PENDING_PRINCIPAL_ARCHITECT; admitted=false always.
No production pass thresholds or device budgets are invented. No candidate is
admitted from generic upstream benchmarks. Baseline copies explicit semantic goals;
it does not claim image understanding. Current project-generated tiny RGB fixtures
exercise harness mechanics only, not natural-photo correction quality. They are
project-owned algorithmic data, not scraped photographs. Production benchmarks must
add independent, reviewed expected labels, realistic images/negative combinations,
per-tool calibration, failure/fallback and physical memory/latency evidence before
any admission decision. Expectations must be frozen before candidate measurements.

A future embedding provider must explicitly implement embedding -> bounded
similarity/classifier evidence -> confidence/margin -> abstention -> deterministic
calibration -> ordinary untrusted Envelope. No individual embedding dimension is
an edit value. No model-specific policy, NLP/chat, weights or runtime is added here.
No mutation, approval, candidate or history API is reachable from this tooling.
Production model UNRESOLVED; CPU VALIDATED; Vulkan/Metal UNKNOWN.
