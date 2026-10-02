# Phase 1 dependency and build-input review

Reviewed 2026-10-02. Only official Android/JetBrains libraries and platform tooling are added. No photo, network, analytics, authentication, inference or cloud SDK is present. iOS has no external package; its local Swift package compiles the identical C++17 source.

| Input | Exact version | License / purpose / size consideration |
| --- | --- | --- |
| AGP / Gradle | 8.13.2 / 8.13 | Apache-2.0; build tooling only, excluded from APK |
| Kotlin / Compose compiler plugin | 2.2.21 | Apache-2.0; native Android presentation compiler; stdlib ships, compiler does not |
| Compose BOM | 2025.12.00 | Apache-2.0; fixes UI/material/test coordinates; resolved versions in app/gradle.lockfile |
| Activity Compose | 1.12.2 | Apache-2.0; native activity lifecycle/back handling |
| Lifecycle runtime/viewmodel/savedstate Compose | 2.10.0 | Apache-2.0; observable UI state, lifecycle collection and recreation |
| JUnit | 4.13.2 | EPL-1.0; JVM/test APK only; includes the temporary-directory security fix missing in old JUnit 4 versions |
| AndroidX test ext JUnit / runner | 1.3.0 / 1.7.0 | Apache-2.0; test APK only |
| Espresso core | 3.7.0 | Apache-2.0; explicit test-only pin replaces Compose's old Espresso 3.5.0, which failed on Android 16; no release footprint |
| Compose UI test | BOM-resolved | Apache-2.0; test/debug only, absent from release |
| JDK | 21.0.10 | Local JetBrains runtime / CI Temurin; GPL-2.0-with-classpath-exception; build only |
| Android SDK / NDK / CMake | API 36 / 28.2.13676358 / 3.22.1 | Official platform tools and bundled notices; NDK libc++ runtime ships with its LLVM exception license |

Provenance: Google Maven, Maven Central and Gradle Plugin Portal exclusively. [Official AGP compatibility](https://developer.android.com/build/releases/gradle-plugin) supplies the supported baseline; [official Compose BOM guidance](https://developer.android.com/develop/ui/compose/bom) explains the pinned mapping. No dynamic shipping versions. Lockfile records all resolved direct/transitive runtime/test configurations. Generated SHA-256 metadata enforces exact downloaded artifacts; Linux AAPT2 was independently downloaded from Google Maven and its digest added so Linux CI does not require disabling verification.

Security review: bounded version-only native probe, no runtime network dependency or dangerous permission; reviewed plugin origins and release compatibility, current JUnit security-fixed baseline, and locked artifact integrity. Final OSV audit covers 289 locked coordinates: 107 release-runtime entries have zero database findings; 18 flagged coordinates occur only in upstream build/test tooling. Advisory details, owner and review deadline are recorded in the Phase 1 report; this is not a zero-vulnerability certification. Checksum bootstrapping from official repositories is trust-on-first-use, not independent signature attestation. This review does not substitute for the release SBOM/continuous advisory review in G12. Test dependencies never ship in the release APK. Keep private content out of build reports and logs.

Measured combined three-ABI APKs before subsequent test-only changes: debug 14,838,271 bytes, unsigned release 11,439,371 bytes (about 14.15/10.91 MiB). These are total shell sizes, not isolated per-library contributions; no pre-shell APK existed for comparison. Release currently retains unshrunk official UI libraries, so R8/split-ABI optimization remains future release work. iOS binary size is unmeasured until macOS build evidence exists. Gradle wrapper JAR is the only committed binary tooling artifact, with the official 8.13 digest checked in Node tests; no app builds or model weights are committed.

Node 24 action upgrades are pinned official commits: checkout v5.0.0, upload-artifact v6.0.0, setup-java v5.0.0. Their immutable tag refs were read from GitHub's API. They require current hosted runners; no third-party emulator action was introduced.
