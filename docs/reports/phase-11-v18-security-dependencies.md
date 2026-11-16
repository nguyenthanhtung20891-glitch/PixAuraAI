# Phase11 v18 source privacy / dependency review

No new production third-party dependency, network permission, telemetry, provider,
model, cloud image path, user account requirement or signing setting. New CPU v2
code uses frozen pinned SQLite/libjpeg/libpng/zlib and existing numerical kernels.
Private stream codec uses the SAME C-compatible Budget/Block layout as the frozen
owned JPEG system allocator, same quality/DCT/alpha-compositing/PNG filter/compression
settings. C owns longjmp and bounded allocations; C++ callbacks catch all errors.
Native writer reservation is an empty SQLite transaction (ROLLBACK only); no Schema3
row/table/trigger change. All clock/boot facts are local and excluded from store requests.

Android Billing9.1 remains NON-SHIPPING compile-only/test runtime. Its reviewed
POM/bytecode includes a CCT transport-send path and optional transport/location
runtime dependencies; no telemetry-free claim. Existing exact artifact SHA256
 d28286d4e4c18725510980de01211d9b72501e7146d96817fa0c9999af9b0d22
and Android SDK License/official Maven provenance remain unchanged. No Shipping
Billing inclusion is authorized by this review; runtime/license/privacy admission,
legitimate product/account setup and genuine sandbox transactions are outstanding.
New optional native handoff receives ONLY a signature-verified immutable record,
after protected-cache persistence, fenced against adapter close. PURCHASED alone,
pending, unavailable verification and malformed/suspended events never become grants.

Apple StoreKit2 handoff occurs only after .verified/environment/type/ownership
validation and durable ThisDeviceOnly Keychain caching. Protected-cache replay is
labelled as cached evidence; native policy must determine clock/offline/grace and
revocation. No new raw paid boolean or product IDs. Sink is unconfigured by default,
and no transaction/account operation is run by ordinary editing. Native bridge and
catalog setup are still required; callback compilation is not authenticated purchase
or end-to-end entitlement certification.

Apple OS clock uses public sysctl kern.boottime + mach_continuous_time, with
required-reason SystemBootTime35F9.1 for local elapsed-time/timer calculations.
[Apple required-reason documentation](https://developer.apple.com/documentation/bundleresources/app-privacy-configuration/nsprivacyaccessedapitypes/nsprivacyaccessedapitype).
[Continuous clock includes device sleep](https://developer.apple.com/documentation/kernel/1646199-mach_continuous_time).
Boot tuple is cached for one sampler lifetime; wall adjustment between process
lifetimes may cause conservative clock uncertainty, NEVER a fresh quota day or paid
grant. No device/advertising/vendor ID or bootsession UUID is read. No clock sample
is claimed to be authenticated absolute time. Native ledger handles high-water,
monotonic elapsed time and safe uncertainty/recovery.

Android OS clock reads SystemClock.elapsedRealtime and the public API24 BOOT_COUNT,
with no added runtime permission or setting mutation. [Android API reference](https://developer.android.com/reference/android/provider/Settings.Global#BOOT_COUNT).
Missing/denied boot observation fails closed; never fabricate a boot identity. Actual
Simulator/Emulator API tests do not prove physical clock changes, offline traffic,
backup/data protection or human accessibility. No private images or credentials are
uploaded; Linux tools run in a disposable container with a source-only read-only mount.
Initial stripped container capabilities blocked its own APT cache, retained in log;
retry grants only container-local package file/user capabilities, no host installation.

Key remaining conditions: CR18-01 native commit receipt; CR18-02 versioned app bridge;
real store configuration/signed sandbox lifecycle; physical G7/G8; human G11. Phase11
IN PROGRESS. CPUVALIDATED, Metal/VulkanprovidersUNKNOWN; hardware MetalPASS unchanged.
