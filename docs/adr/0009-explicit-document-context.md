# ADR 0009: Explicit caller-owned document contexts

Status: Accepted
Date: 2026-10-03
Scope: Phase 2 Step 2; supersedes only ADR 0008's contract's process-local handle registry design

## Context
The user approved Step 1 and Step 2, then resolved the conflict between context-free registry functions and the requirement for no hidden global mutable state. All document, history, source, SQLite, canonical schema and migration decisions remain unchanged.

## Decision
Use a separate document API version 1 with explicit caller-owned context storage. Every document-handle call receives that context. No singleton or global mutable registry exists. ABI 1's shipping probe and zero feature bits remain unchanged.

The caller zero-initializes context storage and supplies a fresh context identity, unique across context lifetimes, from its OS identity service. The same application obligation already exists for document/session IDs. A handle contains that opaque identity plus a monotonically increasing serial; it contains no address. Serials never wrap or reuse within a context. Cross-context identity mismatches, zero/forged/released serials and destroyed contexts reject. Context identity must not be reused when reinitializing storage. Context storage must remain valid and may not be copied, modified or freed while calls can access it. As with all C pointers, the caller must supply truthful accessible memory; arbitrary dangling pointers cannot be validated by a C ABI.

Context initialization and destruction require exclusive caller access. Destruction invalidates all its handles and releases its registry. A second destruction of the same still-accessible storage safely rejects. While alive, registry operations are synchronized; acquired shared immutable snapshots survive concurrent handle release. Session transitions are submitted against explicit session/base/generation envelopes. Returned snapshots are detached proposals; the application service alone approves, persists and publishes them. No API creates approval authority from an actor string.

Caller buffers and the approved error layout retain their existing rules. Exceptions are contained at C entry points. Context and document allocation limits fail without replacing outputs or prior snapshots. Document family 1 limits each context to 64 live owned handles; release permits later allocations without serial reuse. This runtime ownership budget leaves schema-1 limits unchanged. The implementation exposes no file, decode, render or export capability.

## Alternatives
A process-global registry violates the approved requirement. Public document pointers cannot safely reject stale/double-released handles. Unqualified per-context counters collide across contexts. A raw context pointer allocated and freed by the library would make repeated context destruction unsafe; caller-owned storage retains a destroyed marker until the caller releases that storage.

## Consequences
Context identity uniqueness and storage lifetime are explicit caller obligations, rather than implicit global state. Handles are wider than the previous illustrative uint64 token. Caller-owned storage holds opaque implementation ownership data, never a public document pointer. Context initialization/destruction cannot race other calls; snapshot calls and handle release can. Exhaustion rejects rather than wrapping. The contract specifies this clarification before implementation.

## Validation
Independent C/C++ and platform boundary tests must exercise context isolation, invalid/destroyed handles, repeated destruction, stale tokens after new allocation and storage reuse with a fresh identity, serialization ownership, error/output sentinels and concurrent release. Native parser/history tests and ASan/UBSan exercise allocation and malformed input. Apple tests are wired into CI; Windows cannot certify their execution.
