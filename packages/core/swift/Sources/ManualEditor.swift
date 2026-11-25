import Foundation
import CoreGraphics
import Darwin
import CPixAuraCore
import CryptoKit
#if DEBUG
@_silgen_name("pixaura_test_review_preview_control")
private func reviewPreviewControl(_ context: UnsafeMutablePointer<pixaura_editor_context>, _ command: UInt32, _ point: UInt32) -> Int32
@_silgen_name("pixaura_test_agent_platform_control")
private func agentPlatformControl(_ context: UnsafeMutablePointer<pixaura_editor_context>, _ command: UInt32, _ point: UInt32) -> Int32
@_silgen_name("pixaura_test_assist_control")
private func assistTestControl(_ context: UnsafeMutablePointer<pixaura_editor_context>, _ command: UInt32, _ point: UInt32) -> Int32
@_silgen_name("pixaura_test_auto_control")
private func PixAuraCoreAutoControl(_ context: UnsafeMutablePointer<pixaura_editor_context>, _ command: UInt32, _ point: UInt32) -> Int32
#endif

public struct ManualParameter: Decodable {
    public let name: String, unit: String
    public let minimum: Int, maximum: Int, step: Int, defaultValue: Int
    enum CodingKeys: String, CodingKey { case name, unit, minimum, maximum, step; case defaultValue = "default" }
    public func integerPosition(_ position: Double) -> Int32 {
        Int32(minimum + Int(floor((position - Double(minimum)) / Double(step) + 0.5)) * step)
    }
    public func displayValue(_ value: Int32) -> String {
        if unit == "clockwise_quarter_turns" { return "\(value * 90)° clockwise" }
        if unit == "daylight_white_kelvin" { return "\(value) K" }
        if unit.contains("exposure_stop") { return "\(Double(value) / 1000) EV" }
        if unit.contains("log2_contrast") { return "\(Double(value) / 1000) stops" }
        if unit.hasPrefix("thousandths") { return "\(Double(value) / 10)%" }
        return String(value)
    }
}
public struct ManualTool: Decodable {
    public let toolID: String, name: String, category: String
    public let parameters: [ManualParameter]
    enum CodingKeys: String, CodingKey { case toolID = "tool_id"; case name, category, parameters }
}
public struct ManualEditorState: Decodable {
    public let active: Bool, approvalPending: Bool, canUndo: Bool, canRedo: Bool
    public let revisionCount: Int, revisionID: String, editID: String, selected: Int, gestureState: Int, values: [Int32]
    enum CodingKeys: String, CodingKey {
        case active, selected, values
        case approvalPending = "approval_pending", canUndo = "can_undo", canRedo = "can_redo"
        case revisionCount = "revision_count", revisionID = "revision_id"
        case editID = "edit_id"
        case gestureState = "gesture_state"
    }
}
public struct ManualEditorFailure: Error, LocalizedError {
    public let status: Int32, boundary: String
    public init(status: Int32, boundary: String) { self.status = status; self.boundary = boundary }
    public var errorDescription: String? {
        let meaning: String
        switch status {
        case 3001: meaning = "Daily photo limit reached. Saved photos remain recoverable."
        case 3002: meaning = "Daily AI proposal limit reached. Manual editing remains available."
        case 3003: meaning = "Editing admission unavailable. Inspect or export your saved revision."
        case 7: meaning = "Invalid parameter"
        case 3, 9: meaning = "Stale edit; reopen the committed project"
        case 8: meaning = "Resource/admission failure"
        case 15: meaning = "Human approval required"
        case 5, 16: meaning = "Tool or image capability unavailable"
        default: meaning = "\(boundary) failure"
        }
        let prefix = boundary == "Preview" || boundary == "Commit" ? "\(boundary) failure: " : ""
        return "\(prefix)\(meaning) (core \(status))"
    }
}
private final class EditorInput {
    let file: FileHandle
    init(_ file: FileHandle) { self.file = file }
}
private final class EditorContextStorage {
    let pointer = UnsafeMutablePointer<pixaura_editor_context>.allocate(capacity: 1)
    init() { pointer.initialize(to: pixaura_editor_context()) }
    deinit {
        // Also owns failed-initializer cleanup. NativeEditor close joins renders;
        // a failed initializer cannot have any render calls.
        if pointer.pointee.api_version != 0 { _ = pixaura_editor_stop(pointer); _ = pixaura_editor_destroy(pointer) }
        pointer.deinitialize(count: 1); pointer.deallocate()
    }
}
private let editorRead: @convention(c) (UnsafeMutableRawPointer?, UnsafeMutablePointer<UInt8>?, UInt64) -> Int64 = { raw, output, count in
    guard let raw, let output, count <= 65536 else { return -1 }
    do {
        let data = try Unmanaged<EditorInput>.fromOpaque(raw).takeUnretainedValue().file.read(upToCount: Int(count)) ?? Data()
        data.copyBytes(to: output, count: data.count)
        return Int64(data.count)
    } catch { return -1 }
}

/// PRV1's established owner pattern. No task creation, scheduler or shadow
/// history. Lifecycle and actual display replacement share the short-held gate.
/// Render/copy is off-main; stop/join precedes stable native storage destruction.
public final class ManualEditor {
    public struct Request {
        fileprivate let identity: String
        fileprivate var native: pixaura_editor_request
        fileprivate let approved: Bool
        fileprivate var v2State: EditorStateV2? = nil
        fileprivate var original = false
        fileprivate var serial: UInt64 = 0
    }
    public struct Candidate {
        public let request: Request, image: CGImage
        fileprivate var v2Frame: EditorFrameV2? = nil
    }
    private var versioned: EditorContextV2?
    private var runtimeV2: EditorRuntimeV2?
    public final class PreparedAdvisory {
        fileprivate let owner: String, index: Int, native: EditorContextV2.AdvisoryRequest
        fileprivate init(_ owner: String, _ index: Int, _ native: EditorContextV2.AdvisoryRequest) { self.owner = owner; self.index = index; self.native = native }
    }
    private var pendingV2Advisory: PreparedAdvisory?
    private var lastV2Advisory: EditorAdvisoryV2?
    /// Call on the command lane before dispatching optional AI work. Legacy v1
    /// keeps its existing native request protocol and therefore returns nil.
    public func prepareAdvisory(_ index: Int) throws -> PreparedAdvisory? {
        gate.lock(); defer { gate.unlock() }
        try checked(closed ? 13 : 0, "AI request")
        guard let versioned else { return nil }
        guard let choice = UInt32(exactly: index) else { throw ManualEditorFailure(status: 1, boundary: "AI request") }
        let task = PreparedAdvisory(identity, index, try versioned.beginAdvisory(choice))
        pendingV2Advisory = task; lastV2Advisory = nil; return task
    }
    private func runV2Advisory(_ index: UInt32, _ prepared: PreparedAdvisory?) throws -> EditorAdvisoryV2 {
        guard let versioned else { throw ManualEditorFailure(status: 5, boundary: "Versioned AI") }
        gate.lock()
        let task: PreparedAdvisory
        do {
            guard let chosen = try prepared ?? prepareAdvisory(Int(index)) else { throw ManualEditorFailure(status: 5, boundary: "Versioned AI") }
            try checked(chosen.owner == identity && chosen.index == Int(index) ? 0 : 9, "AI request binding")
            task = chosen
        } catch { gate.unlock(); throw error }
        gate.unlock()
        defer { gate.lock(); if pendingV2Advisory === task { pendingV2Advisory = nil }; gate.unlock() }
        let advice = try versioned.runAdvisory(task.native)
        gate.lock(); lastV2Advisory = advice; gate.unlock(); return advice
    }
    private func cancelV2Advisory() throws {
        guard let versioned else { return }
        gate.lock(); let request = pendingV2Advisory, advice = lastV2Advisory
        pendingV2Advisory = nil; lastV2Advisory = nil; gate.unlock()
        func retire(_ action: () throws -> Void) throws {
            do { try action() } catch let error as ManualEditorFailure where error.status == 9 || error.status == 13 { /* Already retired; never cancel unrelated editing. */ }
        }
        if let request { try retire { try versioned.cancelAdvisoryRequest(request.native) } }
        if let advice { try retire { try versioned.cancelAdvisory(advice) } }
    }

