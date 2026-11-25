import Foundation
import Combine
import PixAuraCore

@MainActor
final class ShellModel: ObservableObject {
    @Published private(set) var state = ShellState()
    @Published private(set) var probe = "Checking shared core…"

    func checkCore() async {
        let version = await Task.detached(priority: .userInitiated) {
            CoreProbe.abiVersion()
        }.value
        probe = version == 1 ? "Shared core ready · ABI 1" : "Shared core unavailable"
    }

    func navigate(_ destination: Destination) { state.navigate(destination) }
    func openEditor(_ mode: EditMode) { state.openEditor(mode) }
    func back() { state.back() }
    func restore(destination: String, mode: String) {
        state = ShellState.restore(destination: destination, mode: mode)
    }
}

// Presentation only: derive the scheduled boundary from native quota day, never
// from Date() or the device's current wall clock. Timezone changes grant nothing.
struct CommercialResetPresentationV2 {
    static func local(day: Int64, timeZone: TimeZone = .current, locale: Locale = .current) -> String? {
        guard day >= 0 else { return nil }
        let (next, overflowDay) = day.addingReportingOverflow(1)
        let (seconds, overflowSeconds) = next.multipliedReportingOverflow(by: 86400)
        guard !overflowDay, !overflowSeconds else { return nil }
        let formatter = DateFormatter()
        formatter.locale = locale; formatter.calendar = Calendar(identifier: .gregorian)
        formatter.timeZone = timeZone; formatter.dateFormat = "yyyy-MM-dd HH:mm"
        let value = formatter.string(from: Date(timeIntervalSince1970: Double(seconds)))
        return value.isEmpty ? nil : value+" ("+timeZone.identifier+")"
    }
}

// Phase11 independently executable store components. No ShellModel/UI/runtime
// instantiates these until legitimate product configuration and admission review.
import StoreKit
import Security
import CryptoKit

struct ProtectedCommercialCache {
    let service: String
    func read() throws -> Data? {
        var query = attributes
        query[kSecReturnData as String] = true
        query[kSecMatchLimit as String] = kSecMatchLimitOne
        var result: CFTypeRef?
        let status = SecItemCopyMatching(query as CFDictionary, &result)
        if status == errSecItemNotFound { return nil }
        guard status == errSecSuccess else { throw StoreAccessError.keychain(status) }
        guard let data = result as? Data, data.count <= 16384 else { throw StoreAccessError.unavailable }
        return data
    }
    func write(_ data: Data) throws {
        guard data.count <= 16384 else { throw StoreAccessError.unavailable }
        let status = SecItemUpdate(attributes as CFDictionary, [kSecValueData as String: data] as CFDictionary)
        if status == errSecItemNotFound {
            var item = attributes
            item[kSecValueData as String] = data
            item[kSecAttrAccessible as String] = kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly
            let added = SecItemAdd(item as CFDictionary, nil)
            guard added == errSecSuccess else { throw StoreAccessError.keychain(added) }
        } else if status != errSecSuccess { throw StoreAccessError.keychain(status) }
    }
    func remove() throws {
        let status = SecItemDelete(attributes as CFDictionary)
        guard status == errSecSuccess || status == errSecItemNotFound else { throw StoreAccessError.unavailable }
    }
    private var attributes: [String: Any] {
        [kSecClass as String: kSecClassGenericPassword, kSecAttrService as String: service,
         kSecAttrAccount as String: "commercial-state-v1", kSecAttrSynchronizable as String: false]
    }
}
// App-owned native receipt MAC secret. Separate from all store and Apple signing
// identities. Creation is atomic; an existing value is NEVER replaced or reset.
struct NativeEditorInstallationKeyV2 {
    let service: String
    private var attributes: [String: Any] {
        [kSecClass as String: kSecClassGenericPassword,
         kSecAttrService as String: service,
         kSecAttrAccount as String: "native-receipt-key-v2",
         kSecAttrSynchronizable as String: false]
    }
    private func existing() throws -> Data? {
        var query = attributes; query[kSecReturnData as String] = true
        query[kSecMatchLimit as String] = kSecMatchLimitOne
        var result: CFTypeRef?
        let status = SecItemCopyMatching(query as CFDictionary, &result)
        if status == errSecItemNotFound { return nil }
        guard status == errSecSuccess else { throw StoreAccessError.keychain(status) }
        guard let data = result as? Data, data.count == 32 else { throw StoreAccessError.unavailable }
        return data
    }
    func readOrCreate(allowCreate: Bool = true) throws -> Data {
        guard !service.isEmpty, service.utf8.count <= 200 else { throw StoreAccessError.unavailable }
        if let value = try existing() { return value }
        guard allowCreate else { throw StoreAccessError.unavailable }
        var bytes = Data(count: 32)
        let random = bytes.withUnsafeMutableBytes { SecRandomCopyBytes(kSecRandomDefault, $0.count, $0.baseAddress!) }
        guard random == errSecSuccess else { throw StoreAccessError.keychain(random) }
        defer { bytes.resetBytes(in: 0..<bytes.count) }
        var item = attributes; item[kSecValueData as String] = bytes
        item[kSecAttrAccessible as String] = kSecAttrAccessibleAfterFirstUnlockThisDeviceOnly
        let status = SecItemAdd(item as CFDictionary, nil)
        guard status == errSecSuccess || status == errSecDuplicateItem else { throw StoreAccessError.keychain(status) }
        guard let value = try existing() else { throw StoreAccessError.unavailable }
        return value
    }
}

enum StoreAccessError: Error { case unconfigured, unavailable, unverified, stale; case keychain(OSStatus) }

