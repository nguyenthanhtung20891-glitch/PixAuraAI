import Foundation
import CoreGraphics
import Darwin
import CPixAuraCore
#if DEBUG
@_silgen_name("pixaura_test_editor_v2_control")
private func editorV2CertificationControl(_ owner: UInt64, _ family: UInt32, _ command: UInt32, _ value: UInt32) -> Int32
#endif

private func v2Check(_ status: Int32, _ boundary: String) throws {
    if status != 0 { throw ManualEditorFailure(status: status, boundary: boundary) }
}
private func v2String<T>(_ value: T) -> String {
    withUnsafeBytes(of: value) { raw in
        String(decoding: raw.prefix(while: { $0 != 0 }), as: UTF8.self)
    }
}
private func v2Path(_ url: URL) throws -> String {
    try url.withUnsafeFileSystemRepresentation { path in
        guard let path, let canonical = realpath(path, nil) else {
            throw ManualEditorFailure(status: 12, boundary: "Open")
        }
        defer { free(canonical) }; return String(cString: canonical)
    }
}

/// One application-owned runtime shared by all v2 documents. The installation
/// key comes from the platform's protected, non-synchronizing local key store;
/// it is not a store entitlement, signing identity, user identifier or UI grant.
public final class EditorRuntimeV2 {
    private let lifecycle = NSLock()
    private var nativeHandle: UInt64
    private var observationSequence: UInt64 = 0
    private var frameRetirement: (() -> Void)?
    private var storeObserversV3: [UUID: @Sendable () -> Void] = [:]
    fileprivate var handle: UInt64 { lifecycle.lock(); defer { lifecycle.unlock() }; return nativeHandle }
    public init(privateRoot: URL, protectedInstallationKey: Data, capability: AppleCapabilitySnapshot? = nil) throws {
        let path = try v2Path(privateRoot)
        var native: UInt64 = 0
        let status = path.withCString { pointer in
            protectedInstallationKey.withUnsafeBytes { key in
                if var capability {
                    var strategy = pixaura_execution_strategy()
                    return pixaura_v2_runtime_create_policy(2, pointer, UInt64(path.utf8.count),
                        key.bindMemory(to: UInt8.self).baseAddress, UInt64(key.count),
                        &capability.profile, &capability.observation, &strategy, &native)
                }
                return pixaura_v2_runtime_create(2, pointer, UInt64(path.utf8.count),
                    key.bindMemory(to: UInt8.self).baseAddress, UInt64(key.count), &native)
            }
        }
        try v2Check(status, "Commercial owner"); nativeHandle = native
        observationSequence = capability?.observation.sequence ?? 0
    }
    /// Explicit key-loss recovery creates no commercial state and never grants
    /// NEW editing. Existing approved source/history/export remain native-owned.
    public init(recoveringPrivateRoot root: URL, capability: AppleCapabilitySnapshot? = nil) throws {
        let path = try v2Path(root); var opened: UInt64 = 0
        let status = path.withCString { pointer in
            if var capability {
                var strategy = pixaura_execution_strategy()
                return pixaura_v2_runtime_recovery_create(2, pointer, UInt64(path.utf8.count),
                    &capability.profile, &capability.observation, &strategy, &opened)
            }
            return pixaura_v2_runtime_recovery_create(2, pointer, UInt64(path.utf8.count), nil, nil, nil, &opened)
        }
        try v2Check(status, "Recovery owner"); nativeHandle = opened
        observationSequence = capability?.observation.sequence ?? 0
    }
    /// Resource retirement only; this callback cannot publish or approve edits.
    public func observeFrameRetirement(_ observer: @escaping () -> Void) {
        lifecycle.lock(); frameRetirement = observer; lifecycle.unlock()
    }
    fileprivate func frameRetired() {
        lifecycle.lock(); let observer = frameRetirement; lifecycle.unlock(); observer?()
    }
    public func strategy() throws -> AppleExecutionStrategy {
        var native = pixaura_execution_strategy()
        try v2Check(pixaura_v2_runtime_strategy(2, handle, &native), "Capability")
        return AppleExecutionStrategy(native)
    }
    @discardableResult public func updateCapability(_ capability: AppleCapabilitySnapshot) throws -> AppleExecutionStrategy {
        lifecycle.lock(); defer { lifecycle.unlock() }
        var observation = capability.observation, native = pixaura_execution_strategy()
        // All app scenes share one OS observation owner. A duplicate callback
        // must not become a stale-edit error or regress the runtime's policy.
        if observation.sequence <= observationSequence {
            try v2Check(pixaura_v2_runtime_strategy(2, nativeHandle, &native), "Capability")
        } else {
            try v2Check(pixaura_v2_runtime_update(2, nativeHandle, &observation, &native), "Capability")
            observationSequence = observation.sequence
        }
        return AppleExecutionStrategy(native)
    }
    public func observeStoreChangesV3(_ observer: @escaping @Sendable () -> Void) throws -> UUID {
        lifecycle.lock(); defer { lifecycle.unlock() }
        guard nativeHandle != 0, storeObserversV3.count < 8 else { throw ManualEditorFailure(status: 8, boundary: "Store observers") }
        let token = UUID(); storeObserversV3[token] = observer; return token
    }
    public func removeStoreObserverV3(_ token: UUID) {
        lifecycle.lock(); storeObserversV3.removeValue(forKey: token); lifecycle.unlock()
    }
    func storeDidCommitV3() {
        lifecycle.lock(); let observers = Array(storeObserversV3.values); lifecycle.unlock()
        for observer in observers { observer() } // After native return, never inside a cryptographic callback.
    }
    // Module-private, versioned store adapter uses the same retained runtime.
    var storeRuntimeHandleV3: UInt64 { handle }
    public func commercialState() throws -> CommercialStateV2 {
        var native = pixaura_v2_commercial_state()
        try v2Check(pixaura_v2_runtime_commercial_state(2, handle, &native), "Commercial state")
        return CommercialStateV2(native)
    }
    /// Close contexts and retire display frames first. A busy native owner is
    /// retained on failure; it is never hidden by clearing a live handle.
    public func close() throws {
        lifecycle.lock(); defer { lifecycle.unlock() }
        if nativeHandle == 0 { return }
        try v2Check(pixaura_v2_runtime_close(2, nativeHandle), "Runtime close")
        nativeHandle = 0; storeObserversV3.removeAll()
    }
    deinit { try? close() }
}

