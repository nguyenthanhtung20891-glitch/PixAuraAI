# Architectural decision index

| ADR | Status | Decision |
| --- | --- | --- |
| [0001](docs/adr/0001-native-shells-shared-core.md) | Accepted | Native mobile UIs; shared C++17 C ABI |
| [0002](docs/adr/0002-local-project-history.md) | Accepted | Immutable originals, local SQLite + asset store, shared revision graph |
| [0003](docs/adr/0003-deterministic-photo-pipeline.md) | Accepted | CPU semantic oracle, platform GPU adapters, SDR first |
| [0004](docs/adr/0004-ai-execution-boundary.md) | Accepted | Typed local plans, bounded executor, explicit approval |
| [0005](docs/adr/0005-local-inference-and-tiers.md) | Accepted | Provider-neutral AI; ONNX interchange, measured progressive tiers |
| [0006](docs/adr/0006-build-and-security-policy.md) | Accepted | Multi-host quality gates and dependency/privacy policy |
| [0007](docs/adr/0007-shell-navigation-and-build-inputs.md) | Accepted | Typed minimal shell coordinators, pinned inputs, deterministic Xcode project |

Accepted architecture does not imply an implementation or verified runtime. Exact models, Android GPU backend fallback coverage, shader tolerance and subscription grace remain evaluation items in relevant phases. Major changes require new numbered ADRs and index updates.