struct VerifiedStoreHandoffV1: Sendable {
    enum Origin: Sendable { case storeVerified, protectedCache }
    let record: AppleStoreEntitlementAdapter.Record
    let origin: Origin
    fileprivate init(record: AppleStoreEntitlementAdapter.Record, origin: Origin) { self.record = record; self.origin = origin }
}

actor AppleStoreEntitlementAdapter {
    enum Kind: String, Codable, Sendable { case subscription, lifetime }
    struct Record: Codable, Equatable, Sendable {
        let productID: String
        let kind: Kind
        let transactionID: UInt64
        let signedAt: Date
        let expires: Date?
        let revoked: Bool
        let environment: String
    }
    enum PurchaseOutcome { case verified(Record), pending, cancelled }
    private struct Snapshot: Codable { let version: Int; let records: [Record] }
    private let products: [String: Kind]
    private let environment: String
    private let cache: ProtectedCommercialCache
    private let nativeSink: (@Sendable (VerifiedStoreHandoffV1) async throws -> Void)?
    private let signedNativeSinkV2: (@Sendable (SignedNativeStoreHandoffV2) async throws -> Void)?
    private let signedCacheV2: ProtectedAppleSignedCacheV2?
    private let signedNativeSinkV3: (@Sendable (SignedNativeStoreHandoffV2) async throws -> EditorStoreAcknowledgmentV3)?
    private var generation: UInt64 = 0
    init(products: [String: Kind], environment: String, cache: ProtectedCommercialCache,
         nativeSink: (@Sendable (VerifiedStoreHandoffV1) async throws -> Void)? = nil,
         signedCacheV2: ProtectedAppleSignedCacheV2? = nil,
         signedNativeSinkV2: (@Sendable (SignedNativeStoreHandoffV2) async throws -> Void)? = nil,
         signedNativeSinkV3: (@Sendable (SignedNativeStoreHandoffV2) async throws -> EditorStoreAcknowledgmentV3)? = nil) throws {
        guard !products.isEmpty, products.count <= 8,
              products.keys.allSatisfy({ !$0.isEmpty && $0.utf8.count <= 128 }),
              ["Sandbox", "Production"].contains(environment) else { throw StoreAccessError.unconfigured }
        if signedNativeSinkV2 != nil || signedNativeSinkV3 != nil {
            guard let signedCacheV2, signedCacheV2.products == products, signedCacheV2.environment == environment,
                  signedCacheV2.application == Bundle.main.bundleIdentifier else { throw StoreAccessError.unconfigured }
        }
        self.products = products; self.environment = environment; self.cache = cache; self.nativeSink = nativeSink
        self.signedCacheV2 = signedCacheV2; self.signedNativeSinkV2 = signedNativeSinkV2; self.signedNativeSinkV3 = signedNativeSinkV3
    }
    // These are cached previously verified store facts, not a current clock/access
    // grant. Native quota projection must separately reconcile trusted time/revocation.
    func cached() throws -> [Record] {
        guard let bytes = try cache.read() else { return [] }
        guard let snapshot = try? JSONDecoder().decode(Snapshot.self, from: bytes),
              snapshot.version == 1, snapshot.records.count <= 8,
              Set(snapshot.records.map(\.productID)).count == snapshot.records.count,
              snapshot.records.allSatisfy({ products[$0.productID] == $0.kind && $0.environment == environment }) else { throw StoreAccessError.unverified }
        return snapshot.records
    }
    // Restores authenticated app-Keychain facts; native owner still determines
    // trusted time/offline grace/revocation. This is not a platform paid boolean.
    func replayProtectedCacheToNative() async throws {
        guard let nativeSink else { throw StoreAccessError.unconfigured }
        let expected = generation
        for record in try cached() {
            guard generation == expected, !Task.isCancelled else { throw StoreAccessError.stale }
            try await nativeSink(VerifiedStoreHandoffV1(record: record, origin: .protectedCache))
        }
    }
    func replayAuthenticatedSignedCacheV2() async throws {
        guard let signedCacheV2, let signedNativeSinkV2 else { throw StoreAccessError.unconfigured }
        let expected = generation
        let replay = try signedCacheV2.readAvailable()
        for evidence in replay.values {
            guard generation == expected, !Task.isCancelled else { throw StoreAccessError.stale }
            try await signedNativeSinkV2(SignedNativeStoreHandoffV2(evidence, origin: .protectedSignedCache))
        }
        // Deliver independently verified products first, but never conceal a
        // rejected product or reinterpret its absence as authoritative revocation.
        guard replay.rejected.isEmpty else { throw StoreAccessError.unverified }
    }
    func availableProducts() async throws -> [Product] {
        try await Product.products(for: Array(products.keys))
    }
    func refresh() async throws -> [Record] {
        generation &+= 1; let expected = generation
        var delivered: [Record] = []; var firstFailure: Error?
        for id in products.keys.sorted() {
            if let result = await Transaction.latest(for: id) {
                guard generation == expected, !Task.isCancelled else { throw StoreAccessError.stale }
                do { delivered.append(try await accept(result)) }
                catch {
                    guard signedNativeSinkV3 != nil else { throw error }
                    guard generation == expected, !Task.isCancelled else { throw StoreAccessError.stale }
                    if firstFailure == nil { firstFailure = error }
                }
            }
        }
        if let firstFailure { throw firstFailure } // Other independently verified products were still delivered.
        // v3 truth is the native journal; the unrelated legacy cache is not
        // rewritten by v3 and cannot represent its successful restore result.
        return signedNativeSinkV3 != nil ? delivered : try cached()
    }
    func restore() async throws -> [Record] {
        // Explicit operator/user restore only; never called during ordinary editing.
        try await AppStore.sync(); return try await refresh()
    }
    func purchase(_ product: Product) async throws -> PurchaseOutcome {
        guard products[product.id] != nil else { throw StoreAccessError.unconfigured }
        switch try await product.purchase() {
        case .success(let result): return .verified(try await accept(result))
        case .pending: return .pending
        case .userCancelled: return .cancelled
        @unknown default: throw StoreAccessError.unavailable
        }
    }
    func observeUpdates() async throws {
        for await result in Transaction.updates {
            guard !Task.isCancelled else { return }
            if signedNativeSinkV3 != nil {
                guard case .verified(let notification) = result else { throw StoreAccessError.unverified }
                guard products[notification.productID] != nil else { continue }
                // An update may concern an older transaction. Select the SDK's
                // actual most recent transaction for this product, not ID order
                // or a guessed entitlement reconstructed from history.
                if let latest = await Transaction.latest(for: notification.productID) {
                    guard !Task.isCancelled else { return }
                    _ = try await accept(latest)
                } // Absence is not a fabricated authoritative revocation.
            } else { _ = try await accept(result) }
        }
    }
    func stop() { generation &+= 1 }
    private func accept(_ result: VerificationResult<Transaction>) async throws -> Record {
        guard case .verified(let transaction) = result,
              let kind = products[transaction.productID], transaction.ownershipType == .purchased else { throw StoreAccessError.unverified }
        let actualEnvironment: String
        switch transaction.environment {
        case .sandbox: actualEnvironment = "Sandbox"
        case .production: actualEnvironment = "Production"
        default: throw StoreAccessError.unverified // Xcode synthetic config is not authenticated sandbox evidence.
        }
        guard actualEnvironment == environment,
              (kind == .subscription && transaction.productType == .autoRenewable && transaction.expirationDate != nil) ||
              (kind == .lifetime && transaction.productType == .nonConsumable) else { throw StoreAccessError.unverified }
        let record = Record(productID: transaction.productID, kind: kind,
                            transactionID: transaction.id, signedAt: transaction.signedDate,
                            expires: transaction.expirationDate, revoked: transaction.revocationDate != nil,
                            environment: actualEnvironment)
        if let signedNativeSinkV3 {
            guard let signedCacheV2 else { throw StoreAccessError.unconfigured }
            let evidence = try signedCacheV2.verify(result.jwsRepresentation)
            guard evidence.record == record else { throw StoreAccessError.unverified }
            let receipt = try await signedNativeSinkV3(SignedNativeStoreHandoffV2(evidence, origin: .verifiedStoreKit))
            try signedCacheV2.writeAfterNativeCommit(evidence, receipt: receipt)
            await transaction.finish(); return record
        }
        var records = try cached()
        if let previous = records.first(where: { $0.productID == record.productID }), (previous.signedAt > record.signedAt || (previous.signedAt == record.signedAt && previous.transactionID == record.transactionID && previous.revoked && !record.revoked)) { throw StoreAccessError.stale }
        records.removeAll { $0.productID == record.productID }; records.append(record)
        try cache.write(JSONEncoder().encode(Snapshot(version: 1, records: records.sorted { $0.productID < $1.productID })))
        if let signedNativeSinkV2 {
            guard let signedCacheV2 else { throw StoreAccessError.unconfigured }
            let evidence = try signedCacheV2.verify(result.jwsRepresentation)
            guard evidence.record == record else { throw StoreAccessError.unverified }
            try signedCacheV2.write(evidence)
            try await signedNativeSinkV2(SignedNativeStoreHandoffV2(evidence, origin: .verifiedStoreKit))
        }
        if let nativeSink { try await nativeSink(VerifiedStoreHandoffV1(record: record, origin: .storeVerified)) }
        // Finish only after durable cache and configured native handoff. Unverified/pending results never finish/grant.
        await transaction.finish(); return record
    }
}

