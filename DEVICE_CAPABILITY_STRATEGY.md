# Adaptive device capability

Do not equate price, chipset name or OS version with capability. Detect memory budget, CPU threads, GPU features/limits, thermal state, provider operator support and storage headroom. Run short cancellable synthetic render/inference probes with no user photo. Cache versioned results per OS/runtime/model change; adapt during sessions to memory pressure and heat.

| Tier | Editing | AI | Initial resource target |
| --- | --- | --- | --- |
| T0 baseline | Tiled CPU; 720px preview | Rules/statistics advisor; no segmentation required | <=128 MiB incremental working set |
| T1 balanced | Validated GPU or CPU; 1280px preview | Compact quantized segmentation if admitted | <=256 MiB incremental working set |
| T2 quality | GPU; up to 2048px preview | Higher-quality segmentation/provider | <=512 MiB incremental working set |

These are profiling targets, not universal memory allowances. Admission uses min(tier cap, OS-safe available budget), reserves UI/decoder headroom and accounts for peak buffers and model resident memory. Default to T0 until probes pass. Downgrade immediately on pressure/thermal warning; promote only between jobs after stable probes. Export may take longer but preserves requested dimensions using tiles; never silently reduce export resolution. Refuse infeasible dimensions with an actionable message.

Target baseline devices: Android API 26 with 2 GB RAM and supported CPU ABI; iOS 16-compatible device. Phase 4 selects and records actual low/mid/high physical devices and adjusts limits before promotion. Supported release ABIs provisionally arm64-v8a and armeabi-v7a Android (native dependencies must prove compatibility); x86_64 for emulator tests; arm64 iOS. If runtime loses 32-bit support, manual core remains available without that runtime; change minimum support only through a product decision.

Measure cold/warm latency, p50/p95, peak resident memory, battery/thermal response and output quality. No AI tier prevents importing, manual tools, history, compare or export. Show available task capabilities, not a misleading device score. Model downloads include size and device suitability; missing pack has explicit offline status. Failed provider is quarantined per model/runtime version until retry/probe succeeds.