public struct CommercialStateV2 {
    public let access: UInt32, photos: UInt32, ai: UInt32, pending: UInt32
    public let day: Int64, logicalUTC: Int64, trialStart: Int64, trialEnd: Int64
    public let clockUncertain: Bool
    fileprivate init(_ native: pixaura_v2_commercial_state) {
        access = native.access; photos = native.photos; ai = native.ai; pending = native.pending
        day = native.day; logicalUTC = native.logical_utc
        trialStart = native.trial_start; trialEnd = native.trial_end; clockUncertain = native.clock_uncertain != 0
    }
}

public struct EditorStateV2 {
    public let revision: String, revisionCount: Int, selected: Int
    public let epoch: UInt64, generation: UInt64
    public let pending: Bool, canUndo: Bool, canRedo: Bool
    public let recoveryOnly: Bool, commercialUnavailable: Bool
    public let values: [Int32]
    fileprivate init(_ native: pixaura_v2_state) {
        revision = v2String(native.revision); revisionCount = Int(native.revisions)
        selected = native.selected == UInt32.max ? -1 : Int(native.selected)
        epoch = native.epoch; generation = native.generation
        pending = native.pending != 0; canUndo = native.can_undo != 0; canRedo = native.can_redo != 0
        recoveryOnly = native.recovery_only != 0; commercialUnavailable = native.commerce_unavailable != 0
        values = withUnsafeBytes(of: native.values) {
            Array($0.bindMemory(to: Int32.self).prefix(Int(native.value_count)))
        }
    }
}
public struct EditorAdvisoryV2 {
    public let id: String, delivery: String, choice: Int, tool: Int, epoch: UInt64
    fileprivate init(_ native: pixaura_v2_advisory) {
        id = v2String(native.id); delivery = v2String(native.delivery)
        choice = Int(native.choice); tool = Int(native.tool); epoch = native.epoch
    }
}
public struct EditorHistoryV2: Decodable {
    public struct Revision: Decodable {
        public let id: String, actor: String, geometrySHA256: String
        public let parentID: String?, planID: String?
        enum CodingKeys: String, CodingKey { case id, actor; case parentID = "parent_id", planID = "plan_id", geometrySHA256 = "geometry_sha256" }
    }
    public let current: String, documentID: String, sourceSHA256: String
    public let epoch: UInt64, version: UInt32
    public let redo: [String], revisions: [Revision]
    enum CodingKeys: String, CodingKey {
        case current, epoch, version, redo, revisions
        case documentID = "document_id", sourceSHA256 = "source_sha256"
    }
}
public struct NativePublicationV2 {
    public let event: String, source: String, project: String, document: String
    public let parent: String, revision: String, revisionDigest: String
    public let epoch: UInt64, logicalUTC: Int64
    public let clockUncertain: Bool, utcAuthenticated: Bool
    fileprivate init(_ native: pixaura_v2_commit_result) {
        event = v2String(native.event); source = v2String(native.source)
        project = v2String(native.project); document = v2String(native.document)
        parent = v2String(native.parent); revision = v2String(native.revision)
        revisionDigest = v2String(native.revision_digest); epoch = native.epoch; logicalUTC = native.logical_utc
        clockUncertain = native.clock_uncertain != 0; utcAuthenticated = native.utc_authenticated != 0
    }
}