// Local OS timing only. No boot facts are logged, uploaded or treated as trusted
// absolute time. Native ledger owns rollback/UTC/quota semantics.
import Darwin
struct CommercialClockObservationV1: Equatable, Sendable {
    let utcSeconds: Int64
    let uptimeSeconds: Int64
    let bootToken: String
}
final class AppleCommercialClockSourceV1: Sendable {
    private let bootToken: String
    private let numerator: Double
    private let denominator: Double
    init() throws {
        var boot = timeval(); var length = MemoryLayout<timeval>.size
        guard sysctlbyname("kern.boottime", &boot, &length, nil, 0) == 0,
              length == MemoryLayout<timeval>.size, boot.tv_sec > 0,
              boot.tv_usec >= 0, boot.tv_usec < 1_000_000 else { throw StoreAccessError.unavailable }
        bootToken = String(format: "%016llx%016llx", UInt64(boot.tv_sec), UInt64(boot.tv_usec))
        var timebase = mach_timebase_info_data_t()
        guard mach_timebase_info(&timebase) == KERN_SUCCESS, timebase.numer > 0, timebase.denom > 0 else { throw StoreAccessError.unavailable }
        numerator = Double(timebase.numer); denominator = Double(timebase.denom)
    }
    func sample() throws -> CommercialClockObservationV1 {
        let utc = Date().timeIntervalSince1970
        let uptime = Double(mach_continuous_time()) * numerator / denominator / 1_000_000_000
        guard utc.isFinite, uptime.isFinite, utc >= 0, uptime >= 0,
              utc < Double(Int64.max), uptime < Double(Int64.max) else { throw StoreAccessError.unavailable }
        return CommercialClockObservationV1(utcSeconds: Int64(utc.rounded(.down)),
            uptimeSeconds: Int64(uptime.rounded(.down)), bootToken: bootToken)
    }
}

