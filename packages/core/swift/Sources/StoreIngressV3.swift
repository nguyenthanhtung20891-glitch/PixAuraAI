import Foundation
import CPixAuraCore

/// Application configuration only, never a decoded purchase/cache/paid flag.
public struct EditorStoreConfigurationV3: Sendable {
    public let application: String, keyIdentity: String
    public let store: UInt32, environment: UInt32
    public let products: [String: UInt32]
    public init(application: String, keyIdentity: String, store: UInt32,
                environment: UInt32, products: [String: UInt32]) {
        self.application = application; self.keyIdentity = keyIdentity
        self.store = store; self.environment = environment; self.products = products
    }
}
public enum EditorStoreProofOriginV3: UInt32, Sendable { case fresh = 1, cached = 2 }
/// Metadata returned by the configured cryptographic adapter. This value alone
/// grants nothing; native scope, challenge, ordering and COMMIT remain required.
public struct EditorStoreVerifiedMetadataV3: Sendable {
    public let product: String, transactionDigest: String, verifiedKeyIdentity: String
    public let kind: UInt32, status: UInt32, signedMillisecond: UInt32
    public let signedSeconds: Int64, expires: Int64
    public init(product: String, transactionDigest: String, verifiedKeyIdentity: String,
                kind: UInt32, status: UInt32, signedSeconds: Int64,
                signedMillisecond: UInt32, expires: Int64) {
        self.product = product; self.transactionDigest = transactionDigest
        self.verifiedKeyIdentity = verifiedKeyIdentity; self.kind = kind; self.status = status
        self.signedSeconds = signedSeconds; self.signedMillisecond = signedMillisecond; self.expires = expires
    }
}
/// Must perform actual pinned store signature verification and application/device
/// binding. Fresh Apple verification additionally requires the private actual
/// StoreKit SDK delivery fence. Cached replay must never fabricate that fence.
/// Called synchronously on workers; do not reenter native APIs or dispatch/wait.
public protocol EditorStoreVerifierV3: AnyObject {
    var configuration: EditorStoreConfigurationV3 { get }
    func verifySignedProof(_ proof: Data, origin: EditorStoreProofOriginV3,
                           nativeNonce: String) throws -> EditorStoreVerifiedMetadataV3
}
public struct EditorStoreAcknowledgmentV3: Sendable {
    public let product: String, identity: String, proofDigest: String
    public let replay: Bool
    fileprivate init(_ value: pixaura_store_v3_acknowledgment) {
        product = storeStringV3(value.product); identity = storeStringV3(value.identity)
        proofDigest = storeStringV3(value.proof_digest); replay = value.replay != 0
    }
}
private func storeStringV3<T>(_ value: T) -> String {
    withUnsafeBytes(of: value) { String(decoding: $0.prefix(while: { $0 != 0 }), as: UTF8.self) }
}
private func storeWriteV3<T>(_ value: String, into field: inout T) throws {
    let bytes = Array(value.utf8)
    guard !bytes.isEmpty, !bytes.contains(0), bytes.count < MemoryLayout<T>.size else {
        throw ManualEditorFailure(status: 6, boundary: "Store configuration")
    }
    withUnsafeMutableBytes(of: &field) { target in
        target.initializeMemory(as: UInt8.self, repeating: 0)
        target.copyBytes(from: bytes)
    }
}
private final class StoreVerifierOwnerV3 {
    let verifier: any EditorStoreVerifierV3
    let configuration: EditorStoreConfigurationV3
    init(_ verifier: any EditorStoreVerifierV3) {
        self.verifier = verifier; configuration = verifier.configuration
    }
}
extension EditorRuntimeV2 {
    /// Bind once using an actual application-owned verifier. Missing legitimate
    /// store configuration leaves this unused and checkout disabled.
    public func configureStoreV3(_ verifier: any EditorStoreVerifierV3) throws {
        let box = StoreVerifierOwnerV3(verifier)
        let configuration = box.configuration
        guard !configuration.products.isEmpty, configuration.products.count <= 8 else {
            throw ManualEditorFailure(status: 6, boundary: "Store configuration")
        }
        var native = pixaura_store_v3_configuration()
        native.version = 3; native.struct_size = UInt32(MemoryLayout.size(ofValue: native))
        native.store = configuration.store; native.environment = configuration.environment
        native.product_count = UInt32(configuration.products.count)
        try storeWriteV3(configuration.application, into: &native.application)
        try storeWriteV3(configuration.keyIdentity, into: &native.key_identity)
        try withUnsafeMutableBytes(of: &native.products) { raw in
            let entries = raw.bindMemory(to: pixaura_store_v3_product.self)
            for (index, item) in configuration.products.sorted(by: { $0.key < $1.key }).enumerated() {
                try storeWriteV3(item.key, into: &entries[index].identifier); entries[index].kind = item.value
            }
        }
        native.context = Unmanaged.passUnretained(box).toOpaque()
        native.retain = { context in
            guard let context else { return 6 }
            _ = Unmanaged<StoreVerifierOwnerV3>.fromOpaque(context).retain(); return 0
        }
        native.release = { context in
            if let context { Unmanaged<StoreVerifierOwnerV3>.fromOpaque(context).release() }
        }
        native.verify = { context, bytes, count, origin, nonce, nonceCount, output in
            guard let context, let bytes, count > 0, count <= 16384, let output,
                  let origin = EditorStoreProofOriginV3(rawValue: origin), nonceCount <= 32,
                  nonceCount == 0 || nonce != nil else { return 6 }
            do {
                let box = Unmanaged<StoreVerifierOwnerV3>.fromOpaque(context).takeUnretainedValue()
                let challenge = nonceCount == 0 ? "" : String(decoding: UnsafeBufferPointer(start: UnsafeRawPointer(nonce!).assumingMemoryBound(to: UInt8.self), count: Int(nonceCount)), as: UTF8.self)
                let result = try box.verifier.verifySignedProof(Data(bytes: bytes, count: Int(count)), origin: origin, nativeNonce: challenge)
                var value = pixaura_store_v3_verified()
                value.version = 3; value.struct_size = UInt32(MemoryLayout.size(ofValue: value))
                value.kind = result.kind; value.status = result.status
                value.signed_seconds = result.signedSeconds; value.signed_millisecond = result.signedMillisecond
                value.expires = result.expires
                try storeWriteV3(result.product, into: &value.product)
                try storeWriteV3(result.transactionDigest, into: &value.transaction_digest)
                try storeWriteV3(result.verifiedKeyIdentity, into: &value.verified_key_identity)
                output.pointee = value; return 0
            } catch { return 6 }
        }
        try storeCheckV3(pixaura_store_v3_bind(3, storeRuntimeHandleV3, &native))
        withExtendedLifetime(box) {}
        storeDidCommitV3()
    }
    public func beginStoreRequestV3() throws -> String {
        var nonce = [CChar](repeating: 0, count: 33)
        try storeCheckV3(pixaura_store_v3_begin(3, storeRuntimeHandleV3, &nonce, UInt64(nonce.count)))
        return String(decoding: nonce.prefix(32).map { UInt8(bitPattern: $0) }, as: UTF8.self)
    }
    public func cancelStoreRequestV3(_ nonce: String) throws {
        try nonce.withCString { try storeCheckV3(pixaura_store_v3_cancel(3, storeRuntimeHandleV3, $0, UInt64(nonce.utf8.count))) }
    }
    public func acceptStoreProofV3(_ proof: Data, nativeNonce: String) throws -> EditorStoreAcknowledgmentV3 {
        var receipt = pixaura_store_v3_acknowledgment()
        try proof.withUnsafeBytes { bytes in
            try nativeNonce.withCString { nonce in
                try storeCheckV3(pixaura_store_v3_accept(3, storeRuntimeHandleV3, bytes.bindMemory(to: UInt8.self).baseAddress,
                    UInt64(bytes.count), nonce, UInt64(nativeNonce.utf8.count), &receipt))
            }
        }
        storeDidCommitV3()
        return EditorStoreAcknowledgmentV3(receipt)
    }
}
private func storeCheckV3(_ status: Int32) throws {
    if status != 0 { throw ManualEditorFailure(status: status, boundary: "Verified store ingress") }
}
