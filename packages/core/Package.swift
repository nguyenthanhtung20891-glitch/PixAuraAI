// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "PixAuraCore",
    platforms: [.iOS(.v16), .macOS(.v13)],
    products: [.library(name: "PixAuraCore", targets: ["PixAuraCore"])],
    targets: [
        .target(name: "CPixAuraSQLite", path: "vendor/sqlite", exclude: ["LICENSE.md", "provenance.json"], sources: ["sqlite3.c"], publicHeadersPath: ".",
                cSettings: [.define("SQLITE_THREADSAFE", to: "1"), .define("SQLITE_DQS", to: "0"),
                    .define("SQLITE_OMIT_LOAD_EXTENSION"), .define("SQLITE_OMIT_SHARED_CACHE"),
                    .define("SQLITE_DEFAULT_MEMSTATUS", to: "0"), .define("SQLITE_MAX_LENGTH", to: "8388608"),
                    .define("SQLITE_MAX_SQL_LENGTH", to: "65536"), .define("SQLITE_MAX_ATTACHED", to: "0"),
                    .define("SQLITE_API", to: "__attribute__((visibility(\"hidden\")))")]),
        .target(name: "CPixAuraCore", dependencies: ["CPixAuraSQLite"], path: ".",
                exclude: ["Package.swift", "swift", "tests", "vendor"],
                sources: ["src/core.cpp", "src/document.cpp", "src/document_api.cpp", "src/storage_api.cpp", "src/storage.cpp", "src/storage_files.cpp", "src/sha256.cpp"], publicHeadersPath: "include"),
        .target(name: "PixAuraCore", dependencies: ["CPixAuraCore"], path: "swift/Sources"),
        .testTarget(name: "PixAuraCoreTests", dependencies: ["PixAuraCore", "CPixAuraCore"], path: "swift/Tests",
                    resources: [.copy("Fixtures")])
    ],
    cxxLanguageStandard: .cxx17
)