/// Application resource owner shared by scenes. Eligibility and publication
/// remain native. A detached CGImage retains its native reservation until its
/// data provider retires; closing an editor must not release those bytes early.
final class NativeEditorRuntimePoolV2 {
    static let shared = NativeEditorRuntimePoolV2(storeVerifier: try? AppleNativeStoreVerifierV3.fromConfiguredBundle())
    private let gate = NSRecursiveLock()
    private let configuredRoot: URL?
    private let keyService: String
    private var runtime: EditorRuntimeV2?
    private var leases = 0
    private var retirementError: Error?
    private let storeVerifier: AppleNativeStoreVerifierV3?
    private var storeFailure: Error?
    init(root: URL? = nil, keyService: String = "ai.pixaura.native-receipt-v2", storeVerifier: AppleNativeStoreVerifierV3? = nil) {
        configuredRoot = root; self.keyService = keyService; self.storeVerifier = storeVerifier
    }
    final class Lease {
        let runtime: EditorRuntimeV2
        private let pool: NativeEditorRuntimePoolV2
        private let gate = NSLock()
        private var retired = false
        fileprivate init(_ pool: NativeEditorRuntimePoolV2, _ runtime: EditorRuntimeV2) { self.pool = pool; self.runtime = runtime }
        func close() throws {
            gate.lock(); defer { gate.unlock() }
            if retired { return }; retired = true; try pool.release(runtime)
        }
        deinit { try? close() }
    }
    private func create(_ snapshot: AppleCapabilitySnapshot) throws -> EditorRuntimeV2 {
        let files = FileManager.default
        let base = try configuredRoot ?? files.url(for: .applicationSupportDirectory, in: .userDomainMask, appropriateFor: nil, create: true).appendingPathComponent("native-commercial-v2", isDirectory: true)
        var root = base
        if files.fileExists(atPath: root.path) {
            let facts = try root.resourceValues(forKeys: [.isSymbolicLinkKey, .isDirectoryKey])
            guard facts.isSymbolicLink != true, facts.isDirectory == true else { throw StoreAccessError.unavailable }
        }
        try files.createDirectory(at: root, withIntermediateDirectories: true, attributes: [.posixPermissions: 0o700, .protectionKey: FileProtectionType.completeUntilFirstUserAuthentication])
        var policy = URLResourceValues(); policy.isExcludedFromBackup = true; try root.setResourceValues(policy)
        let marker = root.appendingPathComponent("installation-v2")
        do {
            if files.fileExists(atPath: marker.path) {
                let facts = try marker.resourceValues(forKeys: [.isRegularFileKey, .isSymbolicLinkKey, .fileSizeKey])
                guard facts.isRegularFile == true, facts.isSymbolicLink != true, facts.fileSize == 0 else { throw StoreAccessError.unavailable }
            }
            var key = try NativeEditorInstallationKeyV2(service: keyService).readOrCreate(allowCreate:
                !files.fileExists(atPath: marker.path) && !files.fileExists(atPath: root.appendingPathComponent("commercial-v2.sqlite").path))
            defer { key.resetBytes(in: 0..<key.count) }
            if !files.fileExists(atPath: marker.path) {
                try Data().write(to: marker, options: [.withoutOverwriting, .completeFileProtectionUntilFirstUserAuthentication])
                try files.setAttributes([.posixPermissions: 0o600], ofItemAtPath: marker.path)
            }
            let owner = try EditorRuntimeV2(privateRoot: root, protectedInstallationKey: key, capability: snapshot)
            // Optional store failure must not turn an otherwise healthy native
            // Free/trial owner into key-loss recovery or disable manual editing.
            if let storeVerifier {
                do { try owner.configureStoreV3(storeVerifier); storeFailure = nil }
                catch { storeFailure = error } // Optional store failure leaves Free/trial/recovery intact.
            }
            return owner
        } catch {
            // Missing or malformed committed keys are never replaced. Recovery
            // opens no commerce ledger and grants no new editing or paid access.
            return try EditorRuntimeV2(recoveringPrivateRoot: root, capability: snapshot)
        }
    }
    func acquire(_ snapshot: AppleCapabilitySnapshot) throws -> Lease {
        gate.lock(); defer { gate.unlock() }
        if let retirementError { throw retirementError }
        guard leases < 8 else { throw ManualEditorFailure(status: 8, boundary: "Editor owners") }
        let owner: EditorRuntimeV2
        if let runtime { owner = runtime; _ = try owner.updateCapability(snapshot) }
        else {
            owner = try create(snapshot); runtime = owner
            owner.observeFrameRetirement { [weak self, weak owner] in
                guard let self, let owner else { return }
                self.gate.lock(); defer { self.gate.unlock() }
                do { try self.idle(owner) } catch { self.retirementError = error }
            }
        }
        leases += 1; return Lease(self, owner)
    }
    func configuredStoreAdapter(capability: AppleCapabilityOwner) throws -> AppleStoreEntitlementAdapter {
        guard let storeVerifier else { throw StoreAccessError.unconfigured }
        let cache = storeVerifier.signedCache
        return try AppleStoreEntitlementAdapter(products: cache.products, environment: cache.environment,
            cache: ProtectedCommercialCache(service: cache.application+".store-legacy-v1"), signedCacheV2: cache,
            signedNativeSinkV3: { [self] handoff in
                try await Task.detached(priority: .utility) {
                    try self.deliverStore(handoff, snapshot: capability.snapshot())
                }.value
            })
    }
    // Called on the existing worker lane. A pool lease prevents teardown while
    // the actual verifier/native COMMIT path runs; no second commercial owner.
    func deliverStore(_ handoff: SignedNativeStoreHandoffV2, snapshot: AppleCapabilitySnapshot) throws -> EditorStoreAcknowledgmentV3 {
        guard let storeVerifier else { throw StoreAccessError.unconfigured }
        let lease = try acquire(snapshot)
        do {
            let result = try storeVerifier.deliver(handoff, runtime: lease.runtime)
            try lease.close(); return result
        } catch { try? lease.close(); throw error }
    }
    private func release(_ owner: EditorRuntimeV2) throws {
        gate.lock(); defer { gate.unlock() }
        guard runtime === owner, leases > 0 else { throw ManualEditorFailure(status: 15, boundary: "Editor owners") }
        leases -= 1; try idle(owner)
    }
    private func idle(_ owner: EditorRuntimeV2) throws {
        guard runtime === owner, leases == 0 else { return }
        do { try owner.close(); runtime = nil }
        catch let failure as ManualEditorFailure { if failure.status != 15 { throw failure } }
    }
}

