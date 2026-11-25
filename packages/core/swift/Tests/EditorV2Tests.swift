import XCTest
import Foundation
import CoreGraphics
import ImageIO
import CryptoKit
@testable import PixAuraCore

// Real P-256 synthetic protocol fixture, NOT an Apple/Google transaction or
// genuine sandbox evidence. The shipping Apple adapter pins Apple's root instead.
private final class SyntheticStoreVerifierV3: EditorStoreVerifierV3 {
    let key: P256.Signing.PrivateKey
    let configuration: EditorStoreConfigurationV3
    init(key: P256.Signing.PrivateKey) {
        self.key = key
        configuration = EditorStoreConfigurationV3(application: "test.synthetic.application",
            keyIdentity: SHA256.hash(data: key.publicKey.derRepresentation).map { String(format: "%02x", $0) }.joined(),
            store: 1, environment: 1, products: ["test.synthetic.lifetime": 2])
    }
    func sign(nonce: String, status: UInt32, stamp: Int64) throws -> Data {
        let body = Data("\(nonce)\n\(status)\n\(stamp)\n".utf8)
        return body + (try key.signature(for: body).rawRepresentation)
    }
    func verifySignedProof(_ proof: Data, origin: EditorStoreProofOriginV3, nativeNonce: String) throws -> EditorStoreVerifiedMetadataV3 {
        guard proof.count > 64 else { throw ManualEditorFailure(status: 6, boundary: "Synthetic signature") }
        let body = proof.dropLast(64), signature = try P256.Signing.ECDSASignature(rawRepresentation: proof.suffix(64))
        guard key.publicKey.isValidSignature(signature, for: body), let text = String(data: body, encoding: .utf8) else { throw ManualEditorFailure(status: 6, boundary: "Synthetic signature") }
        let fields = text.components(separatedBy: "\n")
        guard fields.count == 4, fields[3].isEmpty, let status = UInt32(fields[1]), let stamp = Int64(fields[2]),
              origin != .fresh || fields[0] == nativeNonce else { throw ManualEditorFailure(status: 6, boundary: "Synthetic signature") }
        return EditorStoreVerifiedMetadataV3(product: "test.synthetic.lifetime", transactionDigest: String(repeating: "b", count: 64),
            verifiedKeyIdentity: configuration.keyIdentity, kind: 2, status: status, signedSeconds: stamp, signedMillisecond: 0, expires: 0)
    }
}

private final class StoreChangeCounterV3: @unchecked Sendable {
    private let gate = NSLock(); private var value = 0
    func received() { gate.lock(); value += 1; gate.unlock() }
    var count: Int { gate.lock(); defer { gate.unlock() }; return value }
}
private final class WeakStoreVerifierV3 {
    weak var value: SyntheticStoreVerifierV3?
    init(_ value: SyntheticStoreVerifierV3?) { self.value = value }
}

