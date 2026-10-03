// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "PixAuraCore",
    platforms: [.iOS(.v16), .macOS(.v13)],
    products: [.library(name: "PixAuraCore", targets: ["PixAuraCore"])],
    targets: [
        .target(name: "CPixAuraCore", path: ".",
                exclude: ["Package.swift", "swift", "tests"],
                sources: ["src/core.cpp", "src/document.cpp", "src/document_api.cpp"], publicHeadersPath: "include"),
        .target(name: "PixAuraCore", dependencies: ["CPixAuraCore"], path: "swift/Sources"),
        .testTarget(name: "PixAuraCoreTests", dependencies: ["PixAuraCore", "CPixAuraCore"], path: "swift/Tests",
                    resources: [.copy("Fixtures")])
    ],
    cxxLanguageStandard: .cxx17
)