// Local signature verification for protected StoreKit JWS replay. This validates
// historical signed evidence, never authenticates current UTC or revocation now.
struct VerifiedAppleSignedTransactionV2: Sendable {
    let record: AppleStoreEntitlementAdapter.Record
    let transactionDigest: String
    fileprivate let jws: String
    fileprivate let signedMilliseconds: Int64
    fileprivate init(record: AppleStoreEntitlementAdapter.Record, digest: String, jws: String, signedMilliseconds: Int64) {
        self.record = record; transactionDigest = digest; self.jws = jws; self.signedMilliseconds = signedMilliseconds
    }
}
enum AppleStoreJwsV2 {
    // Apple PKI public trust anchor, NOT a development/signing identity.
    // https://www.apple.com/certificateauthority/AppleRootCA-G3.cer
    // SHA256 63343abfb89a6a03ebb57e9b3f5fa7be7c4f5c756f3017b3a8c488c3653e9179
    private static let rootBase64 = "MIICQzCCAcmgAwIBAgIILcX8iNLFS5UwCgYIKoZIzj0EAwMwZzEbMBkGA1UEAwwSQXBwbGUgUm9vdCBDQSAtIEczMSYwJAYDVQQLDB1BcHBsZSBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTETMBEGA1UECgwKQXBwbGUgSW5jLjELMAkGA1UEBhMCVVMwHhcNMTQwNDMwMTgxOTA2WhcNMzkwNDMwMTgxOTA2WjBnMRswGQYDVQQDDBJBcHBsZSBSb290IENBIC0gRzMxJjAkBgNVBAsMHUFwcGxlIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MRMwEQYDVQQKDApBcHBsZSBJbmMuMQswCQYDVQQGEwJVUzB2MBAGByqGSM49AgEGBSuBBAAiA2IABJjpLz1AcqTtkyJygRMc3RCV8cWjTnHcFBbZDuWmBSp3ZHtfTjjTuxxEtX/1H7YyYl3J6YRbTzBPEVoA/VhYDKX1DyxNB0cTddqXl5dvMVztK517IDvYuVTZXpmkOlEKMaNCMEAwHQYDVR0OBBYEFLuw3qFYM4iapIqZ3r6966/ayySrMA8GA1UdEwEB/wQFMAMBAf8wDgYDVR0PAQH/BAQDAgEGMAoGCCqGSM49BAMDA2gAMGUCMQCD6cHEFl4aXTQY2e3v9GwOAEZLuN+yRhHFD/3meoyhpmvOwgPUnPWTxnS4at+qIxUCMG1mihDK1A3UT82NQz60imOlM27jbdoXt2QfyFMm+YhidDkLF1vLUagM6BgD56KyKA=="
    private static let leafOID: [UInt8] = [0x2a,0x86,0x48,0x86,0xf7,0x63,0x64,0x06,0x0b,0x01]
    private static let issuerOID: [UInt8] = [0x2a,0x86,0x48,0x86,0xf7,0x63,0x64,0x06,0x02,0x01]
    private static func require(_ condition: Bool) throws { if !condition { throw StoreAccessError.unverified } }
    private struct TLV { let tag: UInt8; let content: Range<Int> }
    private static func tlv(_ bytes: [UInt8], _ at: inout Int, _ end: Int) throws -> TLV {
        try require(end <= bytes.count && at >= 0 && at < end)
        let tag = bytes[at]; at += 1; try require(tag & 31 != 31 && at < end)
        let first = Int(bytes[at]); at += 1; var count = first
        if first & 128 != 0 {
            let n = first & 127; try require(n > 0 && n <= 3 && n <= end-at && bytes[at] != 0)
            count = 0; for _ in 0..<n { count = count*256 + Int(bytes[at]); at += 1 }
            try require(count >= 128)
        }
        try require(count <= end-at); let range = at..<(at+count); at += count
        return TLV(tag: tag, content: range)
    }
    // Bounded DER extension lookup; SecTrust separately validates certificate DER,
    // signatures, path constraints and validity. No byte-pattern OID search.
    static func hasExtension(_ certificate: Data, oid: [UInt8]) throws -> Bool {
        try require(!certificate.isEmpty && certificate.count <= 16384)
        let b = [UInt8](certificate); var at = 0; let outer = try tlv(b, &at, b.count)
        try require(outer.tag == 0x30 && at == b.count)
        var t = outer.content.lowerBound; let tbs = try tlv(b, &t, outer.content.upperBound); try require(tbs.tag == 0x30)
        var p = tbs.content.lowerBound; var extensions: Range<Int>?
        while p < tbs.content.upperBound {
            let child = try tlv(b, &p, tbs.content.upperBound)
            if child.tag == 0xa3 { try require(extensions == nil); extensions = child.content }
        }
        guard let extensions else { return false }
        var e = extensions.lowerBound; let sequence = try tlv(b, &e, extensions.upperBound)
        try require(sequence.tag == 0x30 && e == extensions.upperBound)
        var next = sequence.content.lowerBound; var seen = Set<Data>(); var found = false
        while next < sequence.content.upperBound {
            let item = try tlv(b, &next, sequence.content.upperBound); try require(item.tag == 0x30)
            var field = item.content.lowerBound; let name = try tlv(b, &field, item.content.upperBound)
            try require(name.tag == 6 && !name.content.isEmpty)
            let value = Data(b[name.content]); try require(seen.insert(value).inserted)
            var data = try tlv(b, &field, item.content.upperBound)
            if data.tag == 1 { try require(data.content.count == 1 && [UInt8(0),255].contains(b[data.content.lowerBound])); data = try tlv(b, &field, item.content.upperBound) }
            try require(data.tag == 4 && field == item.content.upperBound)
            if value == Data(oid) { found = true }
        }
        return found
    }
    private static func segment(_ value: Substring, maximum: Int) throws -> Data {
        try require(!value.isEmpty && value.utf8.count <= maximum && value.utf8.allSatisfy { (65...90).contains($0) || (97...122).contains($0) || (48...57).contains($0) || $0 == 45 || $0 == 95 })
        var padded = value.replacingOccurrences(of: "-", with: "+").replacingOccurrences(of: "_", with: "/")
        padded += String(repeating: "=", count: (4-padded.count%4)%4)
        guard let bytes = Data(base64Encoded: padded), bytes.base64EncodedString().replacingOccurrences(of: "+", with: "-").replacingOccurrences(of: "/", with: "_").replacingOccurrences(of: "=", with: "") == value else { throw StoreAccessError.unverified }
        return bytes
    }
    static func validSignature(publicKey: Data, message: Data, signature: Data) throws -> Bool {
        guard publicKey.count == 65, signature.count == 64 else { throw StoreAccessError.unverified }
        let key = try P256.Signing.PublicKey(x963Representation: publicKey)
        return key.isValidSignature(try P256.Signing.ECDSASignature(rawRepresentation: signature), for: message)
    }
    static func verify(_ jws: String, products: [String: AppleStoreEntitlementAdapter.Kind], environment: String, application: String) throws -> VerifiedAppleSignedTransactionV2 {
        try require(jws.utf8.count <= 12288 && !products.isEmpty && products.count <= 8 && !application.isEmpty && application.utf8.count <= 255 && ["Sandbox","Production"].contains(environment))
        let parts = jws.split(separator: ".", omittingEmptySubsequences: false); try require(parts.count == 3)
        let header = try segment(parts[0], maximum: 8192), payload = try segment(parts[1], maximum: 8192), signature = try segment(parts[2], maximum: 128)
        guard let h = try JSONSerialization.jsonObject(with: header) as? [String: Any], h["alg"] as? String == "ES256", h["crit"] == nil,
              let chain = h["x5c"] as? [String], chain.count == 3,
              let claims = try JSONSerialization.jsonObject(with: payload) as? [String: Any] else { throw StoreAccessError.unverified }
        func milliseconds(_ name: String) throws -> Int64 {
            guard let number = claims[name] as? NSNumber, CFGetTypeID(number) != CFBooleanGetTypeID() else { throw StoreAccessError.unverified }
            let value = number.doubleValue; try require(value.isFinite && value > 0 && value <= 8640000000000000 && value.rounded(.down) == value)
            return Int64(value)
        }
        let signed = try milliseconds("signedDate")
        var certificates: [SecCertificate] = []; var encoded: [Data] = []
        for entry in chain {
            try require(entry.utf8.count <= 8192)
            guard let der = Data(base64Encoded: entry), der.base64EncodedString() == entry,
                  let certificate = SecCertificateCreateWithData(nil, der as CFData) else { throw StoreAccessError.unverified }
            encoded.append(der); certificates.append(certificate)
        }
        guard let root = Data(base64Encoded: rootBase64) else { throw StoreAccessError.unverified }
        try require(encoded[2] == root && hasExtension(encoded[0], oid: leafOID) && hasExtension(encoded[1], oid: issuerOID))
        var trust: SecTrust?
        try require(SecTrustCreateWithCertificates(certificates as CFArray, SecPolicyCreateBasicX509(), &trust) == errSecSuccess)
        guard let trust else { throw StoreAccessError.unverified }
        try require(SecTrustSetAnchorCertificates(trust, [certificates[2]] as CFArray) == errSecSuccess)
        try require(SecTrustSetAnchorCertificatesOnly(trust, true) == errSecSuccess && SecTrustSetNetworkFetchAllowed(trust, false) == errSecSuccess)
        try require(SecTrustSetVerifyDate(trust, Date(timeIntervalSince1970: Double(signed)/1000) as CFDate) == errSecSuccess && SecTrustEvaluateWithError(trust, nil))
        guard let key = SecTrustCopyKey(trust), let rawKey = SecKeyCopyExternalRepresentation(key, nil) as Data? else { throw StoreAccessError.unverified }
        let signedBytes = Data((String(parts[0])+"."+String(parts[1])).utf8)
        try require(validSignature(publicKey: rawKey, message: signedBytes, signature: signature))
        guard claims["bundleId"] as? String == application, claims["environment"] as? String == environment,
              claims["inAppOwnershipType"] as? String == "PURCHASED", let product = claims["productId"] as? String, let kind = products[product],
              let transaction = claims["transactionId"] as? String, let id = UInt64(transaction), id > 0, String(id) == transaction,
              let nonceText = claims["deviceVerificationNonce"] as? String, let nonce = UUID(uuidString: nonceText),
              let current = AppStore.deviceVerificationID, let deviceText = claims["deviceVerification"] as? String,
              let device = Data(base64Encoded: deviceText) else { throw StoreAccessError.unverified }
        let expected = Data(SHA384.hash(data: Data((nonce.uuidString.lowercased()+current.uuidString.lowercased()).utf8)))
        try require(device.count == expected.count)
        var difference: UInt8 = 0; for (a,b) in zip(device,expected) { difference |= a ^ b }; try require(difference == 0)
        let expires: Date?
        if kind == .subscription { try require(claims["type"] as? String == "Auto-Renewable Subscription"); expires = Date(timeIntervalSince1970: Double(try milliseconds("expiresDate"))/1000) }
        else { try require(claims["type"] as? String == "Non-Consumable" && (claims["expiresDate"] == nil || claims["expiresDate"] is NSNull)); expires = nil }
        let revoked = claims["revocationDate"] != nil && !(claims["revocationDate"] is NSNull); if revoked { _ = try milliseconds("revocationDate") }
        let record = AppleStoreEntitlementAdapter.Record(productID: product, kind: kind, transactionID: id, signedAt: Date(timeIntervalSince1970: Double(signed)/1000), expires: expires, revoked: revoked, environment: environment)
        return VerifiedAppleSignedTransactionV2(record: record, digest: SHA256.hash(data: Data(jws.utf8)).map { String(format: "%02x", $0) }.joined(), jws: jws, signedMilliseconds: signed)
    }
}