    private let storage = EditorContextStorage()
    private var context: UnsafeMutablePointer<pixaura_editor_context> { storage.pointer }
    private let gate = NSRecursiveLock()
    private let idle = NSCondition()
    private let identity = UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased()
    private var closed = false, destroyed = false, stopped = false, acceptingExport = true
    private var renders = 0, agentCalls = 0, assistCalls = 0, exportCalls = 0
    private var applyingReview: AutoReview?
    private var applyingApproval: AgentApproval?
    private var cancellingReview = false
    private var attemptedV2Proposal: String?
    private var attemptedAutoSerial: UInt64?
    private var displayed: CGImage?, committed: CGImage?
    private var comparison: ReviewComparison?
    private var requestedGeneration: UInt64 = 0
    private var previewSerial: UInt64 = 0
    public private(set) var tools: [ManualTool] = []
    private func checked(_ status: Int32, _ boundary: String) throws {
        if status != 0 { throw ManualEditorFailure(status: status, boundary: boundary) }
    }
    /// Read-only native commercial projection; never an admission or Apply grant.
    public func commercialStateV2() throws -> CommercialStateV2? {
        gate.lock(); defer { gate.unlock() }; try checked(closed ? 13 : 0, "Commercial state")
        return try runtimeV2?.commercialState()
    }
    public func recoveryOnlyV2() throws -> Bool {
        gate.lock(); defer { gate.unlock() }; try checked(closed ? 13 : 0, "Recovery state")
        return try versioned?.state().recoveryOnly ?? false
    }
    public convenience init(root: URL, importing input: URL? = nil, capability: AppleCapabilitySnapshot? = nil) throws {
        try self.init(root: root, importing: input, capability: capability, runtimeV2: nil)
    }
    public init(root: URL, importing input: URL? = nil, capability: AppleCapabilitySnapshot? = nil, runtimeV2: EditorRuntimeV2?) throws {
        self.runtimeV2 = runtimeV2
        do {
            let canonical: String = try root.withUnsafeFileSystemRepresentation { p in
                guard let p, let resolved = realpath(p, nil) else { throw ManualEditorFailure(status: 12, boundary: "Open") }
                defer { free(resolved) }; return String(cString: resolved)
            }
            let rootBytes = Array(canonical.utf8), id = Array(identity.utf8)
            let handle = try input.map { try FileHandle(forReadingFrom: $0) }
            defer { try? handle?.close() }
            let reader = handle.map(EditorInput.init)
            let count = try input.map { UInt64(try $0.resourceValues(forKeys: [.fileSizeKey]).fileSize ?? 0) } ?? 0
            let status: Int32
            if let runtimeV2 {
                versioned = try EditorContextV2(runtime: runtimeV2, root: root, importing: input, explicitlyMigrateSchema3: true)
                status = 0
            } else {
            status = rootBytes.withUnsafeBufferPointer { path in id.withUnsafeBufferPointer { seed in
                if var capability {
                    var strategy = pixaura_execution_strategy()
                    return pixaura_editor_open_policy(1, context, path.baseAddress, UInt64(path.count), seed.baseAddress, UInt64(seed.count),
                        reader == nil ? nil : editorRead, reader.map { Unmanaged.passUnretained($0).toOpaque() }, count,
                        &capability.profile, &capability.observation, &strategy)
                }
                return pixaura_editor_open(1, context, path.baseAddress, UInt64(path.count), seed.baseAddress, UInt64(seed.count),
                    reader == nil ? nil : editorRead, reader.map { Unmanaged.passUnretained($0).toOpaque() }, count)
            } }
            }
            try checked(status, "Open")
            var size: UInt64 = 0
            try checkedRegistry(pixaura_manual_registry_json(1, nil, 0, &size))
            var json = Data(count: Int(size))
            try checked(json.withUnsafeMutableBytes { pixaura_manual_registry_json(1, $0.bindMemory(to: UInt8.self).baseAddress, size, &size) }, "Tools")
            struct Registry: Decodable { let tools: [ManualTool] }
            tools = try JSONDecoder().decode(Registry.self, from: json).tools
        } catch {
            close(); throw error
        }
    }
    private func checkedRegistry(_ status: Int32) throws { if status != 11 { try checked(status == 0 ? 14 : status, "Tools") } }
    public func strategy() throws -> AppleExecutionStrategy {
        gate.lock(); defer { gate.unlock() }
        if let runtimeV2 { try checked(closed ? 13 : 0, "Capability"); return try runtimeV2.strategy() }
        var value = pixaura_execution_strategy()
        try checked(closed ? 13 : pixaura_editor_policy_query(1, context, &value), "Capability")
        return AppleExecutionStrategy(value)
    }
    @discardableResult public func applyCapability(_ snapshot: AppleCapabilitySnapshot) throws -> AppleExecutionStrategy {
        gate.lock(); defer { gate.unlock() }
        if let runtimeV2 { try checked(closed ? 13 : 0, "Capability"); return try runtimeV2.updateCapability(snapshot) }
        var observation = snapshot.observation, value = pixaura_execution_strategy()
        try checked(closed ? 13 : pixaura_editor_policy_update(1, context, &observation, &value), "Capability")
        return AppleExecutionStrategy(value)
    }
    public func state() throws -> ManualEditorState {
        gate.lock(); defer { gate.unlock() }
        return try stateLocked()
    }
    private func stateLocked() throws -> ManualEditorState {
        try checked(closed ? 13 : 0, "State")
        if let versioned {
            let value = try versioned.state()
            return ManualEditorState(active: value.selected >= 0, approvalPending: value.pending,
                canUndo: value.canUndo, canRedo: value.canRedo, revisionCount: value.revisionCount,
                revisionID: value.revision, editID: try versioned.gestureIdentity(), selected: value.selected,
                gestureState: value.selected >= 0 ? 1 : 0, values: value.values)
        }
        var size: UInt64 = 0
        try checked(pixaura_editor_state(context, nil, 0, &size), "State")
        var json = Data(count: Int(size))
        try checked(json.withUnsafeMutableBytes { pixaura_editor_state(context, $0.bindMemory(to: UInt8.self).baseAddress, size, &size) }, "State")
        return try JSONDecoder().decode(ManualEditorState.self, from: json)
    }
    public func select(_ index: Int) throws {
        guard let checkedIndex = UInt32(exactly: index) else { throw ManualEditorFailure(status: 5, boundary: "Tool") }
        gate.lock(); defer { gate.unlock() }; displayed = committed; requestedGeneration = 0
        if let versioned { try checked(closed ? 13 : 0, "Tool"); try versioned.select(checkedIndex); return }
        try checked(closed ? 13 : pixaura_editor_select(context, checkedIndex), "Tool")
    }
    public func update(_ values: [Int32], editID: String? = nil) throws {
        gate.lock(); defer { gate.unlock() }
        if let editID, try stateLocked().editID != editID { throw ManualEditorFailure(status: 9, boundary: "Parameter") }
        if let versioned { try checked(closed ? 13 : 0, "Parameter"); try versioned.update(values); requestedGeneration = 0; return }
        try checked(closed ? 13 : values.withUnsafeBufferPointer { pixaura_editor_update(context, $0.baseAddress, UInt32($0.count)) }, "Parameter")
        requestedGeneration = 0
    }
    public func action(_ action: UInt32) throws {
        gate.lock(); defer { gate.unlock() }
        if let versioned {
            try checked(closed ? 13 : 0, "History/cancel")
            switch action { case 0, 4: try versioned.cancel(); displayed = committed
            case 1: _ = try versioned.applyFromHuman()
            case 2: try versioned.undo()
            case 3: try versioned.redo()
            default: throw ManualEditorFailure(status: 1, boundary: "History/cancel") }
            requestedGeneration = 0; return
        }
        let status = closed ? 13 : pixaura_editor_action(context, action)
        if status == 0 || status == 9 { requestedGeneration = 0; if action == 0 || action == 4 || status == 9 { displayed = committed } }
        try checked(status, action == 1 ? "Commit" : "History/cancel")
    }
    public func request() throws -> Request { try request(original: false) }
    public func request(original: Bool) throws -> Request {
        gate.lock(); defer { gate.unlock() }
        let approved = try !stateLocked().active
        if let versioned {
            let state = try versioned.state(); requestedGeneration = state.generation
            try checked(previewSerial == UInt64.max ? 8 : 0, "Preview sequence")
            previewSerial += 1
            return Request(identity: identity, native: pixaura_editor_request(), approved: approved && !original, v2State: state, original: original, serial: previewSerial)
        }
        try checked(original ? 5 : 0, "Original preview")
        var request = pixaura_editor_request()
        try checked(closed ? 13 : pixaura_editor_request_preview(context, &request), "Preview")
        requestedGeneration = request.ticket.generation
        return Request(identity: identity, native: request, approved: approved)
    }
    public func render(_ request: Request) throws -> Candidate? {
        gate.lock()
        guard !closed, request.identity == identity else { gate.unlock(); return nil }
        idle.lock()
        guard renders < 1 else { idle.unlock(); gate.unlock(); return nil }
        renders += 1; idle.unlock(); gate.unlock()
        defer { idle.lock(); renders -= 1; idle.broadcast(); idle.unlock() }
        if let snapshot = request.v2State, let versioned {
            let state = try versioned.state()
            try checked(state.epoch == snapshot.epoch && state.generation == snapshot.generation ? 0 : 9, "Preview")
            let frame = try versioned.preview(original: request.original)
            let image = try frame.image(); let current = try versioned.state()
            // Native frame_copy waits for and validates the exact binding.
            // isCurrent is a nonblocking UI probe: metadata contention alone
            // returns false and must not discard the sole worker-produced image.
            guard current.epoch == snapshot.epoch, current.generation == snapshot.generation else { return nil }
            return Candidate(request: request, image: image, v2Frame: frame)
        }
        var request = request, bytes = Data(count: 512 * 512 * 4), width: UInt32 = 0, height: UInt32 = 0
        try checked(bytes.withUnsafeMutableBytes {
            pixaura_editor_render(context, &request.native, 512, 512, $0.bindMemory(to: UInt8.self).baseAddress, UInt64($0.count), &width, &height)
        }, "Preview")
        bytes.count = Int(width) * Int(height) * 4
        guard let provider = CGDataProvider(data: bytes as CFData), let space = CGColorSpace(name: CGColorSpace.sRGB),
              let image = CGImage(width: Int(width), height: Int(height), bitsPerComponent: 8, bitsPerPixel: 32,
                bytesPerRow: Int(width) * 4, space: space,
                bitmapInfo: CGBitmapInfo.byteOrder32Big.union(CGBitmapInfo(rawValue: CGImageAlphaInfo.last.rawValue)),
                provider: provider, decode: nil, shouldInterpolate: false, intent: .defaultIntent) else {
            throw ManualEditorFailure(status: 8, boundary: "Preview")
        }
        return Candidate(request: request, image: image)
    }
    public enum PreviewInstallation: Equatable { case installed, busy, discarded }
    @discardableResult public func install(_ candidate: Candidate, replace: (CGImage) -> Void = { _ in }) -> Bool {
        installStatus(candidate, replace: replace) == .installed
    }
    public func installStatus(_ candidate: Candidate, replace: (CGImage) -> Void = { _ in }) -> PreviewInstallation {
        guard gate.try() else { return .busy }; defer { gate.unlock() }
        if let frame = candidate.v2Frame {
            guard !closed, candidate.request.identity == identity, candidate.request.serial == previewSerial else { return .discarded }
            switch frame.currency {
            case .busy: return .busy
            case .current: break
            case .stale, .unavailable: return .discarded
            }
            displayed = candidate.image; if candidate.request.approved { committed = candidate.image }
            replace(candidate.image); return .installed
        }
        var request = candidate.request.native
        guard !closed, candidate.request.identity == identity, pixaura_editor_current(context, &request) == 0 else { return .discarded }
        displayed = candidate.image
        if candidate.request.approved { committed = candidate.image }
        replace(candidate.image)
        return .installed
    }
    public func displayedImage() -> CGImage? { gate.lock(); defer { gate.unlock() }; return displayed }
    public func isRequested(_ request: Request) -> Bool {
        gate.lock(); defer { gate.unlock() }
        return !closed && request.identity == identity && (request.v2State?.generation ?? request.native.ticket.generation) == requestedGeneration && (request.v2State == nil || request.serial == previewSerial)
    }
    public struct AgentPlan {
        public let canonicalBytes: Data
        fileprivate init(_ bytes: Data) { canonicalBytes = bytes }
    }
    public final class AgentApproval {
        fileprivate let owner: ManualEditor
        fileprivate var token = pixaura_agent_approval()
        fileprivate var owned = false
        fileprivate init(_ owner: ManualEditor) { self.owner = owner }
        public func close() { owner.releaseAgentApproval(self) }
        deinit { close() }
    }
    public struct AgentReceipt {
        public let planSHA256: String, sourceRevision: String, resultingRevision: String
        fileprivate var v2Publication: NativePublicationV2? = nil
    }
    private func agentRequest() -> pixaura_agent_execution_request {
        var request = pixaura_agent_execution_request()
        request.api_version = 1; request.struct_size = UInt32(MemoryLayout.size(ofValue: request)); request.registry_version = 1
        return request
    }
    private func agentBytes(_ mode: Int, _ input: Data = Data()) throws -> Data {
        gate.lock(); defer { gate.unlock() }; try checked(closed ? 13 : 0, "Agent")
        return try agentBytesLocked(mode, input)
    }
    private func agentBytesLocked(_ mode: Int, _ input: Data) throws -> Data {
        if let versioned { return try versioned.agentBytes(mode: UInt32(mode), input: input) }
        var output = Data(count: 8192), size: UInt64 = 0, request = agentRequest()
        let status = output.withUnsafeMutableBytes { out in input.withUnsafeBytes { bytes in
            let source = bytes.bindMemory(to: UInt8.self).baseAddress
            let destination = out.bindMemory(to: UInt8.self).baseAddress
            if mode == 1 { return pixaura_editor_agent_binding(1, context, destination, 8192, &size) }
            if mode == 2 { return pixaura_editor_agent_validate(1, context, &request, source, UInt64(bytes.count), destination, 8192, &size) }
            var planner = pixaura_reference_planner_request()
            planner.api_version = 1; planner.struct_size = UInt32(MemoryLayout.size(ofValue: planner))
            planner.planner_version = 1; planner.registry_version = 1
            return pixaura_editor_agent_plan(1, context, &planner, source, UInt64(bytes.count), destination, 8192, &size)
        } }
        try checked(status, "Agent"); try checked(size > 0 && size <= 8192 ? 0 : 14, "Agent")
        output.count = Int(size); return output
    }
    public func agentBinding() throws -> Data { try agentBytes(1) }
    public func referenceAgentPlan(_ canonicalStructuredIntent: Data) throws -> AgentPlan { AgentPlan(try agentBytes(3, canonicalStructuredIntent)) }
    public func validateAgentPlan(_ canonicalPlan: Data) throws -> AgentPlan { AgentPlan(try agentBytes(2, canonicalPlan)) }
    /// Trusted application confirmation only. Never invoked by planning/validation.
    public func approveAgentPlanFromHuman(_ plan: AgentPlan) throws -> AgentApproval {
        gate.lock(); defer { gate.unlock() }; try checked(closed ? 13 : 0, "Agent approval")
        return try approveAgentPlanLocked(plan)
    }
    private func approveAgentPlanLocked(_ plan: AgentPlan) throws -> AgentApproval {
        try checked(versioned == nil ? 0 : 5, "Legacy agent approval")
        // Construct the owning ARC object only after native approval succeeds.
        // A rejected call must not deinit an unowned object while this NSLock
        // is held (its close path reacquires the same owner gate).
        var token = pixaura_agent_approval(); var request = agentRequest()
        try checked(plan.canonicalBytes.withUnsafeBytes {
            pixaura_editor_agent_approve(1, context, &request, $0.bindMemory(to: UInt8.self).baseAddress, UInt64($0.count), &token)
        }, "Agent approval")
        let approval = AgentApproval(self); approval.token = token; approval.owned = true; return approval
    }
    public func revokeAgentApproval(_ approval: AgentApproval) throws {
        gate.lock(); defer { gate.unlock() }
        try checked(versioned == nil ? 0 : 5, "Legacy agent revocation")
        try checked(closed ? 13 : pixaura_editor_agent_revoke(1, context, &approval.token), "Agent revocation")
    }
    private func releaseAgentApproval(_ approval: AgentApproval) {
        gate.lock(); defer { gate.unlock() }
        if approval.owned {
            if !closed && versioned == nil { _ = pixaura_editor_agent_release(1, context, &approval.token) }
            approval.owned = false
        }
    }
    public func executeAgentPlan(_ plan: AgentPlan, approval: AgentApproval? = nil) throws -> AgentReceipt {
        gate.lock()
        if closed || versioned != nil { gate.unlock(); throw ManualEditorFailure(status: closed ? 13 : 15, boundary: "Agent execution") }
        idle.lock()
        if agentCalls >= 8 { idle.unlock(); gate.unlock(); throw ManualEditorFailure(status: 8, boundary: "Agent execution") }
        agentCalls += 1; idle.unlock(); gate.unlock()
        defer { idle.lock(); agentCalls -= 1; idle.broadcast(); idle.unlock() }
        var request = agentRequest(), receipt = pixaura_agent_execution_receipt()
        let status = plan.canonicalBytes.withUnsafeBytes { bytes -> Int32 in
            if let approval {
                // Immutable after approval; release/revoke never writes the token.
                var token = approval.token
                return pixaura_editor_agent_execute(1, context, &request, &token, bytes.bindMemory(to: UInt8.self).baseAddress, UInt64(bytes.count), &receipt)
            }
            return pixaura_editor_agent_execute(1, context, &request, nil, bytes.bindMemory(to: UInt8.self).baseAddress, UInt64(bytes.count), &receipt)
        }
        try checked(status, "Agent execution")
        gate.lock(); requestedGeneration = 0; gate.unlock()
        return try decodeAgentReceipt(receipt)
    }
    func decodeAgentReceipt(_ receipt: pixaura_agent_execution_receipt) throws -> AgentReceipt {
        try checked(receipt.api_version == 1 && receipt.struct_size == 144 && receipt.approval_consumed == 1 && receipt.reserved == 0 ? 0 : 14, "Agent receipt")
        func text<T>(_ value: T) -> String { withUnsafeBytes(of: value) { String(decoding: $0, as: UTF8.self) } }
        let result = AgentReceipt(planSHA256: text(receipt.plan_sha256), sourceRevision: text(receipt.source_revision), resultingRevision: text(receipt.resulting_revision))
        try checked([result.planSHA256, result.sourceRevision, result.resultingRevision].allSatisfy { $0.utf8.allSatisfy { (48...57).contains($0) || (97...102).contains($0) } } && result.sourceRevision != result.resultingRevision ? 0 : 14, "Agent receipt")
        return result
    }
    public func cancelAgentExecution() throws {
        gate.lock(); defer { gate.unlock() }
        try checked(versioned == nil ? 0 : 5, "Legacy agent cancellation")
        try checked(closed ? 13 : pixaura_editor_agent_cancel(1, context), "Agent cancellation")
    }
    public struct AssistChoice {
        public let index: Int, toolIndex: Int, values: [Int32]
    }
    public struct AssistAdvice {
        fileprivate let owner: String, packet: pixaura_assist_advice
        public let choice: AssistChoice
        fileprivate var v2Advice: EditorAdvisoryV2? = nil
    }
    private func assistChoice(_ value: pixaura_assist_advice) throws -> AssistChoice {
        try checked(value.api_version == 1 && value.struct_size == UInt32(MemoryLayout<pixaura_assist_advice>.size) && value.parameter_count > 0 && value.parameter_count <= 4 ? 0 : 14, "Advice")
        let values = [value.values.0, value.values.1, value.values.2, value.values.3]
        return AssistChoice(index: Int(value.choice_index), toolIndex: Int(value.tool_index), values: Array(values.prefix(Int(value.parameter_count))))
    }
    public func adviceChoices() throws -> [AssistChoice] {
        gate.lock(); defer { gate.unlock() }; try checked(closed ? 13 : 0, "Advice")
        return try (0..<Int(PIXAURA_ASSIST_CHOICES)).map { index in
            var value = pixaura_assist_advice(); try checked(pixaura_assist_choice(1, UInt32(index), &value), "Advice")
            return try assistChoice(value)
        }
    }
    public func requestAdvice(_ index: Int) throws -> AssistAdvice { try requestAdvice(index, prepared: nil) }
    public func requestAdvice(_ index: Int, prepared: PreparedAdvisory?) throws -> AssistAdvice {
        guard let index = UInt32(exactly: index) else { throw ManualEditorFailure(status: 1, boundary: "Advice") }
        gate.lock(); idle.lock()
        if closed || assistCalls != 0 || applyingReview != nil || cancellingReview { let code: Int32 = closed ? 13 : 8; idle.unlock(); gate.unlock(); throw ManualEditorFailure(status: code, boundary: "Advice") }
        assistCalls += 1; idle.unlock(); gate.unlock()
        defer { idle.lock(); assistCalls -= 1; idle.broadcast(); idle.unlock() }
        if versioned != nil {
            let advice = try runV2Advisory(index, prepared)
            let projection = try v2Proposal(advice)
            guard let action = projection.actions.first else { throw ManualEditorFailure(status: 14, boundary: "Advice") }
            return AssistAdvice(owner: identity, packet: pixaura_assist_advice(), choice: AssistChoice(index: Int(index), toolIndex: action.toolIndex, values: action.values), v2Advice: advice)
        }
        var request = pixaura_assist_request(); request.api_version = 1; request.struct_size = UInt32(MemoryLayout.size(ofValue: request)); request.choice_index = index
        var value = pixaura_assist_advice()
        try checked(pixaura_editor_assist_request(1, context, &request, &value), "Advice")
        return AssistAdvice(owner: identity, packet: value, choice: try assistChoice(value))
    }
    @discardableResult public func inspectAdvice(_ advice: AssistAdvice) throws -> AssistChoice {
        gate.lock(); defer { gate.unlock() }
        if let native = advice.v2Advice, let versioned {
            try checked(closed ? 13 : advice.owner != identity ? 9 : 0, "Advice")
            _ = try versioned.advisoryContent(native, operations: false); return advice.choice
        }
        var value = advice.packet
        try checked(closed ? 13 : advice.owner != identity ? 9 : pixaura_editor_assist_current(1, context, &value), "Advice")
        return advice.choice
    }
    public func cancelAdvice() throws {
        if versioned != nil { try cancelV2Advisory(); return }
        gate.lock(); defer { gate.unlock() }; try checked(closed ? 13 : pixaura_editor_assist_cancel(1, context), "Advice")
    }
    public struct AutoProposal {
        fileprivate let owner: String, packet: pixaura_auto_proposal
        public let actions: [AssistChoice], proposalID: String, providerID: String
        fileprivate var v2Advice: EditorAdvisoryV2? = nil
    }
    public func requestAuto(_ index: Int) throws -> AutoProposal { try requestAuto(index, prepared: nil) }
    public func requestAuto(_ index: Int, prepared: PreparedAdvisory?) throws -> AutoProposal {
        guard let index = UInt32(exactly: index) else { throw ManualEditorFailure(status: 1, boundary: "Auto proposal") }
        gate.lock(); idle.lock()
        if closed || assistCalls != 0 || applyingReview != nil || cancellingReview { let code: Int32 = closed ? 13 : 8; idle.unlock(); gate.unlock(); throw ManualEditorFailure(status: code, boundary: "Auto proposal") }
        assistCalls += 1; idle.unlock(); gate.unlock()
        defer { idle.lock(); assistCalls -= 1; idle.broadcast(); idle.unlock() }
        if versioned != nil { return try v2Proposal(runV2Advisory(index, prepared)) }
        var request = pixaura_auto_request(); request.api_version = 1; request.struct_size = UInt32(MemoryLayout.size(ofValue: request)); request.choice_index = index; request.kind = 2
        var value = pixaura_auto_proposal()
        try checked(pixaura_editor_auto_request(1, context, &request, nil, 0, &value), "Auto proposal")
        try checked(value.api_version == 1 && value.struct_size == 328 && value.action_count > 0 && value.action_count <= 8 ? 0 : 14, "Auto proposal")
        let actions: [AssistChoice] = try withUnsafeBytes(of: value.actions) { raw in
            try raw.bindMemory(to: pixaura_auto_action.self).prefix(Int(value.action_count)).enumerated().map { i, action in
                try checked(action.parameter_count > 0 && action.parameter_count <= 4 ? 0 : 14, "Auto proposal")
                return AssistChoice(index: i, toolIndex: Int(action.tool_index), values: Array([action.values.0, action.values.1, action.values.2, action.values.3].prefix(Int(action.parameter_count))))
            }
        }
        func id<T>(_ bytes: T) -> String { withUnsafeBytes(of: bytes) { String(decoding: $0.prefix { $0 != 0 }, as: UTF8.self) } }
        return AutoProposal(owner: identity, packet: value, actions: actions, proposalID: id(value.proposal_id), providerID: id(value.provider_id))
    }
    public func inspectAuto(_ proposal: AutoProposal) throws -> Data {
        gate.lock(); defer { gate.unlock() }
        return try inspectAutoLocked(proposal)
    }
    private func inspectAutoLocked(_ proposal: AutoProposal) throws -> Data {
        try checked(closed ? 13 : proposal.owner != identity ? 9 : 0, "Auto proposal")
        if let native = proposal.v2Advice, let versioned { return try versioned.advisoryContent(native, operations: false) }
        var value = proposal.packet, required: UInt64 = 0
        let query = pixaura_editor_auto_plan(1, context, &value, nil, 0, &required)
        try checked(query == 11 ? 0 : query == 0 ? 14 : query, "Auto proposal")
        try checked(required > 0 && required <= 8192 ? 0 : 14, "Auto proposal")
        var bytes = Data(count: Int(required)); let capacity = required
        let status = bytes.withUnsafeMutableBytes { pixaura_editor_auto_plan(1, context, &value, $0.bindMemory(to: UInt8.self).baseAddress, capacity, &required) }
        try checked(status, "Auto proposal"); return bytes
    }
    public func cancelAuto() throws { if versioned != nil { try cancelV2Advisory(); return }; gate.lock(); defer { gate.unlock() }; try checked(closed ? 13 : pixaura_editor_auto_cancel(1, context), "Auto proposal") }
    public struct ReviewParameter {
        public let name: String, unit: String
        public let raw: Int32, minimum: Int32, maximum: Int32, neutral: Int32
    }
    public struct ReviewAction {
        public let index: Int, toolID: String, label: String
        public let operationVersion: UInt32, parameterVersion: UInt32
        public let geometry: Bool, parameters: [ReviewParameter]
    }
    public struct PlanInspection {
        fileprivate let packet: pixaura_review_inspection
        public let actions: [ReviewAction], warnings: UInt32
    }
    private func describeReviewLocked(_ proposal: AutoProposal, _ plan: AgentPlan) throws -> PlanInspection {
        var p = pixaura_review_inspection(), source = proposal.packet
        try checked(plan.canonicalBytes.withUnsafeBytes { pixaura_review_inspect(1, context, &source, $0.bindMemory(to: UInt8.self).baseAddress, UInt64($0.count), &p) }, "Plan inspection")
        try checked(p.api_version == 1 && p.struct_size == 4808 && p.kind == 0x52494e31 && (1...8).contains(p.action_count) && p.registry_version == 1 && p.warnings <= 3 ? 0 : 14, "Plan inspection")
        func text<T>(_ value: T) throws -> String {
            try withUnsafeBytes(of: value) { raw in
                guard let n = raw.firstIndex(of: 0), n > 0, raw.prefix(n).allSatisfy({ $0 >= 32 && $0 <= 126 }) else { throw ManualEditorFailure(status: 14, boundary: "Inspection text") }
                return String(decoding: raw.prefix(n), as: UTF8.self)
            }
        }
        let count = Int(p.action_count)
        let actions: [ReviewAction] = try withUnsafeBytes(of: p.actions) { raw in
            try raw.bindMemory(to: pixaura_review_action.self).prefix(count).enumerated().map { i, a in
                try checked(a.index == UInt32(i) && (1...4).contains(a.parameter_count) && a.operation_version == 1 && a.parameter_version == 1 && a.geometry_changed <= 1 && (a.warnings == 0 || a.warnings == 2) && a.reserved == 0 ? 0 : 14, "Inspection action")
                let parameters: [ReviewParameter] = try withUnsafeBytes(of: a.parameters) { values in
                    try values.bindMemory(to: pixaura_review_parameter.self).prefix(Int(a.parameter_count)).map { v in
                        ReviewParameter(name: try text(v.name), unit: try text(v.unit), raw: v.raw_value, minimum: v.minimum, maximum: v.maximum, neutral: v.neutral)
                    }
                }
                return ReviewAction(index: i, toolID: try text(a.tool_id), label: try text(a.label), operationVersion: a.operation_version, parameterVersion: a.parameter_version, geometry: a.geometry_changed == 1, parameters: parameters)
            }
        }
        return PlanInspection(packet: p, actions: actions, warnings: p.warnings)
    }
    public final class AutoReview {
        fileprivate let owner: String, proposal: AutoProposal, plan: AgentPlan
        public let sourceRevision: String
        public let reviewIdentity: String
        public let inspection: PlanInspection
        public var canonicalBytes: Data { plan.canonicalBytes }
        fileprivate var attempted = false, abandoned = false
        fileprivate init(_ owner: String, _ proposal: AutoProposal, _ plan: AgentPlan, _ source: String, _ inspection: PlanInspection) {
            self.owner = owner; self.proposal = proposal; self.plan = plan; sourceRevision = source; self.inspection = inspection
            let context = withUnsafeBytes(of: proposal.packet.context_id) { String(decoding: $0, as: UTF8.self) }
            reviewIdentity = proposal.v2Advice.map { "v2review:" + $0.id } ?? ("review:" + context + ":" + String(proposal.packet.request_serial))
        }
    }
    public func prepareAutoReview(_ proposal: AutoProposal) throws -> AutoReview {
        gate.lock(); defer { gate.unlock() }
        try checked(closed ? 13 : proposal.owner != identity ? 9 : 0, "Auto review")
        try checked(applyingReview != nil || cancellingReview ? 8 : 0, "Auto review")
        if proposal.v2Advice != nil { try checked(attemptedV2Proposal == proposal.proposalID ? 15 : 0, "Auto review"); return try v2Review(proposal) }
        try checked(attemptedAutoSerial == proposal.packet.request_serial ? 15 : 0, "Auto review")
        let plan = AgentPlan(try agentBytesLocked(2, inspectAutoLocked(proposal)))
        return AutoReview(identity, proposal, plan, try stateLocked().revisionID, try describeReviewLocked(proposal, plan))
    }
    public func inspectReview(_ review: AutoReview) throws -> PlanInspection {
        gate.lock(); defer { gate.unlock() }
        if review.proposal.v2Advice != nil {
            try checked(closed ? 13 : review.owner != identity || review.attempted || review.abandoned ? 9 : 0, "Plan inspection current")
            try checked(try inspectAutoLocked(review.proposal) == review.canonicalBytes ? 0 : 9, "Plan inspection current")
            return review.inspection
        }
        var packet = review.inspection.packet
        try checked(closed ? 13 : review.owner != identity || review.attempted || review.abandoned ? 9 : pixaura_review_inspection_current(1, context, &packet), "Plan inspection current")
        return review.inspection
    }
    public final class ReviewEditSession {
        fileprivate let owner: String
        fileprivate let lock = NSRecursiveLock()
        fileprivate var packet: pixaura_review_edit_result
        private var currentReview: AutoReview?
        fileprivate var nativeAdvice: EditorAdvisoryV2?
        fileprivate var nativeState: UInt32 = 1
        public fileprivate(set) var review: AutoReview? {
            get { lock.lock(); defer { lock.unlock() }; return currentReview }
            set { lock.lock(); defer { lock.unlock() }; currentReview = newValue }
        }
        public var state: UInt32 { lock.lock(); defer { lock.unlock() }; if nativeAdvice != nil { return nativeState }; return currentReview == nil && packet.state != 3 ? 4 : packet.state }
        public var bindingIdentity: String {
            lock.lock(); defer { lock.unlock() }
            if let currentReview { return currentReview.reviewIdentity }
            if let nativeAdvice { return "v2edit:" + nativeAdvice.id + ":" + String(nativeState) }
            let context = withUnsafeBytes(of: packet.ticket.context_id) { String(decoding: $0, as: UTF8.self) }
            return "edit:" + context + ":" + String(packet.ticket.serial) + ":" + String(state)
        }
        fileprivate init(_ owner: String, _ packet: pixaura_review_edit_result, _ review: AutoReview) { self.owner = owner; self.packet = packet; currentReview = review }
    }
    public func beginReviewEditing(_ review: AutoReview) throws -> ReviewEditSession {
        guard gate.try() else { throw ManualEditorFailure(status: 8, boundary: "Review edit busy") }; defer { gate.unlock() }
        _ = try inspectReview(review)
        idle.lock(); let busy = assistCalls != 0; idle.unlock()
        try checked(busy || applyingReview != nil || cancellingReview ? 8 : 0, "Review edit")
        if let advice = review.proposal.v2Advice {
            let session = ReviewEditSession(identity, pixaura_review_edit_result(), review)
            session.nativeAdvice = advice; return session
        }
        var description = review.inspection.packet, result = pixaura_review_edit_result()
        try checked(pixaura_review_edit_begin(1, context, &description, &result), "Review edit begin")
        try checked(result.api_version == 1 && result.struct_size == 480 && result.state == 1 ? 0 : 14, "Review edit result")
        return ReviewEditSession(identity, result, review)
    }
    public func editReview(_ session: ReviewEditSession, expectedIdentity: String, operation: UInt32, action: UInt32 = 0,
        parameter: UInt32 = 0, value: Int32 = 0, destination: UInt32 = 0) throws -> AutoReview? {
        guard gate.try() else { throw ManualEditorFailure(status: 8, boundary: "Review edit busy") }; defer { gate.unlock() }
        idle.lock(); let busy = assistCalls != 0; idle.unlock()
        try checked(closed ? 13 : session.owner != identity ? 9 : busy || applyingReview != nil || cancellingReview ? 8 : 0, "Review edit")
        session.lock.lock(); defer { session.lock.unlock() }
        try checked(session.bindingIdentity == expectedIdentity ? 0 : 9, "Review edit binding")
        let old = session.review
        try checked(old?.attempted == true || old?.abandoned == true ? 9 : 0, "Review edit")
        if let versioned, let advice = session.nativeAdvice {
            do {
                let next = try versioned.editReviewCommand(advice, operation: operation, action: action, parameter: parameter, value: value, destination: destination)
                old?.abandoned = true; session.review = nil; comparison = nil; session.nativeAdvice = next; lastV2Advisory = next
                let inspection = try versioned.inspectReview(next)
                if inspection.actions.isEmpty { session.nativeState = 3; return nil }
                let review = try v2Review(v2Proposal(next)); session.review = review
                session.nativeState = operation == 4 ? 1 : 2; return review
            } catch {
                old?.abandoned = true; session.review = nil; session.nativeState = 4; comparison = nil
                throw error
            }
        }
        var request = pixaura_review_edit_request(); request.api_version = 1; request.struct_size = UInt32(MemoryLayout.size(ofValue: request))
        request.operation = operation; request.ticket = session.packet.ticket; request.action_index = action; request.parameter_index = parameter; request.destination = destination; request.raw_value = value
        if operation != 4, let old, Int(action) < old.inspection.actions.count {
            let target = withUnsafeBytes(of: old.inspection.packet.actions) { $0.bindMemory(to: pixaura_review_action.self)[Int(action)] }
            request.tool_id = target.tool_id
            request.expected_action = target
            if operation == 1, parameter < target.parameter_count {
                request.parameter_name = withUnsafeBytes(of: target.parameters) { $0.bindMemory(to: pixaura_review_parameter.self)[Int(parameter)].name }
            }
        }
        var result = pixaura_review_edit_result()
        let status = pixaura_review_edit(1, context, &request, &result)
        if status != 0 {
            var ticket = session.packet.ticket
            old?.abandoned = true; session.review = nil; comparison = nil
            if pixaura_review_edit_current(1, context, &ticket, &result) == 0 && result.api_version == 1 && result.struct_size == 480 && (1...4).contains(result.state) {
                session.packet = result
            }
            try checked(status, "Review edit")
        }
        old?.abandoned = true; session.review = nil; comparison = nil
        try checked(result.api_version == 1 && result.struct_size == 480 && (1...3).contains(result.state) ? 0 : 14, "Review edit result")
        session.packet = result
        if result.state == 3 { return nil }
        let packet = result.review
        try checked(packet.api_version == 1 && packet.struct_size == 328 && (1...8).contains(packet.action_count) ? 0 : 14, "Edited review")
        let actions: [AssistChoice] = try withUnsafeBytes(of: packet.actions) { raw in
            try raw.bindMemory(to: pixaura_auto_action.self).prefix(Int(packet.action_count)).enumerated().map { i, a in
                try checked((1...4).contains(a.parameter_count) ? 0 : 14, "Edited review")
                return AssistChoice(index: i, toolIndex: Int(a.tool_index), values: Array([a.values.0, a.values.1, a.values.2, a.values.3].prefix(Int(a.parameter_count))))
            }
        }
        func text<T>(_ value: T) -> String { withUnsafeBytes(of: value) { String(decoding: $0.prefix { $0 != 0 }, as: UTF8.self) } }
        let proposal = AutoProposal(owner: identity, packet: packet, actions: actions, proposalID: text(packet.proposal_id), providerID: text(packet.provider_id))
        let review = try prepareAutoReview(proposal); session.review = review; return review
    }
    /// ONLY explicit review Apply may call this trusted application boundary.
    public func applyAutoFromHuman(_ review: AutoReview, onExecuting: () -> Void = {}) throws -> AgentReceipt {
        if let versioned, let native = review.proposal.v2Advice {
            gate.lock()
            do {
                try checked(closed ? 13 : review.owner != identity ? 9 : review.attempted || review.abandoned ? 15 : applyingReview != nil || cancellingReview ? 8 : 0, "Auto Apply")
                idle.lock(); let busy = assistCalls != 0; idle.unlock()
                try checked(busy ? 8 : 0, "Auto Apply")
                try checked(inspectAutoLocked(review.proposal) == review.canonicalBytes ? 0 : 9, "Auto review")
                try checked(attemptedV2Proposal == review.proposal.proposalID ? 15 : 0, "Auto Apply")
                attemptedV2Proposal = review.proposal.proposalID; review.attempted = true; applyingReview = review
                idle.lock(); agentCalls += 1; idle.unlock()
            } catch { gate.unlock(); throw error }
            gate.unlock()
            defer {
                gate.lock(); applyingReview = nil; gate.unlock()
                idle.lock(); agentCalls -= 1; idle.broadcast(); idle.unlock()
            }
            onExecuting()
            gate.lock(); let denied: Int32 = closed ? 13 : review.abandoned ? 15 : 0; gate.unlock()
            try checked(denied, "Auto Apply")
            let publication = try versioned.applyAdvisoryReceiptFromHuman(native)
            try checked(publication.parent == review.sourceRevision ? 0 : 14, "Auto receipt")
            let digest = SHA256.hash(data: review.canonicalBytes).map { String(format: "%02x", $0) }.joined()
            return AgentReceipt(planSHA256: digest, sourceRevision: publication.parent,
                resultingRevision: publication.revision, v2Publication: publication)
        }
        var approval: AgentApproval?
        gate.lock()
        do {
            try checked(closed ? 13 : review.owner != identity ? 9 : review.attempted || review.abandoned ? 15 : applyingReview != nil || cancellingReview ? 8 : 0, "Auto Apply")
            idle.lock(); let busy = assistCalls != 0; idle.unlock()
            try checked(busy ? 8 : 0, "Auto Apply")
            try checked(inspectAutoLocked(review.proposal) == review.plan.canonicalBytes ? 0 : 9, "Auto review")
            try checked(agentBytesLocked(2, review.plan.canonicalBytes) == review.plan.canonicalBytes ? 0 : 9, "Auto review")
            try checked(attemptedAutoSerial == review.proposal.packet.request_serial ? 15 : 0, "Auto Apply")
            review.attempted = true; attemptedAutoSerial = review.proposal.packet.request_serial
            approval = try approveAgentPlanLocked(review.plan); applyingApproval = approval
            applyingReview = review; idle.lock(); agentCalls += 1; idle.unlock()
        } catch { gate.unlock(); throw error }
        gate.unlock()
        defer {
            approval?.close()
            gate.lock(); applyingApproval = nil; applyingReview = nil; gate.unlock()
            idle.lock(); agentCalls -= 1; idle.broadcast(); idle.unlock()
        }
        onExecuting() // transport notification only; carries no approval issuer
        let result = try executeAgentPlan(review.plan, approval: approval)
        try checked(result.sourceRevision == review.sourceRevision ? 0 : 14, "Auto receipt")
        return result
    }
    public func cancelAutoReview(_ review: AutoReview) throws {
        gate.lock()
        do { try checked(closed ? 13 : review.owner != identity ? 9 : 0, "Auto cancellation") }
        catch { gate.unlock(); throw error }
        review.abandoned = true
        if let versioned, let native = review.proposal.v2Advice {
            gate.unlock()
            try versioned.cancelAdvisory(native)
            return
        }
        guard applyingReview === review, !cancellingReview else { gate.unlock(); return }
        cancellingReview = true; let approval = applyingApproval
        idle.lock(); agentCalls += 1; idle.unlock(); gate.unlock()
        defer {
            gate.lock(); cancellingReview = false; gate.unlock()
            idle.lock(); agentCalls -= 1; idle.broadcast(); idle.unlock()
        }
        // Do not hold the wrapper gate while native revocation waits for admission.
        // Revoke BEFORE cancel closes the pre-execution cancellation gap.
        if let approval { var token = approval.token; _ = pixaura_editor_agent_revoke(1, context, &token) }
        try cancelAgentExecution()
    }
    public struct ResultReview {
        public let state, count: UInt32
        public let documentID, previous, result, live: String
        public let geometryChanged, canUndo, canRedo, publicationVisible, hasPrevious: Bool
        fileprivate let packet: pixaura_result_review
        fileprivate var v2Owner: String? = nil
        fileprivate var v2Epoch: UInt64 = 0
        fileprivate init(history: EditorHistoryV2, native: EditorStateV2, receipt: NativePublicationV2?, owner: String) throws {
            packet = pixaura_result_review(); v2Owner = owner; v2Epoch = history.epoch
            guard history.epoch == native.epoch, history.current == native.revision,
                  let liveRevision = history.revisions.first(where: { $0.id == history.current }) else { throw ManualEditorFailure(status: 9, boundary: "Result history") }
            documentID = history.documentID; live = history.current; count = UInt32(history.revisions.count)
            canUndo = native.selected < 0 && native.canUndo; canRedo = native.selected < 0 && native.canRedo
            if let receipt {
                guard receipt.document == history.documentID, receipt.source == history.sourceSHA256 else { throw ManualEditorFailure(status: 9, boundary: "Result source") }
                if let published = history.revisions.first(where: { $0.id == receipt.revision }) {
                    publicationVisible = true; result = published.id; previous = published.parentID ?? ""
                    hasPrevious = published.parentID != nil
                    state = published.parentID == receipt.parent && history.current == published.id ? 1 : 2
                    if let parent = published.parentID, let before = history.revisions.first(where: { $0.id == parent }) {
                        geometryChanged = before.geometrySHA256 != published.geometrySHA256
                    } else { geometryChanged = false }
                } else {
                    state = 4; publicationVisible = false; result = ""; previous = liveRevision.parentID ?? ""
                    hasPrevious = liveRevision.parentID != nil; geometryChanged = false
                }
            } else {
                state = count > 1 ? 3 : 0; publicationVisible = false; result = ""; previous = liveRevision.parentID ?? ""
                hasPrevious = liveRevision.parentID != nil; geometryChanged = false
            }
        }
        fileprivate init(_ packet: pixaura_result_review) {
            self.packet = packet; state = packet.state; count = packet.revision_count
            geometryChanged = packet.geometry_changed != 0
            hasPrevious = packet.flags & 1 != 0; canUndo = packet.flags & 2 != 0
            canRedo = packet.flags & 4 != 0; publicationVisible = packet.flags & 8 != 0
            func text(_ value: UnsafeRawBufferPointer) -> String { String(decoding: Data(value.prefix { $0 != 0 }), as: UTF8.self) }
            var copy = packet
            documentID = withUnsafeBytes(of: &copy.document_id, text)
            previous = withUnsafeBytes(of: &copy.previous_revision, text)
            result = withUnsafeBytes(of: &copy.result_revision, text)
            live = withUnsafeBytes(of: &copy.live_revision, text)
        }
    }
    private func nativeReceipt(_ receipt: AgentReceipt) -> pixaura_agent_execution_receipt {
        var raw = pixaura_agent_execution_receipt()
        withUnsafeMutableBytes(of: &raw) { buffer in
            buffer.storeBytes(of: UInt32(1), toByteOffset: 0, as: UInt32.self)
            buffer.storeBytes(of: UInt32(MemoryLayout<pixaura_agent_execution_receipt>.size), toByteOffset: 4, as: UInt32.self)
            buffer.storeBytes(of: UInt32(1), toByteOffset: 8, as: UInt32.self)
            func copy(_ text: String, _ offset: Int) { Array(text.utf8).withUnsafeBytes { buffer.baseAddress!.advanced(by: offset).copyMemory(from: $0.baseAddress!, byteCount: $0.count) } }
            copy(receipt.planSHA256, 16); copy(receipt.sourceRevision, 80); copy(receipt.resultingRevision, 112)
        }
        return raw
    }
    /// Reads ordinary history. A nil receipt reports history without claiming Apply success.
    public func resultReview(_ receipt: AgentReceipt?) throws -> ResultReview {
        gate.lock(); defer { gate.unlock() }
        try checked(closed ? 13 : 0, "Result review")
        if let versioned {
            if let receipt { try checked(receipt.v2Publication != nil ? 0 : 9, "Result receipt") }
            return try ResultReview(history: versioned.history(), native: versioned.state(), receipt: receipt?.v2Publication, owner: identity)
        }
        var out = pixaura_result_review()
        if let receipt { var native = nativeReceipt(receipt); try checked(pixaura_result_review_query(1, context, &native, &out), "Result review") }
        else { try checked(pixaura_result_review_query(1, context, nil, &out), "Result review") }
        return ResultReview(out)
    }
    public func renderResult(_ review: ResultReview, revision: String) throws -> CGImage {
        let id = Array(revision.utf8); guard id.count == 32 else { throw ManualEditorFailure(status: 1, boundary: "Result review") }
        gate.lock(); defer { gate.unlock() }
        try checked(closed ? 13 : 0, "Result review")
        if let versioned {
            let history = try versioned.history()
            try checked(review.v2Owner == identity && review.v2Epoch == history.epoch && review.live == history.current && review.documentID == history.documentID ? 0 : 9, "Result review")
            let frame = try versioned.previewRevision(revision)
            let image = try frame.image(); try checked(frame.isCurrent ? 0 : 9, "Result preview current")
            return image
        }
        var binding = review.packet, width: UInt32 = 0, height: UInt32 = 0
        var bytes = Data(count: 256 * 256 * 4)
        try checked(id.withUnsafeBytes { revisionPointer in bytes.withUnsafeMutableBytes { raw in
            pixaura_result_review_render(1, context, &binding, revisionPointer.bindMemory(to: CChar.self).baseAddress, 256, 256, raw.bindMemory(to: UInt8.self).baseAddress, UInt64(raw.count), &width, &height)
        } }, "Result review")
        try checked(width > 0 && height > 0 && width <= 256 && height <= 256 ? 0 : 14, "Result review")
        let count = Int(width) * Int(height) * 4
        guard let provider = CGDataProvider(data: Data(bytes.prefix(count)) as CFData), let space = CGColorSpace(name: CGColorSpace.sRGB),
              let image = CGImage(width: Int(width), height: Int(height), bitsPerComponent: 8, bitsPerPixel: 32, bytesPerRow: Int(width) * 4, space: space,
                bitmapInfo: CGBitmapInfo.byteOrder32Big.union(CGBitmapInfo(rawValue: CGImageAlphaInfo.last.rawValue)),
                provider: provider, decode: nil, shouldInterpolate: false, intent: .defaultIntent) else { throw ManualEditorFailure(status: 8, boundary: "Result review") }
        return image
    }
#if DEBUG
    public func reviewPreviewTestControl(_ command: UInt32, _ point: UInt32 = 0) -> Int32 {
        gate.lock(); defer { gate.unlock() }; if let versioned { return versioned.certificationControl(4, command, point) }; return destroyed ? -1 : reviewPreviewControl(context, command, point)
    }
    public func reviewTestControl(_ command: UInt32, _ point: UInt32 = 0) -> Int32 {
        gate.lock(); defer { gate.unlock() }; if let versioned { return versioned.certificationControl(3, command, point) }; return destroyed ? -1 : agentPlatformControl(context, command, point)
    }
    public func autoTestControl(_ command: UInt32, _ value: UInt32 = 0) -> Int32 {
        gate.lock(); defer { gate.unlock() }; if let versioned { return versioned.certificationControl(2, command, value) }; return destroyed ? -1 : PixAuraCoreAutoControl(context, command, value)
    }
    // Certification transport ONLY: existing kind-3 provider and all validators.
    // No approval or executor. Entire method absent from release compilation.
    public func preparePlanCandidate(_ bytes: Data) throws -> PreparedAdvisory? {
        gate.lock(); defer { gate.unlock() }; try checked(closed ? 13 : 0, "Plan candidate")
        guard let versioned else { return nil }
        let task = PreparedAdvisory(identity, -1, try versioned.beginPlanCandidate(bytes))
        pendingV2Advisory = task; lastV2Advisory = nil; return task
    }
    public func inspectionCandidateTest(_ bytes: Data) throws -> AutoProposal { try inspectionCandidateTest(bytes, prepared: nil) }
    public func inspectionCandidateTest(_ bytes: Data, prepared: PreparedAdvisory?) throws -> AutoProposal {
        if let versioned {
            let task = try prepared ?? preparePlanCandidate(bytes)
            guard let task, task.owner == identity, task.index == -1 else { throw ManualEditorFailure(status: 9, boundary: "Plan candidate") }
            defer { gate.lock(); if pendingV2Advisory === task { pendingV2Advisory = nil }; gate.unlock() }
            let advice = try versioned.runAdvisory(task.native)
            gate.lock(); defer { gate.unlock() }; lastV2Advisory = advice
            return try v2Proposal(advice)
        }
        gate.lock(); defer { gate.unlock() }
        try checked(closed ? 13 : 0, "Inspection test candidate")
        var request = pixaura_auto_request(); request.api_version = 1; request.struct_size = UInt32(MemoryLayout.size(ofValue: request)); request.kind = 3
        var value = pixaura_auto_proposal()
        try checked(bytes.withUnsafeBytes { pixaura_editor_auto_request(1, context, &request, $0.bindMemory(to: UInt8.self).baseAddress, UInt64($0.count), &value) }, "Inspection test candidate")
        try checked(value.action_count > 0 && value.action_count <= 8 ? 0 : 14, "Inspection test candidate")
        let actions = withUnsafeBytes(of: value.actions) { raw in raw.bindMemory(to: pixaura_auto_action.self).prefix(Int(value.action_count)).enumerated().map { i, a in
            AssistChoice(index: i, toolIndex: Int(a.tool_index), values: Array([a.values.0, a.values.1, a.values.2, a.values.3].prefix(Int(a.parameter_count))))
        } }
        func id<T>(_ v: T) -> String { withUnsafeBytes(of: v) { String(decoding: $0.prefix { $0 != 0 }, as: UTF8.self) } }
        return AutoProposal(owner: identity, packet: value, actions: actions, proposalID: id(value.proposal_id), providerID: id(value.provider_id))
    }
    public func adviceTestControl(_ command: UInt32, _ value: UInt32 = 0) -> Int32 {
        gate.lock(); defer { gate.unlock() }; if let versioned { return versioned.certificationControl(1, command, value) }; return destroyed ? -1 : assistTestControl(context, command, value)
    }
    func agentTestControl(_ command: UInt32, _ point: UInt32 = 0) -> Int32 {
        gate.lock(); defer { gate.unlock() }
        if let versioned { return versioned.certificationControl(3, command, point) }; return destroyed ? -1 : agentPlatformControl(context, command, point)
    }
#endif
    public final class ReviewPreviewRequest {
        fileprivate let identity: String, review: AutoReview
        fileprivate var ticket: pixaura_review_preview_ticket
        fileprivate var v2Task: EditorReviewPreviewV2? = nil
        fileprivate init(_ identity: String, _ review: AutoReview, _ ticket: pixaura_review_preview_ticket) { self.identity = identity; self.review = review; self.ticket = ticket }
    }
    public final class ReviewComparison {
        fileprivate let identity: String
        fileprivate let review: AutoReview
        fileprivate var packet: pixaura_review_preview_pair
        fileprivate var v2Pair: EditorReviewPairV2? = nil
        public let before: CGImage, after: CGImage, geometryChanged: Bool, dimensions: String
        fileprivate init(_ identity: String, _ pair: EditorReviewPairV2, _ before: CGImage, _ after: CGImage, _ review: AutoReview) {
            self.identity = identity; self.packet = pixaura_review_preview_pair(); self.v2Pair = pair
            self.before = before; self.after = after; self.review = review
            geometryChanged = review.inspection.actions.contains(where: \.geometry)
            dimensions = "Before \(pair.before.sourceWidth) × \(pair.before.sourceHeight); After \(pair.after.sourceWidth) × \(pair.after.sourceHeight)"
        }
        fileprivate init(_ identity: String, _ packet: pixaura_review_preview_pair, _ before: CGImage, _ after: CGImage, _ review: AutoReview) {
            self.identity = identity; self.packet = packet; self.before = before; self.after = after; self.review = review
            geometryChanged = packet.geometry_changed == 1
            dimensions = "Before \(packet.before_width) × \(packet.before_height); After \(packet.after_width) × \(packet.after_height)"
        }
    }
    public func beginReviewPreview(_ review: AutoReview) throws -> ReviewPreviewRequest {
        gate.lock(); defer { gate.unlock() }
        idle.lock(); let busy = assistCalls != 0; idle.unlock()
        try checked(closed ? 13 : review.owner != identity || review.attempted || review.abandoned ? 9 : busy || applyingReview != nil || cancellingReview ? 8 : 0, "Review preview")
        if let versioned, let advice = review.proposal.v2Advice {
            _ = try inspectReview(review); comparison = nil
            let request = ReviewPreviewRequest(identity, review, pixaura_review_preview_ticket())
            request.v2Task = try versioned.beginReviewPreview(advice); return request
        }
        var inspection = review.inspection.packet
        try checked(pixaura_review_inspection_current(1, context, &inspection), "Plan inspection current")
        comparison = nil; var ticket = pixaura_review_preview_ticket(), proposal = review.proposal.packet
        let bytes = review.canonicalBytes
        try checked(bytes.withUnsafeBytes { pixaura_review_preview_begin(1, context, &proposal, $0.bindMemory(to: UInt8.self).baseAddress, UInt64($0.count), 512, 512, &ticket) }, "Review preview")
        return ReviewPreviewRequest(identity, review, ticket)
    }
    public func renderReviewPreview(_ request: ReviewPreviewRequest) throws -> ReviewComparison {
        gate.lock()
        idle.lock(); let busy = assistCalls != 0; idle.unlock()
        do { try checked(closed ? 13 : request.identity != identity ? 9 : busy || applyingReview != nil ? 8 : 0, "Review preview") }
        catch { gate.unlock(); throw error }
        idle.lock(); assistCalls += 1; idle.unlock(); gate.unlock()
        if let task = request.v2Task {
            defer { idle.lock(); assistCalls -= 1; idle.broadcast(); idle.unlock() }
            let pair = try task.render(); let before = try pair.before.image(), after = try pair.after.image()
            try checked(pair.isCurrent ? 0 : 9, "Review preview current")
            return ReviewComparison(identity, pair, before, after, request.review)
        }
        var ticket = request.ticket, pair = pixaura_review_preview_pair()
        defer {
            _ = pixaura_review_preview_release(1, context, &pair)
            idle.lock(); assistCalls -= 1; idle.broadcast(); idle.unlock()
        }
        try checked(pixaura_review_preview_render(1, context, &ticket, &pair), "Review preview")
        func image(_ side: UInt32, _ metadata: pixaura_preview_metadata) throws -> CGImage {
            try checked(metadata.width > 0 && metadata.height > 0 && metadata.width <= 1024 && metadata.height <= 1024 && metadata.image_bytes == UInt64(metadata.width) * UInt64(metadata.height) * 4 ? 0 : 14, "Review preview metadata")
            var bytes = Data(count: Int(metadata.image_bytes))
            try checked(bytes.withUnsafeMutableBytes { pixaura_review_preview_copy(1, context, &pair, side, 0, $0.bindMemory(to: UInt8.self).baseAddress, UInt64($0.count)) }, "Review preview copy")
            guard let provider = CGDataProvider(data: bytes as CFData), let space = CGColorSpace(name: CGColorSpace.sRGB),
                  let value = CGImage(width: Int(metadata.width), height: Int(metadata.height), bitsPerComponent: 8, bitsPerPixel: 32,
                    bytesPerRow: Int(metadata.row_stride), space: space,
                    bitmapInfo: CGBitmapInfo.byteOrder32Big.union(CGBitmapInfo(rawValue: CGImageAlphaInfo.last.rawValue)),
                    provider: provider, decode: nil, shouldInterpolate: false, intent: .defaultIntent) else { throw ManualEditorFailure(status: 8, boundary: "Review preview") }
            return value
        }
        let before = try image(0, pair.before), after = try image(1, pair.after)
        try checked(pixaura_review_preview_current(1, context, &pair), "Review preview current")
        return ReviewComparison(identity, pair, before, after, request.review)
    }
    @discardableResult public func installReviewPreview(_ value: ReviewComparison, replace: () -> Void) -> Bool {
        guard gate.try() else { return false }; defer { gate.unlock() }
        if let pair = value.v2Pair {
            guard !closed, value.identity == identity, !value.review.attempted, !value.review.abandoned, pair.isCurrent,
                  (try? inspectAutoLocked(value.review.proposal)) == value.review.canonicalBytes else { return false }
            comparison = value; replace(); return true
        }
        var pair = value.packet
        guard !closed, value.identity == identity, pixaura_review_preview_current(1, context, &pair) == 0 else { return false }
        var inspection = value.review.inspection.packet
        guard pixaura_review_inspection_preview(1, context, &inspection, &pair) == 0 else { return false }
        comparison = value; replace(); return true
    }
    public func cancelReviewPreview(_ request: ReviewPreviewRequest) throws {
        if let task = request.v2Task {
            try checked(request.identity == identity ? 0 : 9, "Review preview owner")
            try task.cancel(); gate.lock(); comparison = nil; gate.unlock(); return
        }
        gate.lock(); defer { gate.unlock() }; var ticket = request.ticket
        try checked(closed ? 13 : request.identity != identity ? 9 : pixaura_review_preview_cancel(1, context, &ticket), "Review preview cancellation")
        comparison = nil
    }
    public final class PreparedExport {
        fileprivate let owner: String, format: UInt32, quality: UInt32, task: EditorExportV2
        fileprivate init(_ owner: String, _ format: UInt32, _ quality: UInt32, _ task: EditorExportV2) { self.owner = owner; self.format = format; self.quality = quality; self.task = task }
    }
    public struct StagedExport: Sendable {
        public let url: URL, width: Int, height: Int
        public let native: NativeExportObservationV2?
    }
    private var activeV2Export: PreparedExport?
    public func prepareExport(format: UInt32, quality: UInt32) throws -> PreparedExport? {
        gate.lock(); defer { gate.unlock() }; try checked(closed ? 13 : 0, "Export")
        guard let versioned else { return nil }
        idle.lock(); let busy = !acceptingExport || exportCalls != 0 || assistCalls != 0; idle.unlock()
        try checked(busy || applyingReview != nil ? 8 : 0, "Export")
        guard let q = Int32(exactly: quality) else { throw ManualEditorFailure(status: 7, boundary: "Export quality") }
        let task = PreparedExport(identity, format, quality, try versioned.beginExport(format: format, quality: q))
        activeV2Export = task; return task
    }
    /// Private staging only. Destination delivery/permission is a platform action.
    /// v2 streams to disk; legacy retains its certified bounded encoder.
    public func exportStaged(to url: URL, format: UInt32, quality: UInt32, prepared: PreparedExport? = nil) throws -> StagedExport {
        if versioned == nil {
            let result = try exportEncoded(format: format, quality: quality)
            let fd = url.withUnsafeFileSystemRepresentation { $0.map { Darwin.open($0, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0o600) } ?? -1 }
            guard fd >= 0 else { throw ManualEditorFailure(status: 12, boundary: "Export staging") }
            let handle = FileHandle(fileDescriptor: fd, closeOnDealloc: true)
            defer { try? handle.close() }
            do { try handle.write(contentsOf: result.bytes); try handle.synchronize() }
            catch { try? FileManager.default.removeItem(at: url); throw error }
            return StagedExport(url: url, width: result.width, height: result.height, native: nil)
        }
        gate.lock(); let task: PreparedExport
        do {
            guard let chosen = try prepared ?? prepareExport(format: format, quality: quality) else { throw ManualEditorFailure(status: 5, boundary: "Export") }
            try checked(!closed && chosen.owner == identity && chosen.format == format && chosen.quality == quality ? 0 : 9, "Export binding")
            task = chosen
            idle.lock(); let busy = !acceptingExport || exportCalls != 0 || assistCalls != 0
            if !busy { exportCalls += 1 }; idle.unlock(); try checked(busy ? 8 : 0, "Export")
        } catch { gate.unlock(); throw error }
        gate.unlock()
        defer { gate.lock(); if activeV2Export === task { activeV2Export = nil }; gate.unlock(); idle.lock(); exportCalls -= 1; idle.broadcast(); idle.unlock() }
        let result = try task.task.run(to: url)
        return StagedExport(url: url, width: Int(result.width), height: Int(result.height), native: result)
    }
    public struct EncodedExport {
        public let bytes: Data, width: Int, height: Int, state: UInt32
    }
    /// Reads one committed revision. Does not approve, execute, or change history.
    public func exportEncoded(format: UInt32, quality: UInt32, mode: UInt32 = 0, maxWidth: UInt32 = 0, maxHeight: UInt32 = 0, revisionID: String? = nil) throws -> EncodedExport {
        if let revisionID, revisionID.utf8.count != 32 { throw ManualEditorFailure(status: 1, boundary: "Export") }
        idle.lock()
        if !acceptingExport { idle.unlock(); throw ManualEditorFailure(status: 13, boundary: "Export") }
        if exportCalls != 0 || assistCalls != 0 { idle.unlock(); throw ManualEditorFailure(status: 8, boundary: "Export") }
        exportCalls += 1
        idle.unlock()
        defer { idle.lock(); exportCalls -= 1; idle.broadcast(); idle.unlock() }
        gate.lock(); let reviewing = applyingReview != nil; gate.unlock()
        if reviewing { throw ManualEditorFailure(status: 8, boundary: "Export") }
        guard MemoryLayout<pixaura_export_request>.size == 72 else { throw ManualEditorFailure(status: 14, boundary: "Export") }
        var request = pixaura_export_request()
        try checked(pixaura_export_bind(1, context, &request), "Export")
        if let revisionID {
            let bytes = Array(revisionID.utf8)
            try withUnsafeMutableBytes(of: &request) { raw in
                guard raw.count >= 72, let base = raw.baseAddress else { throw ManualEditorFailure(status: 14, boundary: "Export") }
                bytes.withUnsafeBytes { source in base.advanced(by: 40).copyMemory(from: source.baseAddress!, byteCount: 32) }
            }
        }
        request.format = format; request.quality = quality; request.mode = mode
        request.max_width = maxWidth; request.max_height = maxHeight
        let capacity = 8 * 1024 * 1024
        var output = [UInt8](repeating: 0, count: capacity)
        var count: UInt64 = 0, width: UInt32 = 0, height: UInt32 = 0, state: UInt32 = 7
        let status = output.withUnsafeMutableBytes { raw in
            pixaura_export_encode(1, context, &request, nil, raw.bindMemory(to: UInt8.self).baseAddress, UInt64(capacity), &count, &width, &height, &state)
        }
        try checked(status, "Export")
        try checked(state == 6 && width > 0 && height > 0 && count <= UInt64(capacity) ? 0 : 14, "Export")
        return EncodedExport(bytes: Data(output.prefix(Int(count))), width: Int(width), height: Int(height), state: state)
    }
    public func exportCancel() throws {
        if versioned != nil {
            gate.lock(); let task = activeV2Export; gate.unlock()
            try task?.task.cancel(); return
        }
        let status = pixaura_export_cancel(context)
        if status != 0 && status != 1 && status != 3 { throw ManualEditorFailure(status: status, boundary: "Export") }
    }
    public func close() {
        if let versioned {
            gate.lock(); closed = true; comparison = nil; displayed = nil; committed = nil; gate.unlock()
            versioned.stop()
            idle.lock(); acceptingExport = false
            while renders != 0 || agentCalls != 0 || assistCalls != 0 || exportCalls != 0 { idle.wait() }
            idle.unlock(); versioned.close(); gate.lock(); destroyed = true; stopped = true; gate.unlock(); return
        }
        gate.lock()
        comparison = nil
        if !closed { closed = true }
        gate.unlock()
        idle.lock(); acceptingExport = false; let pending = exportCalls != 0; idle.unlock()
        if pending { _ = pixaura_export_cancel(context) }
        idle.lock(); while exportCalls != 0 { idle.wait() }; idle.unlock()
        gate.lock()
        if !stopped { _ = pixaura_editor_stop(context); stopped = true }
        gate.unlock()
        idle.lock(); while renders != 0 || agentCalls != 0 || assistCalls != 0 || exportCalls != 0 { idle.wait() }; idle.unlock()
        gate.lock()
        if !destroyed { _ = pixaura_editor_destroy(context); destroyed = true }
        gate.unlock()
    }
    private func v2Actions(_ inspection: NativeReviewInspectionV2) -> [ReviewAction] {
        inspection.actions.map { action in
            ReviewAction(index: action.index, toolID: action.toolID, label: action.label,
                operationVersion: action.operationVersion, parameterVersion: action.parameterVersion,
                geometry: action.geometry, parameters: action.parameters.map {
                    ReviewParameter(name: $0.name, unit: $0.unit, raw: $0.raw, minimum: $0.minimum, maximum: $0.maximum, neutral: $0.neutral)
                })
        }
    }
    private func v2Proposal(_ native: EditorAdvisoryV2) throws -> AutoProposal {
        guard let versioned else { throw ManualEditorFailure(status: 5, boundary: "Versioned editor") }
        let inspection = try versioned.inspectReview(native)
        let actions = try inspection.actions.map { action -> AssistChoice in
            try checked(tools.indices.contains(action.toolIndex) && tools[action.toolIndex].toolID == action.toolID ? 0 : 14, "Native inspection tool")
            return AssistChoice(index: action.index, toolIndex: action.toolIndex, values: action.parameters.map(\.raw))
        }
        return AutoProposal(owner: identity, packet: pixaura_auto_proposal(), actions: actions, proposalID: native.id, providerID: inspection.providerID, v2Advice: native)
    }
    private func v2Review(_ proposal: AutoProposal) throws -> AutoReview {
        guard let versioned, let native = proposal.v2Advice else { throw ManualEditorFailure(status: 5, boundary: "Versioned editor") }
        let data = try inspectAutoLocked(proposal); let described = try versioned.inspectReview(native); let actions = v2Actions(described)
        let state = try versioned.state(); try checked(state.epoch == native.epoch ? 0 : 9, "Auto review")
        let warnings = described.warnings
        let inspection = PlanInspection(packet: pixaura_review_inspection(), actions: actions, warnings: warnings)
        return AutoReview(identity, proposal, AgentPlan(data), state.revision, inspection)
    }
    deinit { close() }
}
