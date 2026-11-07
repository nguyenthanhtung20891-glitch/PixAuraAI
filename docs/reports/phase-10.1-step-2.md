# Phase 10.1 Step 2 — Local Apple native and iOS validation

Status: LOCAL HARDWARE GATE BLOCKED.

No local Mac Studio build or Simulator test was run. No Xcode version, no
simulator runtime, and no local test count is claimed.

When a Darwin host has the pinned developer directory, the existing commands
are:

```
bash scripts/check-apple.sh
bash scripts/check-ios-shell.sh
```

`check-apple.sh` configures `build/apple-host` with the Xcode `clang` and
`clang++`, runs ctest, runs the Swift package tests, builds `PixAuraCore`
for generic iOS with `CODE_SIGNING_ALLOWED=NO`, and tests that scheme on the
selected iPhone simulator. `check-ios-shell.sh` builds the PixAuraAI app
Debug and Release for generic iOS without signing, scans the release app,
and runs the app tests on the simulator. Those builds are unsigned simulator
and generic-iOS software builds. They are not signed physical-device builds.

Hosted Apple CI remains separate evidence. It does not satisfy this step.
The Mac2 family query now uses raw value 2002 so an Xcode 27.2 build does
not name the deprecated enumerator. That source change does not admit
Apple7-only devices and does not remove the software and virtual name
rejection. This Windows session did not compile it with Xcode 27.2. The Mac
Studio still runs `bash scripts/check-apple.sh` and
`bash scripts/check-ios-shell.sh` after a fast-forward pull.
`f7a61a019295772be700a7771083ffa612c98565` is not certified. Hosted
Xcode rejected the optional Mac2 raw value, and the Android review order
was still 101,102 before the required 102,101 move. That SHA is not rerun.
The Swift package's vendored codec target ignores only
`-Wshorten-64-to-32` on Apple platforms. Vendor JPEG, PNG, and zlib bytes
are unchanged. Owned C++ targets and the CMake `-Werror` gate on
`decode_codec.c` do not receive that exemption. Hosted Xcode 26.6 does not
prove the Xcode 27.2 compile.
`CPixAuraSQLite` ignores that same width diagnostic for `sqlite3.c` only.
`check-apple.sh` still passes `-Werror`. libsodium, Metal, and the owned
core do not receive it. The CMake SQLite target keeps `-Werror` plus its
existing unused-parameter exemption and does not gain this flag. The
amalgamation version, checksum, thread-safety, omitted extensions, and
length limits are unchanged.
The macOS package test now uses `xcodebuild -scheme PixAuraCore` with the
same per-target `KEEP_PRIVATE_EXTERNS` expansion as the iOS package builds.
`swift test` does not pass that setting into each target's `ld -r`, and
Xcode localizes hidden externs during that partial link. The missing groups
are those hidden symbols: Metal's private worker entry points, `crc32`,
libsodium, the codec exports, and the vendored SQLite API. Their visibility
is unchanged. A package-wide `KEEP_PRIVATE_EXTERNS=YES` stays rejected
because it breaks the app debug dylib. The SwiftUICore link warning is a
separate SDK note and does not define these symbols. This Windows session
did not run Xcode 27.2. Hosted Xcode 26.6 does not prove that linker.
The Product Owner reported that `bash scripts/check-apple.sh` completed on
Mac Studio Xcode 27.2 for `7ebce2ac4a33ca69f4a39f6f30ba63745d5ef111`. That
result belongs to that SHA. `check-ios-shell.sh` on that toolchain rejected
six nested main-queue callbacks in `ManualEditorModel.swift` because their
`[weak self]` capture differed from the worker closure's implicit strong
`self`. Those five worker closures now write `[self]` explicitly. The six
main-queue callbacks stay `[weak self]`, including both `assistEpoch`
checks. This session did not compile the iOS shell with Xcode 27.2.
On `c846719a4db11259f075cb7690f18ca630bfae15` the Product Owner reported
that `check-ios-shell.sh` compiled and then failed 18 `ShellUITests` on
iPhone Duo, iOS 27.2. The hierarchy showed the Vietnamese catalog
(`Mở ảnh`, `Áp dụng`, `Hoàn tác`, `Làm lại`, `Số bản sửa đã lưu: 1`)
while the suite asserts the English catalog. Every UI-test launch now
passes `-AppleLanguages (en)` and `-AppleLocale en_US`, which Foundation
reads before `String(localized:)` resolves. A normal launch does not
receive those arguments. Apply, Cancel, Undo, and Redo also expose those
English keys as accessibility identifiers, and tool readiness matches
the identifier or the visible label. The English sentences, action
order, human Apply, cancellation, history, undo/redo, export, and the
Vietnamese catalog stay in place. This Windows session did not run
Xcode 27.2. Step 2 stays LOCAL HARDWARE GATE BLOCKED until
`bash scripts/check-ios-shell.sh` passes on the implementation SHA.