struct SignedNativeStoreHandoffV2: Sendable {
    enum Origin: Sendable { case verifiedStoreKit, protectedSignedCache }
    let evidence: VerifiedAppleSignedTransactionV2
    let origin: Origin
    fileprivate init(_ evidence: VerifiedAppleSignedTransactionV2, origin: Origin) { self.evidence = evidence; self.origin = origin }
}
// Optional production adapter: instantiated only with legitimate app/catalog
// configuration. It owns no runtime (avoids a native-verifier/runtime cycle).
// The private SDK handoff is required in addition to pinned JWS verification.
final class AppleNativeStoreVerifierV3: EditorStoreVerifierV3, @unchecked Sendable {
    let configuration: EditorStoreConfigurationV3
    private let cache: ProtectedAppleSignedCacheV2
    fileprivate var signedCache: ProtectedAppleSignedCacheV2 { cache }
    private let gate = NSLock()
    private var fresh: (nonce: String, digest: String)?
    init(cache: ProtectedAppleSignedCacheV2) throws {
        guard cache.application == Bundle.main.bundleIdentifier, !cache.products.isEmpty,
              cache.products.count <= 8, ["Sandbox", "Production"].contains(cache.environment) else { throw StoreAccessError.unconfigured }
        self.cache = cache
        configuration = EditorStoreConfigurationV3(application: cache.application,
            keyIdentity: "63343abfb89a6a03ebb57e9b3f5fa7be7c4f5c756f3017b3a8c488c3653e9179",
            store: 1, environment: cache.environment == "Sandbox" ? 1 : 2,
            products: cache.products.mapValues { $0 == .subscription ? 1 : 2 })
    }
    static func fromConfiguredBundle() throws -> AppleNativeStoreVerifierV3? {
        guard let raw = Bundle.main.object(forInfoDictionaryKey: "PixAuraVerifiedStoreV3") else { return nil }
        guard let values = raw as? [String: Any], Set(values.keys) == ["version", "environment", "products"],
              let version = values["version"] as? Int, version == 3,
              let environment = values["environment"] as? String, let configured = values["products"] as? [String: String],
              !configured.isEmpty, configured.count <= 8, let application = Bundle.main.bundleIdentifier else { throw StoreAccessError.unconfigured }
        var products: [String: AppleStoreEntitlementAdapter.Kind] = [:]
        for (id, kind) in configured {
            guard !id.isEmpty, id.utf8.count <= 128 else { throw StoreAccessError.unconfigured }
            switch kind { case "subscription": products[id] = .subscription
            case "lifetime": products[id] = .lifetime
            default: throw StoreAccessError.unconfigured }
        }
        return try AppleNativeStoreVerifierV3(cache: ProtectedAppleSignedCacheV2(service: application+".store-signed-v3",
            application: application, products: products, environment: environment))
    }
    func verifySignedProof(_ proof: Data, origin: EditorStoreProofOriginV3, nativeNonce: String) throws -> EditorStoreVerifiedMetadataV3 {
        guard !proof.isEmpty, proof.count <= 16384, let jws = String(data: proof, encoding: .utf8) else { throw StoreAccessError.unverified }
        let digest = SHA256.hash(data: proof).map { String(format: "%02x", $0) }.joined()
        if origin == .fresh {
            gate.lock()
            let permitted = fresh?.nonce == nativeNonce && fresh?.digest == digest
            if permitted { fresh = nil }
            gate.unlock()
            guard permitted else { throw StoreAccessError.unverified }
        } else if !nativeNonce.isEmpty { throw StoreAccessError.unverified }
        let evidence = try cache.verify(jws), record = evidence.record
        // Stable actual transaction identity, distinct from a reissued JWS hash.
        let identity = "PIXAURA_APPLE_TRANSACTION_V3\n"+configuration.application+"\n"+record.environment+"\n"+String(record.transactionID)
        let transaction = SHA256.hash(data: Data(identity.utf8)).map { String(format: "%02x", $0) }.joined()
        return EditorStoreVerifiedMetadataV3(product: record.productID, transactionDigest: transaction,
            verifiedKeyIdentity: configuration.keyIdentity, kind: record.kind == .subscription ? 1 : 2,
            status: record.revoked ? 3 : 1, signedSeconds: evidence.signedMilliseconds / 1000,
            signedMillisecond: UInt32(evidence.signedMilliseconds % 1000),
            expires: record.expires.map { Int64($0.timeIntervalSince1970.rounded(.down)) } ?? 0)
    }
    func deliver(_ handoff: SignedNativeStoreHandoffV2, runtime: EditorRuntimeV2) throws -> EditorStoreAcknowledgmentV3 {
        guard case .verifiedStoreKit = handoff.origin else { throw StoreAccessError.unverified }
        let nonce = try runtime.beginStoreRequestV3()
        gate.lock()
        guard fresh == nil else { gate.unlock(); try? runtime.cancelStoreRequestV3(nonce); throw StoreAccessError.stale }
        fresh = (nonce, handoff.evidence.transactionDigest); gate.unlock()
        defer {
            gate.lock(); if fresh?.nonce == nonce { fresh = nil }; gate.unlock()
            try? runtime.cancelStoreRequestV3(nonce)
        }
        return try runtime.acceptStoreProofV3(Data(handoff.evidence.jws.utf8), nativeNonce: nonce)
    }
}
// Each allowlisted product uses one existing <=16KiB protected item; no cap on
// the v1 cache is enlarged. At most eight items, only signed store metadata.
struct ProtectedAppleSignedCacheV2: Sendable {
    private static let gate = NSLock()
    let service: String
    let application: String
    let products: [String: AppleStoreEntitlementAdapter.Kind]
    let environment: String
    private struct Entry: Codable { let version: Int; let jws: String }
    func serviceName(for product: String) -> String {
        let bytes = Data((application+"\n"+environment+"\n"+product).utf8)
        return service+".signed-store-v2."+SHA256.hash(data: bytes).map { String(format: "%02x", $0) }.joined()
    }
    func verify(_ jws: String) throws -> VerifiedAppleSignedTransactionV2 {
        try AppleStoreJwsV2.verify(jws, products: products, environment: environment, application: application)
    }
    static func permitsReplacement(_ old: AppleStoreEntitlementAdapter.Record, _ next: AppleStoreEntitlementAdapter.Record) -> Bool {
        guard old.productID == next.productID, old.kind == next.kind, old.environment == next.environment,
              next.signedAt >= old.signedAt else { return false }
        if next.signedAt == old.signedAt {
            if old.revoked && !next.revoked { return false }
            // Same-generation contradictions are not evidence of renewal.
            if !next.revoked && old != next { return false }
        }
        return true
    }
    func write(_ evidence: VerifiedAppleSignedTransactionV2) throws {
        let checked = try verify(evidence.jws)
        Self.gate.lock(); defer { Self.gate.unlock() }
        let item = ProtectedCommercialCache(service: serviceName(for: checked.record.productID))
        if let bytes = try item.read() {
            guard let entry = try? JSONDecoder().decode(Entry.self, from: bytes), entry.version == 2 else { throw StoreAccessError.unverified }
            let prior = try verify(entry.jws)
            guard Self.permitsReplacement(prior.record, checked.record) else { throw StoreAccessError.stale }
        }
        try item.write(JSONEncoder().encode(Entry(version: 2, jws: checked.jws)))
    }
    // Repair ONLY the acknowledged product after native COMMIT and actual JWS
    // verification. Other products and their independently valid caches survive.
    func writeAfterNativeCommit(_ evidence: VerifiedAppleSignedTransactionV2, receipt: EditorStoreAcknowledgmentV3) throws {
        let checked = try verify(evidence.jws)
        guard receipt.product == checked.record.productID, receipt.identity.utf8.count == 64,
              receipt.proofDigest == checked.transactionDigest else { throw StoreAccessError.unverified }
        Self.gate.lock(); defer { Self.gate.unlock() }
        let item = ProtectedCommercialCache(service: serviceName(for: checked.record.productID))
        if let bytes = try item.read(), let entry = try? JSONDecoder().decode(Entry.self, from: bytes), entry.version == 2,
           let prior = try? verify(entry.jws) {
            guard Self.permitsReplacement(prior.record, checked.record) else { throw StoreAccessError.stale }
        }
        // A malformed/invalid existing item can be replaced only here, after the
        // typed native receipt is bound to this exact newly verified signed proof.
        try item.write(JSONEncoder().encode(Entry(version: 2, jws: checked.jws)))
    }
    // Product isolation is not authentication: load must still verify every JWS.
    // Rejected entries remain untouched and are reported to the caller.
    static func collectIndependently<Value>(products: [String], load: (String) throws -> Value?) -> (values: [Value], rejected: [String]) {
        var values: [Value] = []; var rejected: [String] = []
        for product in products.sorted() {
            do { if let value = try load(product) { values.append(value) } }
            catch { rejected.append(product) }
        }
        return (values, rejected)
    }
    func readAvailable() throws -> (values: [VerifiedAppleSignedTransactionV2], rejected: [String]) {
        guard !products.isEmpty, products.count <= 8, !application.isEmpty,
              ["Sandbox","Production"].contains(environment) else { throw StoreAccessError.unconfigured }
        return Self.collectIndependently(products: Array(products.keys)) { product in
            guard let bytes = try ProtectedCommercialCache(service: serviceName(for: product)).read() else { return nil }
            guard let entry = try? JSONDecoder().decode(Entry.self, from: bytes), entry.version == 2 else { throw StoreAccessError.unverified }
            let result = try verify(entry.jws)
            guard result.record.productID == product else { throw StoreAccessError.unverified }
            return result
        }
    }
    func read() throws -> [VerifiedAppleSignedTransactionV2] {
        let result = try readAvailable()
        guard result.rejected.isEmpty else { throw StoreAccessError.unverified }
        return result.values
    }
}