public struct NativeReviewParameterV2 {
    public let name: String, unit: String
    public let raw: Int32, minimum: Int32, maximum: Int32, neutral: Int32
}
public struct NativeReviewActionV2 {
    public let index: Int, toolIndex: Int, toolID: String, label: String
    public let operationVersion: UInt32, parameterVersion: UInt32, geometry: Bool
    public let parameters: [NativeReviewParameterV2]
}
public struct NativeReviewInspectionV2 {
    public let providerID: String, epoch: UInt64, warnings: UInt32
    public let actions: [NativeReviewActionV2]
}

public struct NativeExportObservationV2: Sendable {
    public let width: UInt32, height: UInt32, format: UInt32
    public let bytes: UInt64, epoch: UInt64
    public let source: String, document: String, revision: String
    fileprivate init(_ result: pixaura_v2_export_result) {
        width = result.width; height = result.height; format = result.format
        bytes = result.bytes; epoch = result.epoch
        source = v2String(result.source); document = v2String(result.document); revision = v2String(result.revision)
    }
}

public final class EditorExportV2 {
    fileprivate let owner: EditorContextV2, editor: UInt64, ticket: UInt64
    fileprivate init(owner: EditorContextV2, editor: UInt64, ticket: UInt64) { self.owner = owner; self.editor = editor; self.ticket = ticket }
    public func cancel() throws { try v2Check(pixaura_v2_export_cancel(2, editor, ticket), "Cancel export") }
    public func run(to staging: URL) throws -> NativeExportObservationV2 {
        let fd = staging.withUnsafeFileSystemRepresentation { path in path.map { Darwin.open($0, O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0o600) } ?? -1 }
        guard fd >= 0 else { throw ManualEditorFailure(status: 12, boundary: "Export staging") }
        defer { Darwin.close(fd) }; var result = pixaura_v2_export_result()
        do {
            try v2Check(pixaura_v2_export_run(2, editor, ticket, fd, &result), "Export")
            guard result.version == 2, result.struct_size == 176, result.bytes > 0, result.width > 0, result.height > 0 else { throw ManualEditorFailure(status: 14, boundary: "Export observation") }
        } catch { try? FileManager.default.removeItem(at: staging); throw error }
        return NativeExportObservationV2(result)
    }
    deinit { _ = pixaura_v2_export_release(2, editor, ticket) }
}

