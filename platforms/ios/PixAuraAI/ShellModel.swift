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

// Phase11 independently executable store components. No ShellModel/UI/runtime
// instantiates these until legitimate product configuration and admission review.
import StoreKit
import Security

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
    private var generation: UInt64 = 0
    init(products: [String: Kind], environment: String, cache: ProtectedCommercialCache,
         nativeSink: (@Sendable (VerifiedStoreHandoffV1) async throws -> Void)? = nil) throws {
        guard !products.isEmpty, products.count <= 8,
              products.keys.allSatisfy({ !$0.isEmpty && $0.utf8.count <= 128 }),
              ["Sandbox", "Production"].contains(environment) else { throw StoreAccessError.unconfigured }
        self.products = products; self.environment = environment; self.cache = cache; self.nativeSink = nativeSink
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
    func availableProducts() async throws -> [Product] {
        try await Product.products(for: Array(products.keys))
    }
    func refresh() async throws -> [Record] {
        generation &+= 1; let expected = generation
        for id in products.keys.sorted() {
            if let result = await Transaction.latest(for: id) {
                guard generation == expected, !Task.isCancelled else { throw StoreAccessError.stale }
                _ = try await accept(result)
            }
        }
        return try cached()
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
            _ = try await accept(result)
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
        var records = try cached()
        if let previous = records.first(where: { $0.productID == record.productID }), (previous.signedAt > record.signedAt || (previous.signedAt == record.signedAt && previous.transactionID == record.transactionID && previous.revoked && !record.revoked)) { throw StoreAccessError.stale }
        records.removeAll { $0.productID == record.productID }; records.append(record)
        try cache.write(JSONEncoder().encode(Snapshot(version: 1, records: records.sorted { $0.productID < $1.productID })))
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
