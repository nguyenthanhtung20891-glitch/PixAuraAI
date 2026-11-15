# Phase 11 final G8/G10 execution plan

Baseline eb61735be29eb365db76fa57b74170c6656c9efd, independently accepted v12
software scope per Product Owner supplied Principal Architect decision. Phase11
engineering continues; no phase acceptance/closure, no Phase12 implementation.
Owner signing project remains local/unstaged; original XR evidence immutable.

1. Record approved commercial rules in ADR0024, MONETIZATION and product spec;
   record UTC reset/clock uncertainty, durable delivery and recovery semantics.
2. Implement/test a local transactional SQLite quota component: source deduplication,
   pending capacity, crash cleanup, durable result/idempotency, failed operations,
   UTC midnight/timezone/rollback/reboot and trial/paid/recovery projection. No
   document ABI/schema or native Apply authority change; distinguish standalone
   component evidence from deployed end-to-end admission enforcement.
3. Implement independently executable StoreKit2/Play Billing verification-facing
   and secure-cache components where possible. No fake IDs/accounts/transactions,
   no credentials in source, no purchase UI enabled without legitimate configuration.
   Validate failures and provide exact operator setup/sandbox instructions.
4. Write versioned tiled architecture/compatibility proposal before implementation;
   deterministic/adversarial goldens and bounded CPU evaluator only for proven
   pointwise/local-halo operations. Preserve existing whole-raster ABI/Schema3 and
   caps; stop incompatible production integration and request exact replacement
   contracts for Architect acceptance. No end-to-end12MP claim from synthetic tiles.
5. Minimal G7/G8/G10/G11 operator runbooks and blocker matrix; future Phases12–14
   planning only. Additive provenance, source/release/native/platform regressions.
6. Review secrets/diff; commit/push canonical main; require complete Foundation7/7
   and Native3/3 on final exactSHA. Stop for independent acceptance at real gates.