public final class EditorReviewPreviewV2 {
    fileprivate let owner: EditorContextV2, editor: UInt64, runtime: EditorRuntimeV2, ticket: UInt64
    fileprivate init(owner: EditorContextV2, editor: UInt64, runtime: EditorRuntimeV2, ticket: UInt64) {
        self.owner = owner; self.editor = editor; self.runtime = runtime; self.ticket = ticket
    }
    public func cancel() throws { try v2Check(pixaura_v2_review_preview_cancel(2, editor, ticket), "Cancel review preview") }
    public func render() throws -> EditorReviewPairV2 {
        var before = pixaura_v2_preview(), after = pixaura_v2_preview()
        try v2Check(pixaura_v2_review_preview_render(2, editor, ticket, &before, &after), "Review preview")
        return EditorReviewPairV2(task: self,
            before: EditorFrameV2(before, editor: editor, runtime: runtime),
            after: EditorFrameV2(after, editor: editor, runtime: runtime))
    }
    deinit { _ = pixaura_v2_review_preview_release(2, editor, ticket) }
}
public struct EditorReviewPairV2 {
    fileprivate let task: EditorReviewPreviewV2
    public let before: EditorFrameV2, after: EditorFrameV2
    public var isCurrent: Bool { before.isCurrent && after.isCurrent }
}

public enum EditorFrameCurrencyV2: Equatable { case current, busy, stale, unavailable }

public final class EditorFrameV2 {
    fileprivate let runtime: EditorRuntimeV2
    fileprivate let native: pixaura_v2_preview
    fileprivate let editor: UInt64
    private let imageLock = NSLock()
    private var imageCreated = false
    public var width: Int { Int(native.width) }
    public var height: Int { Int(native.height) }
    public var sourceWidth: Int { Int(native.source_width) }
    public var sourceHeight: Int { Int(native.source_height) }
    fileprivate init(_ native: pixaura_v2_preview, editor: UInt64, runtime: EditorRuntimeV2) {
        self.native = native; self.editor = editor; self.runtime = runtime
    }
    public var isCurrent: Bool { pixaura_v2_frame_current(2, editor, native.frame) == 0 }
    public var currency: EditorFrameCurrencyV2 {
        var value: UInt32 = 2
        guard pixaura_v2_frame_probe(2, editor, native.frame, 0, &value) == 0 else { return .unavailable }
        switch value { case 0: return .current; case 1: return .busy; case 2: return .stale; default: return .unavailable }
    }
    private final class Pixels {
        let frame: EditorFrameV2, bytes: UnsafeMutableRawPointer
        init(_ frame: EditorFrameV2) {
            self.frame = frame; bytes = .allocate(byteCount: Int(frame.native.bytes), alignment: 16)
        }
        deinit { bytes.deallocate() }
    }
    /// The provider retains the native frame and its consumer reservation for
    /// exactly as long as the displayed image owns these bytes.
    public func image() throws -> CGImage {
        imageLock.lock()
        guard !imageCreated else { imageLock.unlock(); throw ManualEditorFailure(status: 8, boundary: "Preview ownership") }
        imageCreated = true; imageLock.unlock()
        let pixels = Pixels(self)
        try v2Check(pixaura_v2_frame_copy(2, editor, native.frame,
            pixels.bytes.assumingMemoryBound(to: UInt8.self), native.bytes), "Preview")
        let owned = Unmanaged.passRetained(pixels).toOpaque()
        guard let provider = CGDataProvider(dataInfo: owned, data: pixels.bytes,
            size: Int(native.bytes), releaseData: { context, _, _ in
                if let context { Unmanaged<Pixels>.fromOpaque(context).release() }
            }) else { Unmanaged<Pixels>.fromOpaque(owned).release(); throw ManualEditorFailure(status: 8, boundary: "Preview") }
        guard let space = CGColorSpace(name: CGColorSpace.sRGB),
            let image = CGImage(width: width, height: height, bitsPerComponent: 8, bitsPerPixel: 32,
                bytesPerRow: width * 4, space: space,
                bitmapInfo: CGBitmapInfo.byteOrder32Big.union(CGBitmapInfo(rawValue: CGImageAlphaInfo.last.rawValue)),
                provider: provider, decode: nil, shouldInterpolate: false, intent: .defaultIntent)
        else { throw ManualEditorFailure(status: 8, boundary: "Preview") }
        return image
    }
    deinit { if pixaura_v2_frame_release(2, native.frame) == 0 { runtime.frameRetired() } }
}