final class EditorV2Tests: XCTestCase {
    func testStoreIngressV3RetainsVerifierAndRequiresSignedFreshNonceBeforeDurableProjection() throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("store-ingress-v3-\(UUID())")
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: root) }
        let key = P256.Signing.PrivateKey()
        var verifier: SyntheticStoreVerifierV3? = SyntheticStoreVerifierV3(key: key)
        let retained = WeakStoreVerifierV3(verifier)
        let runtime = try EditorRuntimeV2(privateRoot: root, protectedInstallationKey: Data(1...32))
        defer { try? runtime.close() }
        try runtime.configureStoreV3(try XCTUnwrap(verifier)); verifier = nil
        XCTAssertNotNil(retained.value); XCTAssertEqual(try runtime.commercialState().access, 0)
        let changes = StoreChangeCounterV3()
        _ = try runtime.observeStoreChangesV3 { changes.received() }
        let extra = try (0..<7).map { _ in try runtime.observeStoreChangesV3 {} }
        XCTAssertThrowsError(try runtime.observeStoreChangesV3 {})
        extra.forEach { runtime.removeStoreObserverV3($0) }
        let failed = try runtime.beginStoreRequestV3()
        XCTAssertThrowsError(try runtime.acceptStoreProofV3(Data("unsigned".utf8), nativeNonce: failed))
        XCTAssertThrowsError(try runtime.acceptStoreProofV3(Data("unsigned".utf8), nativeNonce: failed))
        XCTAssertEqual(changes.count, 0)
        let nonce = try runtime.beginStoreRequestV3()
        let proof = try XCTUnwrap(retained.value).sign(nonce: nonce, status: 1, stamp: 1000)
        let receipt = try runtime.acceptStoreProofV3(proof, nativeNonce: nonce)
        XCTAssertEqual(changes.count, 1)
        XCTAssertEqual(receipt.product, "test.synthetic.lifetime"); XCTAssertEqual(receipt.identity.utf8.count, 64)
        XCTAssertEqual(receipt.proofDigest, SHA256.hash(data: proof).map { String(format: "%02x", $0) }.joined())
        XCTAssertEqual(try runtime.commercialState().access, 3); XCTAssertEqual(try runtime.commercialState().trialStart, -1)
        let badNonce = try runtime.beginStoreRequestV3()
        var tampered = proof; tampered[0] ^= 1
        XCTAssertThrowsError(try runtime.acceptStoreProofV3(tampered, nativeNonce: badNonce))
        XCTAssertEqual(changes.count, 1)
        XCTAssertEqual(try runtime.commercialState().access, 3) // unrelated verification failure cannot erase valid lifetime
        let revokeNonce = try runtime.beginStoreRequestV3()
        let revoke = try XCTUnwrap(retained.value).sign(nonce: revokeNonce, status: 3, stamp: 1100)
        _ = try runtime.acceptStoreProofV3(revoke, nativeNonce: revokeNonce)
        XCTAssertEqual(try runtime.commercialState().access, 0); XCTAssertEqual(try runtime.commercialState().trialStart, -1)
        XCTAssertEqual(changes.count, 2)
        try runtime.close(); XCTAssertNil(retained.value)
        let reopened = try EditorRuntimeV2(privateRoot: root, protectedInstallationKey: Data(1...32))
        defer { try? reopened.close() }
        try reopened.configureStoreV3(SyntheticStoreVerifierV3(key: key))
        XCTAssertEqual(try reopened.commercialState().access, 0)
        let oldNonce = try reopened.beginStoreRequestV3()
        let old = try SyntheticStoreVerifierV3(key: key).sign(nonce: oldNonce, status: 1, stamp: 1000)
        XCTAssertThrowsError(try reopened.acceptStoreProofV3(old, nativeNonce: oldNonce))
        XCTAssertEqual(try reopened.commercialState().access, 0)
    }

    func testVerifiedRevocationDoesNotInterruptAnAuthorizedGestureOrStartTrialBeforeHumanApply() throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("store-gesture-v3-\(UUID())")
        let project = root.appendingPathComponent("project")
        try FileManager.default.createDirectory(at: project, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: root) }
        let fixture = try XCTUnwrap(Bundle.module.url(forResource: "decode", withExtension: "json", subdirectory: "Fixtures"))
        let json = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: fixture)) as? [String: [String: Any]])
        let source = root.appendingPathComponent("source.png")
        try Data(try XCTUnwrap(json["png"]?["bytes"] as? [UInt8])).write(to: source)
        let runtime = try EditorRuntimeV2(privateRoot: root, protectedInstallationKey: Data(1...32))
        defer { try? runtime.close() }
        let verifier = SyntheticStoreVerifierV3(key: P256.Signing.PrivateKey())
        try runtime.configureStoreV3(verifier)
        let paid = try runtime.beginStoreRequestV3()
        _ = try runtime.acceptStoreProofV3(verifier.sign(nonce: paid, status: 1, stamp: 1000), nativeNonce: paid)
        let editor = try ManualEditor(root: project, importing: source, runtimeV2: runtime)
        defer { editor.close() }
        try editor.select(4); let identity = try editor.state().editID
        try editor.update([1000], editID: identity)
        XCTAssertEqual(try editor.state().revisionCount, 1); XCTAssertEqual(try runtime.commercialState().trialStart, -1)
        XCTAssertEqual(try runtime.commercialState().photos, 0) // Paid admission consumed no Free photo slot.
        let revoked = try runtime.beginStoreRequestV3()
        _ = try runtime.acceptStoreProofV3(verifier.sign(nonce: revoked, status: 3, stamp: 1100), nativeNonce: revoked)
        XCTAssertEqual(try runtime.commercialState().access, 0)
        XCTAssertEqual(try editor.state().editID, identity); XCTAssertEqual(try editor.state().revisionCount, 1)
        try editor.action(1) // Explicit Human Apply, independently of store metadata.
        XCTAssertEqual(try editor.state().revisionCount, 2)
        XCTAssertEqual(try runtime.commercialState().access, 1)
        XCTAssertGreaterThanOrEqual(try runtime.commercialState().trialStart, 0)
        XCTAssertEqual(try runtime.commercialState().photos, 0) // No second admission/charge after revocation.
    }
    func testBusyPreviewInstallRetainsValidityButSupersededFrameCannotInstall() throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("preview-currency-\(UUID())")
        let project = root.appendingPathComponent("project")
        try FileManager.default.createDirectory(at: project, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: root) }
        let fixture = try XCTUnwrap(Bundle.module.url(forResource: "decode", withExtension: "json", subdirectory: "Fixtures"))
        let json = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: fixture)) as? [String: [String: Any]])
        let input = root.appendingPathComponent("source.png")
        try Data(try XCTUnwrap(json["png"]?["bytes"] as? [UInt8])).write(to: input)
        let runtime = try EditorRuntimeV2(privateRoot: root, protectedInstallationKey: Data(1...32))
        let owner = try ManualEditor(root: project, importing: input, runtimeV2: runtime)
        defer { owner.close() }
        let candidate = try XCTUnwrap(owner.render(owner.request()))
        let gate = try XCTUnwrap(Mirror(reflecting: owner).children.first { $0.label == "gate" }?.value as? NSRecursiveLock)
        let ready = DispatchSemaphore(value: 0), release = DispatchSemaphore(value: 0), done = DispatchSemaphore(value: 0)
        DispatchQueue.global().async {
            gate.lock(); ready.signal(); release.wait(); gate.unlock(); done.signal()
        }
        defer { release.signal() }
        XCTAssertEqual(ready.wait(timeout: .now()+5), .success)
        let busy = owner.installStatus(candidate) { _ in XCTFail("BUSY must not install pixels") }
        release.signal(); XCTAssertEqual(done.wait(timeout: .now()+5), .success)
        XCTAssertEqual(busy, .busy)
        XCTAssertEqual(owner.installStatus(candidate), .installed)
        _ = try owner.request() // Same native generation, newer display serial.
        XCTAssertEqual(owner.installStatus(candidate), .discarded)
        XCTAssertEqual(try owner.state().revisionCount, 1)
    }
    func testExistingManualFacadeUsesSeparateV2OwnerAndStableGestureIdentity() throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("editor-v2-facade-\(UUID())")
        let project = root.appendingPathComponent("project")
        try FileManager.default.createDirectory(at: project, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: root) }
        let fixture = try XCTUnwrap(Bundle.module.url(forResource: "decode", withExtension: "json", subdirectory: "Fixtures"))
        let json = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: fixture)) as? [String: [String: Any]])
        let bytes = try XCTUnwrap(json["png"]?["bytes"] as? [UInt8])
        let input = root.appendingPathComponent("synthetic.png"); try Data(bytes).write(to: input)
        let runtime = try EditorRuntimeV2(privateRoot: root, protectedInstallationKey: Data(1...32))
        let owner = try ManualEditor(root: project, importing: input, runtimeV2: runtime)
        defer { owner.close() }
        XCTAssertEqual(owner.tools.count, 11)
        let first = try XCTUnwrap(owner.render(owner.request())); XCTAssertTrue(owner.install(first))
        try owner.select(4); let identity = try owner.state().editID
        XCTAssertEqual(identity.utf8.count, 32)
        try owner.update([1000], editID: identity); XCTAssertEqual(try owner.state().editID, identity)
        XCTAssertThrowsError(try owner.update([2000], editID: String(repeating: "0", count: 32))) {
            XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 9)
        }
        XCTAssertFalse(owner.install(first)); XCTAssertEqual(try owner.state().revisionCount, 1)
        let originalRequest = try owner.request(original: true)
        let original = try XCTUnwrap(owner.render(originalRequest))
        XCTAssertEqual(original.image.dataProvider?.data as Data?, first.image.dataProvider?.data as Data?)
        let editedRequest = try owner.request()
        XCTAssertFalse(owner.isRequested(originalRequest)); XCTAssertFalse(owner.install(original))
        XCTAssertTrue(owner.isRequested(editedRequest))
        let edited = try XCTUnwrap(owner.render(editedRequest)); XCTAssertTrue(owner.install(edited))
        XCTAssertEqual(try owner.state().revisionCount, 1)
        let draft = try XCTUnwrap(owner.render(owner.request())); XCTAssertTrue(owner.install(draft))
        try owner.action(1); XCTAssertEqual(try owner.state().revisionCount, 2)
        XCTAssertFalse(owner.install(draft)); let revision = try owner.state().revisionID
        try owner.action(2); XCTAssertNotEqual(try owner.state().revisionID, revision)
        try owner.action(3); XCTAssertEqual(try owner.state().revisionID, revision)
        let pending = try XCTUnwrap(owner.prepareAdvisory(2))
        try owner.cancelAuto()
        XCTAssertThrowsError(try owner.requestAuto(2, prepared: pending)) {
            XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 13)
        }
        let advice = try owner.requestAdvice(2)
        let choice = try owner.inspectAdvice(advice)
        try owner.select(choice.toolIndex); try owner.cancelAdvice()
        XCTAssertTrue(try owner.state().active)
        try owner.action(0)
        let proposal = try owner.requestAuto(2)
        let originalReview = try owner.prepareAutoReview(proposal)
        XCTAssertFalse(try owner.inspectReview(originalReview).actions.isEmpty)
        let session = try owner.beginReviewEditing(originalReview)
        _ = try owner.editReview(session, expectedIdentity: session.bindingIdentity, operation: 1, value: 500)
        XCTAssertThrowsError(try owner.inspectReview(originalReview))
        XCTAssertNil(try owner.editReview(session, expectedIdentity: session.bindingIdentity, operation: 2))
        XCTAssertEqual(session.state, 3)
        let review = try XCTUnwrap(owner.editReview(session, expectedIdentity: session.bindingIdentity, operation: 4))
        XCTAssertThrowsError(try owner.editReview(session, expectedIdentity: originalReview.reviewIdentity, operation: 2))
        let cancelledPreview = try owner.beginReviewPreview(review)
        try owner.cancelReviewPreview(cancelledPreview)
        XCTAssertThrowsError(try owner.renderReviewPreview(cancelledPreview)) {
            XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 13)
        }
        XCTAssertFalse(try owner.inspectReview(review).actions.isEmpty)
        let previewRequest = try owner.beginReviewPreview(review)
        let pair = try owner.renderReviewPreview(previewRequest)
        XCTAssertTrue(owner.installReviewPreview(pair) {})
        try owner.cancelReviewPreview(previewRequest)
        XCTAssertFalse(owner.installReviewPreview(pair) {})
        XCTAssertFalse(try owner.inspectReview(review).actions.isEmpty)
        let legacyTransport = try owner.validateAgentPlan(review.canonicalBytes)
        XCTAssertThrowsError(try owner.approveAgentPlanFromHuman(legacyTransport)) { XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 5) }
        XCTAssertThrowsError(try owner.executeAgentPlan(legacyTransport)) { XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 15) }
        XCTAssertEqual(owner.reviewTestControl(0, 1), 0)
        XCTAssertEqual(owner.reviewTestControl(2), 0)
        let receipt = try owner.applyAutoFromHuman(review)
        XCTAssertEqual(receipt.sourceRevision, revision)
        XCTAssertEqual(receipt.resultingRevision, try owner.state().revisionID)
        XCTAssertThrowsError(try owner.applyAutoFromHuman(review)) {
            XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 15)
        }
        let result = try owner.resultReview(receipt)
        XCTAssertEqual(result.state, 1); XCTAssertTrue(result.publicationVisible)
        XCTAssertEqual(result.previous, revision); XCTAssertFalse(result.geometryChanged)
        XCTAssertGreaterThan(try owner.renderResult(result, revision: result.previous).width, 0)
        try owner.action(2)
        XCTAssertThrowsError(try owner.renderResult(result, revision: result.result)) {
            XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 9)
        }
        XCTAssertEqual(try owner.resultReview(receipt).state, 2)
        try owner.action(3)
        let savedRevision = receipt.resultingRevision
        #if DEBUG
        let binding = String(decoding: try owner.agentBinding(), as: UTF8.self).trimmingCharacters(in: .whitespacesAndNewlines)
        let actions = "{\"action_index\":0,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":101},\"tool_id\":\"pixaura.exposure\"},{\"action_index\":1,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"quarter_turns\":1},\"tool_id\":\"pixaura.rotate\"}"
        let plan = Data("{\"actions\":[\(actions)],\"binding\":\(binding),\"plan_version\":1}\n".utf8)
        let prepared = try owner.preparePlanCandidate(plan)
        let multiProposal = try owner.inspectionCandidateTest(plan, prepared: prepared)
        let multi = try owner.prepareAutoReview(multiProposal)
        XCTAssertEqual(try owner.inspectReview(multi).actions.count, 2)
        XCTAssertTrue(try owner.inspectReview(multi).actions[1].geometry)
        try owner.cancelAutoReview(multi)
        XCTAssertEqual(try owner.state().revisionID, savedRevision)
        #endif
        owner.close()
        let reopened = try ManualEditor(root: project, runtimeV2: runtime); defer { reopened.close() }
        XCTAssertEqual(try reopened.state().revisionID, savedRevision)
    }
    func testExplicitKeyLossRecoveryCannotCreateCommercialStateOrGrantEditing() throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("editor-v2-recovery-\(UUID())")
        let project = root.appendingPathComponent("project")
        try FileManager.default.createDirectory(at: project, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: root) }
        let fixture = try XCTUnwrap(Bundle.module.url(forResource: "decode", withExtension: "json", subdirectory: "Fixtures"))
        let json = try XCTUnwrap(JSONSerialization.jsonObject(with: Data(contentsOf: fixture)) as? [String: [String: Any]])
        let input = root.appendingPathComponent("synthetic.png")
        try Data(try XCTUnwrap(json["png"]?["bytes"] as? [UInt8])).write(to: input)
        let runtime = try EditorRuntimeV2(privateRoot: root, protectedInstallationKey: Data(1...32))
        let owner = try EditorContextV2(runtime: runtime, root: project, importing: input)
        XCTAssertThrowsError(try runtime.close()) { XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 15) }
        try owner.select(4); try owner.update([1000]); XCTAssertTrue(try owner.applyFromHuman())
        let approved = try owner.state().revision; owner.close(); try runtime.close()
        let ledger = root.appendingPathComponent("commercial-v2.sqlite"), previousLedger = try Data(contentsOf: root.appendingPathComponent("commercial-v2.sqlite"))
        let recovery = try EditorRuntimeV2(recoveringPrivateRoot: root)
        let recovered = try EditorContextV2(runtime: recovery, root: project)
        defer { recovered.close(); try? recovery.close() }
        XCTAssertTrue(try recovered.state().recoveryOnly); XCTAssertEqual(try recovered.state().revision, approved)
        XCTAssertThrowsError(try recovery.commercialState()) { XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 3003) }
        XCTAssertThrowsError(try recovered.select(4)) { XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 3003) }
        XCTAssertThrowsError(try EditorContextV2(runtime: recovery, root: project, importing: input)) { XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 3003) }
        let frame = try recovered.preview(width: 64, height: 48); XCTAssertTrue(frame.isCurrent)
        let exported = try recovered.exportSavedObserved(to: root.appendingPathComponent("recovery.png"), format: 2, quality: 0)
        XCTAssertEqual(exported.revision, approved); XCTAssertGreaterThan(exported.bytes, 0)
        XCTAssertEqual(try Data(contentsOf: ledger), previousLedger)
    }
    func testSharedRuntimeIgnoresDuplicateAndOlderOsCallbacks() throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("editor-v2-policy-\(UUID())")
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: root) }
        let signals = TestAppleSignals()
        let owner = AppleCapabilityOwner(signals: signals); defer { owner.close() }
        let first = try owner.snapshot()
        let runtime = try EditorRuntimeV2(privateRoot: root, protectedInstallationKey: Data(1...32), capability: first)
        defer { try? runtime.close() }
        let original = try runtime.strategy()
        XCTAssertEqual(try runtime.updateCapability(first).epoch, original.epoch)
        signals.memory = 4
        let critical = try owner.snapshot(), reduced = try runtime.updateCapability(critical)
        XCTAssertLessThan(reduced.runtimeCapBytes, original.runtimeCapBytes)
        XCTAssertEqual(try runtime.updateCapability(critical).epoch, reduced.epoch)
        XCTAssertEqual(try runtime.updateCapability(first).runtimeCapBytes, reduced.runtimeCapBytes)
        XCTAssertTrue(reduced.forcedCPU); XCTAssertEqual(reduced.backend, 0)
    }
    private func source(_ url: URL) throws {
        let width = 4000, height = 3000
        var bytes = Data(count: width * height * 4)
        bytes.withUnsafeMutableBytes { (raw: UnsafeMutableRawBufferPointer) in
            for y in 0..<height { for x in 0..<width {
                let at = (y * width + x) * 4
                raw[at] = UInt8((x * 13 + y * 7) & 255)
                raw[at + 1] = UInt8((x * 3 + y * 11) & 255)
                raw[at + 2] = UInt8((x + y * 5) & 255); raw[at + 3] = 255
            } }
        }
        let provider = try XCTUnwrap(CGDataProvider(data: bytes as CFData))
        let space = try XCTUnwrap(CGColorSpace(name: CGColorSpace.sRGB))
        let image = try XCTUnwrap(CGImage(width: width, height: height, bitsPerComponent: 8,
            bitsPerPixel: 32, bytesPerRow: width * 4, space: space,
            bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.last.rawValue),
            provider: provider, decode: nil, shouldInterpolate: false, intent: .defaultIntent))
        let destination = try XCTUnwrap(CGImageDestinationCreateWithURL(url as CFURL, "public.png" as CFString, 1, nil))
        CGImageDestinationAddImage(destination, image, nil)
        XCTAssertTrue(CGImageDestinationFinalize(destination))
    }
    func testReal12MPVersionedSwiftBridgeHistoryAIAndRecoveryExport() async throws {
        try await Task.detached {
            let root = FileManager.default.temporaryDirectory.appendingPathComponent("editor-v2-\(UUID())")
            let project = root.appendingPathComponent("project")
            try FileManager.default.createDirectory(at: project, withIntermediateDirectories: true)
            defer { try? FileManager.default.removeItem(at: root) }
            let input = root.appendingPathComponent("synthetic-12mp.png"); try self.source(input)
            // Synthetic installation secret, not Keychain/store/signing evidence.
            let runtime = try EditorRuntimeV2(privateRoot: root, protectedInstallationKey: Data(1...32))
            let owner = try EditorContextV2(runtime: runtime, root: project, importing: input)
            defer { owner.close() }
            XCTAssertEqual(try owner.state().revisionCount, 1)
            let before = try owner.preview(width: 64, height: 48, original: true)
            XCTAssertEqual(before.sourceWidth, 4000); XCTAssertEqual(before.sourceHeight, 3000)
            XCTAssertTrue(before.isCurrent); XCTAssertEqual(try before.image().width, 64)
            XCTAssertThrowsError(try before.image()) { XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 8) }
            try owner.select(4); try owner.update([1000]); XCTAssertFalse(before.isCurrent)
            let preview = try owner.preview(width: 64, height: 48)
            _ = try preview.image(); XCTAssertEqual(try owner.state().revisionCount, 1)
            XCTAssertTrue(try owner.applyFromHuman()); XCTAssertFalse(preview.isCurrent)
            let manual = try owner.state().revision
            try owner.undo(); XCTAssertNotEqual(try owner.state().revision, manual)
            try owner.redo(); XCTAssertEqual(try owner.state().revision, manual)
            let initialUses = try runtime.commercialState().ai
            let cancelledRequest = try owner.beginAdvisory(2)
            try owner.cancelAdvisoryRequest(cancelledRequest)
            XCTAssertThrowsError(try owner.runAdvisory(cancelledRequest)) {
                XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 13)
            }
            XCTAssertEqual(try runtime.commercialState().ai, initialUses)
            let abandoned = try owner.requestAdvisory(2)
            let abandonedPreview = try owner.previewAdvisory(abandoned, width: 64, height: 48)
            XCTAssertTrue(abandonedPreview.isCurrent)
            try owner.cancelAdvisory(abandoned)
            XCTAssertFalse(abandonedPreview.isCurrent)
            try owner.select(4); try owner.update([1000])
            XCTAssertThrowsError(try owner.cancelAdvisory(abandoned)) {
                XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 9)
            }
            XCTAssertEqual(try owner.state().selected, 4)
            try owner.cancel()
            let cancelledExport = try owner.beginExport(format: 2, quality: 93)
            try cancelledExport.cancel()
            let cancelledPath = root.appendingPathComponent("cancelled.png")
            XCTAssertThrowsError(try cancelledExport.run(to: cancelledPath)) {
                XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 13)
            }
            XCTAssertFalse(FileManager.default.fileExists(atPath: cancelledPath.path))
            var advice = try owner.requestAdvisory(2)
            XCTAssertEqual(try owner.inspectReview(advice).actions.count, 1)
            XCTAssertThrowsError(try owner.editAdvisory(advice, operations: Data("{".utf8)))
            XCTAssertThrowsError(try owner.applyAdvisoryFromHuman(advice)) {
                XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 13)
            }
            advice = try owner.restoreReview(advice)
            let operations = try owner.advisoryContent(advice, operations: true)
            XCTAssertFalse(operations.isEmpty); XCTAssertEqual(try owner.state().revisionCount, 2)
            let empty = try owner.editAdvisory(advice, operations: Data("{\"operations\":[]}".utf8))
            XCTAssertThrowsError(try owner.applyAdvisoryFromHuman(empty)) {
                XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 7)
            }
            let restored = try owner.restoreReview(empty)
            XCTAssertTrue(try owner.applyAdvisoryFromHuman(restored))
            XCTAssertThrowsError(try owner.applyAdvisoryFromHuman(restored)) {
                XCTAssertEqual(($0 as? ManualEditorFailure)?.status, 9)
            }
            let approved = try owner.state().revision; XCTAssertEqual(try owner.state().revisionCount, 3)
            for format in [UInt32(1), UInt32(2)] {
                let output = root.appendingPathComponent("full-\(format)")
                let exported = try owner.exportSavedObserved(to: output, format: format, quality: 93)
                XCTAssertGreaterThan(exported.bytes, 0); XCTAssertEqual(exported.revision, approved)
                XCTAssertEqual(exported.width, 4000); XCTAssertEqual(exported.height, 3000)
                XCTAssertEqual(try owner.state().revisionCount, 3)
                let encoded = try XCTUnwrap(CGImageSourceCreateWithURL(output as CFURL, nil))
                let image = try XCTUnwrap(CGImageSourceCreateImageAtIndex(encoded, 0, nil))
                XCTAssertEqual(image.width, 4000); XCTAssertEqual(image.height, 3000)
            }
            owner.close()
            let reopened = try EditorContextV2(runtime: runtime, root: project)
            defer { reopened.close() }
            XCTAssertEqual(try reopened.state().revision, approved)
            XCTAssertEqual(try reopened.state().revisionCount, 3)
        }.value
    }
}