/// Synchronous bridge for platform worker lanes. Native owns history, approvals,
/// cancellation and commercial admission. A restored advisory is untrusted and
/// cannot restore an approval. The platform serializes check/display replacement.
public final class EditorContextV2 {
    private let runtime: EditorRuntimeV2
    private let lock = NSLock()
    private var native: UInt64
    public init(runtime: EditorRuntimeV2, root: URL, importing input: URL? = nil,
                explicitlyMigrateSchema3: Bool = false) throws {
        self.runtime = runtime
        let path = try v2Path(root)
        let file = try input.map { try FileHandle(forReadingFrom: $0) }
        defer { try? file?.close() }
        let count = try input.map { UInt64(try $0.resourceValues(forKeys: [.fileSizeKey]).fileSize ?? 0) } ?? 0
        var opened: UInt64 = 0
        let status = path.withCString {
            pixaura_v2_editor_open(2, runtime.handle, $0, UInt64(path.utf8.count),
                file?.fileDescriptor ?? -1, count, explicitlyMigrateSchema3 ? 1 : 0, &opened)
        }
        try v2Check(status, "Open"); native = opened
    }
    private func handle() throws -> UInt64 {
        lock.lock(); defer { lock.unlock() }
        guard native != 0 else { throw ManualEditorFailure(status: 13, boundary: "Closed editor") }
        return native
    }
    public func state() throws -> EditorStateV2 {
        var value = pixaura_v2_state(); try v2Check(pixaura_v2_editor_state(2, try handle(), &value), "State")
        return EditorStateV2(value)
    }
    public func gestureIdentity() throws -> String {
        var value = [CChar](repeating: 0, count: 33)
        try v2Check(pixaura_v2_editor_edit_identity(2, try handle(), &value, 33), "Gesture identity")
        return value.withUnsafeBufferPointer { String(cString: $0.baseAddress!) }
    }
    public func history() throws -> EditorHistoryV2 {
        let owner = try handle(); var data = Data(count: 1048576); var required: UInt64 = 0
        try v2Check(data.withUnsafeMutableBytes {
            pixaura_v2_editor_history_json(2, owner, $0.bindMemory(to: UInt8.self).baseAddress, 1048576, &required)
        }, "Ordinary history")
        data.count = Int(required)
        let history = try JSONDecoder().decode(EditorHistoryV2.self, from: data)
        guard history.version == 2, history.revisions.count <= 4096 else { throw ManualEditorFailure(status: 14, boundary: "Ordinary history") }
        return history
    }
    public func previewRevision(_ revision: String, width: UInt32 = 256, height: UInt32 = 256) throws -> EditorFrameV2 {
        let owner = try handle(); var result = pixaura_v2_preview()
        let edge = (try? runtime.strategy().previewEdge) ?? 720
        try v2Check(revision.withCString {
            pixaura_v2_editor_revision_preview(2, owner, $0, UInt64(revision.utf8.count), min(width, edge), min(height, edge), &result)
        }, "Saved revision preview")
        return EditorFrameV2(result, editor: owner, runtime: runtime)
    }
    public func select(_ tool: UInt32) throws { try v2Check(pixaura_v2_editor_select(2, try handle(), tool), "Tool") }
    public func update(_ values: [Int32]) throws {
        let owner = try handle()
        try v2Check(values.withUnsafeBufferPointer { pixaura_v2_editor_update(2, owner, $0.baseAddress, UInt32($0.count)) }, "Parameter")
    }
    @discardableResult public func applyFromHuman() throws -> Bool {
        var changed: UInt32 = 0; try v2Check(pixaura_v2_editor_apply(2, try handle(), &changed), "Commit"); return changed != 0
    }
    public func cancel() throws { try v2Check(pixaura_v2_editor_cancel(2, try handle()), "Cancel") }
    public func undo() throws { try v2Check(pixaura_v2_editor_history(2, try handle(), 0), "Undo") }
    public func redo() throws { try v2Check(pixaura_v2_editor_history(2, try handle(), 1), "Redo") }
    public func preview(width: UInt32 = 512, height: UInt32 = 512, original: Bool = false) throws -> EditorFrameV2 {
        let owner = try handle(); var result = pixaura_v2_preview()
        let edge = (try? runtime.strategy().previewEdge) ?? 720
        try v2Check(pixaura_v2_editor_preview(2, owner, min(width, edge), min(height, edge), original ? 1 : 0, &result), "Preview")
        return EditorFrameV2(result, editor: owner, runtime: runtime)
    }
    public func agentBytes(mode: UInt32, input: Data = Data()) throws -> Data {
        let owner = try handle(); var output = Data(count: 8192); var required: UInt64 = 0
        try v2Check(output.withUnsafeMutableBytes { destination in input.withUnsafeBytes { source in
            pixaura_v2_editor_agent_bytes(2, owner, mode, source.bindMemory(to: UInt8.self).baseAddress,
                UInt64(source.count), destination.bindMemory(to: UInt8.self).baseAddress, 8192, &required)
        } }, "Agent data")
        guard required > 0, required <= 8192 else { throw ManualEditorFailure(status: 14, boundary: "Agent data") }
        output.count = Int(required); return output
    }
    public func beginPlanCandidate(_ bytes: Data) throws -> AdvisoryRequest {
        let owner = try handle(); var ticket: UInt64 = 0
        try v2Check(bytes.withUnsafeBytes { pixaura_v2_advisory_plan_begin(2, owner,
            $0.bindMemory(to: UInt8.self).baseAddress, UInt64($0.count), &ticket) }, "Begin Plan candidate")
        return AdvisoryRequest(editor: owner, ticket: ticket)
    }
    public struct AdvisoryRequest {
        fileprivate let editor: UInt64, ticket: UInt64
    }
    /// Begin before scheduling asynchronous work, then retain this ticket for
    /// cancellation. A ticket is not a proposal or approval capability.
    public func beginAdvisory(_ choice: UInt32) throws -> AdvisoryRequest {
        let owner = try handle(); var ticket: UInt64 = 0
        try v2Check(pixaura_v2_advisory_begin(2, owner, choice, &ticket), "Begin AI proposal")
        return AdvisoryRequest(editor: owner, ticket: ticket)
    }
    public func runAdvisory(_ request: AdvisoryRequest) throws -> EditorAdvisoryV2 {
        let owner = try handle(); try v2Check(owner == request.editor ? 0 : 9, "AI request owner")
        var result = pixaura_v2_advisory()
        try v2Check(pixaura_v2_advisory_run(2, owner, request.ticket, &result), "AI proposal")
        return EditorAdvisoryV2(result)
    }
    public func cancelAdvisoryRequest(_ request: AdvisoryRequest) throws {
        let owner = try handle(); try v2Check(owner == request.editor ? 0 : 9, "AI request owner")
        try v2Check(pixaura_v2_advisory_request_cancel(2, owner, request.ticket), "Cancel AI request")
    }
    public func requestAdvisory(_ choice: UInt32) throws -> EditorAdvisoryV2 {
        var result = pixaura_v2_advisory(); try v2Check(pixaura_v2_advisory_request(2, try handle(), choice, &result), "AI proposal")
        return EditorAdvisoryV2(result)
    }
    /// Cancels only this native review. An abandoned review cannot cancel a
    /// newer manual gesture or acquire authority over another proposal.
    public func cancelAdvisory(_ advice: EditorAdvisoryV2) throws {
        let owner = try handle()
        try v2Check(advice.id.withCString {
            pixaura_v2_advisory_cancel(2, owner, $0, UInt64(advice.id.utf8.count))
        }, "Cancel advisory")
    }
    public func restoreAdvisory(_ delivery: String) throws -> EditorAdvisoryV2 {
        let owner = try handle(); var result = pixaura_v2_advisory()
        try v2Check(delivery.withCString { pixaura_v2_advisory_restore(2, owner, $0, UInt64(delivery.utf8.count), &result) }, "Restore advisory")
        return EditorAdvisoryV2(result)
    }
    public func pendingAdvisories() throws -> [String] {
        let owner = try handle(); var bytes = Data(count: 8192); var required: UInt64 = 0
        try v2Check(bytes.withUnsafeMutableBytes {
            pixaura_v2_advisory_pending(2, owner, $0.bindMemory(to: UInt8.self).baseAddress, 8192, &required)
        }, "Advisory inbox")
        bytes.count = Int(required)
        return try JSONDecoder().decode([String].self, from: bytes)
    }
    public func advisoryContent(_ advice: EditorAdvisoryV2, operations: Bool) throws -> Data {
        let owner = try handle(); var bytes = Data(count: 8192); var required: UInt64 = 0
        try v2Check(advice.id.withCString { id in bytes.withUnsafeMutableBytes {
            pixaura_v2_advisory_content(2, owner, id, UInt64(advice.id.utf8.count), operations ? 1 : 0,
                $0.bindMemory(to: UInt8.self).baseAddress, 8192, &required)
        } }, "Inspect advisory")
        bytes.count = Int(required); return bytes
    }
    public func inspectReview(_ advice: EditorAdvisoryV2) throws -> NativeReviewInspectionV2 {
        let editor = try handle(); var result = pixaura_v2_review_inspection()
        try v2Check(advice.id.withCString { pixaura_v2_advisory_inspect(2, editor, $0, UInt64(advice.id.utf8.count), &result) }, "Inspect native review")
        guard result.version == 2, result.struct_size == 4640, result.action_count <= 8, result.warnings <= 3,
              result.epoch == advice.epoch, v2String(result.review_id) == advice.id else { throw ManualEditorFailure(status: 14, boundary: "Native inspection") }
        let count = Int(result.action_count)
        let actions = try withUnsafeBytes(of: result.actions) { buffer in
            try buffer.bindMemory(to: pixaura_v2_review_action.self).prefix(count).enumerated().map { index, action in
                guard action.index == UInt32(index), action.tool_index < 11, action.operation_version == 1, action.parameter_version == 1,
                      action.parameter_count > 0, action.parameter_count <= 4, action.geometry <= 1 else { throw ManualEditorFailure(status: 14, boundary: "Native inspection action") }
                let parameters = withUnsafeBytes(of: action.parameters) { raw in
                    raw.bindMemory(to: pixaura_v2_review_parameter.self).prefix(Int(action.parameter_count)).map {
                        NativeReviewParameterV2(name: v2String($0.name), unit: v2String($0.unit), raw: $0.raw, minimum: $0.minimum, maximum: $0.maximum, neutral: $0.neutral)
                    }
                }
                return NativeReviewActionV2(index: index, toolIndex: Int(action.tool_index), toolID: v2String(action.tool_id), label: v2String(action.label), operationVersion: action.operation_version, parameterVersion: action.parameter_version, geometry: action.geometry == 1, parameters: parameters)
            }
        }
        return NativeReviewInspectionV2(providerID: v2String(result.provider_id), epoch: result.epoch, warnings: result.warnings, actions: actions)
    }
    public func editReviewCommand(_ advice: EditorAdvisoryV2, operation: UInt32, action: UInt32, parameter: UInt32, value: Int32, destination: UInt32) throws -> EditorAdvisoryV2 {
        let editor = try handle(); var result = pixaura_v2_advisory()
        try v2Check(advice.id.withCString { pixaura_v2_advisory_edit_command(2, editor, $0, UInt64(advice.id.utf8.count), operation, action, parameter, value, destination, &result) }, "Native review edit")
        return EditorAdvisoryV2(result)
    }
    public func editAdvisory(_ advice: EditorAdvisoryV2, operations: Data) throws -> EditorAdvisoryV2 {
        let owner = try handle(); var result = pixaura_v2_advisory()
        try v2Check(advice.id.withCString { id in operations.withUnsafeBytes {
            pixaura_v2_advisory_edit(2, owner, id, UInt64(advice.id.utf8.count), $0.bindMemory(to: UInt8.self).baseAddress, UInt64($0.count), &result)
        } }, "Edit advisory"); return EditorAdvisoryV2(result)
    }
    public func restoreReview(_ advice: EditorAdvisoryV2) throws -> EditorAdvisoryV2 {
        let owner = try handle(); var result = pixaura_v2_advisory()
        try v2Check(advice.id.withCString { pixaura_v2_advisory_reset(2, owner, $0, UInt64(advice.id.utf8.count), &result) }, "Restore review")
        return EditorAdvisoryV2(result)
    }
    @discardableResult public func applyAdvisoryFromHuman(_ advice: EditorAdvisoryV2) throws -> Bool {
        _ = try applyAdvisoryReceiptFromHuman(advice); return true
    }
    public func applyAdvisoryReceiptFromHuman(_ advice: EditorAdvisoryV2) throws -> NativePublicationV2 {
        let owner = try handle(); var token = [CChar](repeating: 0, count: 33); var result = pixaura_v2_commit_result()
        try v2Check(advice.id.withCString { pixaura_v2_advisory_approve(2, owner, $0, UInt64(advice.id.utf8.count), &token, 33) }, "Human approval")
        try v2Check(advice.id.withCString { id in token.withUnsafeBufferPointer {
            pixaura_v2_advisory_apply_receipt(2, owner, id, UInt64(advice.id.utf8.count), $0.baseAddress, 32, &result)
        } }, "Human AI Apply")
        guard result.version == 2, result.struct_size == 336, result.kind == 2 else { throw ManualEditorFailure(status: 14, boundary: "Native publication receipt") }
        return NativePublicationV2(result)
    }
    public func beginReviewPreview(_ advice: EditorAdvisoryV2, width: UInt32 = 512, height: UInt32 = 512) throws -> EditorReviewPreviewV2 {
        let editor = try handle(); var ticket: UInt64 = 0
        let edge = (try? runtime.strategy().previewEdge) ?? 720
        try v2Check(advice.id.withCString {
            pixaura_v2_review_preview_begin(2, editor, $0, UInt64(advice.id.utf8.count), min(width, edge), min(height, edge), &ticket)
        }, "Begin review preview")
        return EditorReviewPreviewV2(owner: self, editor: editor, runtime: runtime, ticket: ticket)
    }
    public func previewAdvisory(_ advice: EditorAdvisoryV2, width: UInt32 = 512, height: UInt32 = 512) throws -> EditorFrameV2 {
        let owner = try handle(); var result = pixaura_v2_preview()
        try v2Check(advice.id.withCString { pixaura_v2_advisory_preview(2, owner, $0, UInt64(advice.id.utf8.count), width, height, &result) }, "Review preview")
        return EditorFrameV2(result, editor: owner, runtime: runtime)
    }
    /// Streams full-resolution output to a newly created private staging file.
    /// Platform permission/destination delivery is a separate operation.
    @discardableResult public func exportSaved(to staging: URL, format: UInt32, quality: Int32 = 90) throws -> UInt64 {
        try exportSavedObserved(to: staging, format: format, quality: quality).bytes
    }
    public func beginExport(format: UInt32, quality: Int32 = 90) throws -> EditorExportV2 {
        let editor = try handle(); var ticket: UInt64 = 0
        try v2Check(pixaura_v2_export_begin(2, editor, format, quality, &ticket), "Begin export")
        return EditorExportV2(owner: self, editor: editor, ticket: ticket)
    }
    public func exportSavedObserved(to staging: URL, format: UInt32, quality: Int32 = 90) throws -> NativeExportObservationV2 {
        try beginExport(format: format, quality: quality).run(to: staging)
    }
#if DEBUG
    func certificationControl(_ family: UInt32, _ command: UInt32, _ value: UInt32) -> Int32 {
        lock.lock(); let retained = native; lock.unlock()
        return retained == 0 ? 13 : editorV2CertificationControl(retained, family, command, value)
    }
#endif
    public func stop() { lock.lock(); let owner = native; lock.unlock(); if owner != 0 { _ = pixaura_v2_editor_stop(2, owner) } }
    public func close() {
        lock.lock(); let owner = native; native = 0; lock.unlock()
        if owner != 0 { _ = pixaura_v2_editor_stop(2, owner); _ = pixaura_v2_editor_close(2, owner) }
    }
    deinit { close() }
}
