import Foundation
import CoreGraphics
import Darwin
import CPixAuraCore
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
    }
    public struct Candidate {
        public let request: Request, image: CGImage
    }
    private let storage = EditorContextStorage()
    private var context: UnsafeMutablePointer<pixaura_editor_context> { storage.pointer }
    private let gate = NSRecursiveLock()
    private let idle = NSCondition()
    private let identity = UUID().uuidString.replacingOccurrences(of: "-", with: "").lowercased()
    private var closed = false, destroyed = false, renders = 0, agentCalls = 0, assistCalls = 0
    private var applyingReview: AutoReview?
    private var applyingApproval: AgentApproval?
    private var cancellingReview = false
    private var attemptedAutoSerial: UInt64?
    private var displayed: CGImage?, committed: CGImage?
    private var comparison: ReviewComparison?
    private var requestedGeneration: UInt64 = 0
    public private(set) var tools: [ManualTool] = []
    private func checked(_ status: Int32, _ boundary: String) throws {
        if status != 0 { throw ManualEditorFailure(status: status, boundary: boundary) }
    }
    public init(root: URL, importing input: URL? = nil, capability: AppleCapabilitySnapshot? = nil) throws {
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
            let status = rootBytes.withUnsafeBufferPointer { path in id.withUnsafeBufferPointer { seed in
                if var capability {
                    var strategy = pixaura_execution_strategy()
                    return pixaura_editor_open_policy(1, context, path.baseAddress, UInt64(path.count), seed.baseAddress, UInt64(seed.count),
                        reader == nil ? nil : editorRead, reader.map { Unmanaged.passUnretained($0).toOpaque() }, count,
                        &capability.profile, &capability.observation, &strategy)
                }
                return pixaura_editor_open(1, context, path.baseAddress, UInt64(path.count), seed.baseAddress, UInt64(seed.count),
                    reader == nil ? nil : editorRead, reader.map { Unmanaged.passUnretained($0).toOpaque() }, count)
            } }
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
        var value = pixaura_execution_strategy()
        try checked(closed ? 13 : pixaura_editor_policy_query(1, context, &value), "Capability")
        return AppleExecutionStrategy(value)
    }
    @discardableResult public func applyCapability(_ snapshot: AppleCapabilitySnapshot) throws -> AppleExecutionStrategy {
        gate.lock(); defer { gate.unlock() }
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
        var size: UInt64 = 0
        try checked(pixaura_editor_state(context, nil, 0, &size), "State")
        var json = Data(count: Int(size))
        try checked(json.withUnsafeMutableBytes { pixaura_editor_state(context, $0.bindMemory(to: UInt8.self).baseAddress, size, &size) }, "State")
        return try JSONDecoder().decode(ManualEditorState.self, from: json)
    }
    public func select(_ index: Int) throws {
        guard let checkedIndex = UInt32(exactly: index) else { throw ManualEditorFailure(status: 5, boundary: "Tool") }
        gate.lock(); defer { gate.unlock() }; displayed = committed; requestedGeneration = 0
        try checked(closed ? 13 : pixaura_editor_select(context, checkedIndex), "Tool")
    }
    public func update(_ values: [Int32], editID: String? = nil) throws {
        gate.lock(); defer { gate.unlock() }
        if let editID, try stateLocked().editID != editID { throw ManualEditorFailure(status: 9, boundary: "Parameter") }
        try checked(closed ? 13 : values.withUnsafeBufferPointer { pixaura_editor_update(context, $0.baseAddress, UInt32($0.count)) }, "Parameter")
        requestedGeneration = 0
    }
    public func action(_ action: UInt32) throws {
        gate.lock(); defer { gate.unlock() }
        let status = closed ? 13 : pixaura_editor_action(context, action)
        if status == 0 || status == 9 { requestedGeneration = 0; if action == 0 || action == 4 || status == 9 { displayed = committed } }
        try checked(status, action == 1 ? "Commit" : "History/cancel")
    }
    public func request() throws -> Request {
        gate.lock(); defer { gate.unlock() }
        let approved = try !stateLocked().active
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
    @discardableResult public func install(_ candidate: Candidate, replace: (CGImage) -> Void = { _ in }) -> Bool {
        guard gate.try() else { return false }; defer { gate.unlock() }
        var request = candidate.request.native
        guard !closed, candidate.request.identity == identity, pixaura_editor_current(context, &request) == 0 else { return false }
        displayed = candidate.image
        if candidate.request.approved { committed = candidate.image }
        replace(candidate.image)
        return true
    }
    public func displayedImage() -> CGImage? { gate.lock(); defer { gate.unlock() }; return displayed }
    public func isRequested(_ request: Request) -> Bool {
        gate.lock(); defer { gate.unlock() }
        return !closed && request.identity == identity && request.native.ticket.generation == requestedGeneration
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
        try checked(closed ? 13 : pixaura_editor_agent_revoke(1, context, &approval.token), "Agent revocation")
    }
    private func releaseAgentApproval(_ approval: AgentApproval) {
        gate.lock(); defer { gate.unlock() }
        if approval.owned {
            if !closed { _ = pixaura_editor_agent_release(1, context, &approval.token) }
            approval.owned = false
        }
    }
    public func executeAgentPlan(_ plan: AgentPlan, approval: AgentApproval? = nil) throws -> AgentReceipt {
        gate.lock()
        if closed { gate.unlock(); throw ManualEditorFailure(status: 13, boundary: "Agent execution") }
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
        try checked(closed ? 13 : pixaura_editor_agent_cancel(1, context), "Agent cancellation")
    }
    public struct AssistChoice {
        public let index: Int, toolIndex: Int, values: [Int32]
    }
    public struct AssistAdvice {
        fileprivate let owner: String, packet: pixaura_assist_advice
        public let choice: AssistChoice
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
    public func requestAdvice(_ index: Int) throws -> AssistAdvice {
        guard let index = UInt32(exactly: index) else { throw ManualEditorFailure(status: 1, boundary: "Advice") }
        gate.lock(); idle.lock()
        if closed || assistCalls != 0 || applyingReview != nil || cancellingReview { let code: Int32 = closed ? 13 : 8; idle.unlock(); gate.unlock(); throw ManualEditorFailure(status: code, boundary: "Advice") }
        assistCalls += 1; idle.unlock(); gate.unlock()
        defer { idle.lock(); assistCalls -= 1; idle.broadcast(); idle.unlock() }
        var request = pixaura_assist_request(); request.api_version = 1; request.struct_size = UInt32(MemoryLayout.size(ofValue: request)); request.choice_index = index
        var value = pixaura_assist_advice()
        try checked(pixaura_editor_assist_request(1, context, &request, &value), "Advice")
        return AssistAdvice(owner: identity, packet: value, choice: try assistChoice(value))
    }
    @discardableResult public func inspectAdvice(_ advice: AssistAdvice) throws -> AssistChoice {
        gate.lock(); defer { gate.unlock() }
        var value = advice.packet
        try checked(closed ? 13 : advice.owner != identity ? 9 : pixaura_editor_assist_current(1, context, &value), "Advice")
        return advice.choice
    }
    public func cancelAdvice() throws {
        gate.lock(); defer { gate.unlock() }; try checked(closed ? 13 : pixaura_editor_assist_cancel(1, context), "Advice")
    }
    public struct AutoProposal {
        fileprivate let owner: String, packet: pixaura_auto_proposal
        public let actions: [AssistChoice], proposalID: String, providerID: String
    }
    public func requestAuto(_ index: Int) throws -> AutoProposal {
        guard let index = UInt32(exactly: index) else { throw ManualEditorFailure(status: 1, boundary: "Auto proposal") }
        gate.lock(); idle.lock()
        if closed || assistCalls != 0 || applyingReview != nil || cancellingReview { let code: Int32 = closed ? 13 : 8; idle.unlock(); gate.unlock(); throw ManualEditorFailure(status: code, boundary: "Auto proposal") }
        assistCalls += 1; idle.unlock(); gate.unlock()
        defer { idle.lock(); assistCalls -= 1; idle.broadcast(); idle.unlock() }
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
        var value = proposal.packet, required: UInt64 = 0
        let query = pixaura_editor_auto_plan(1, context, &value, nil, 0, &required)
        try checked(query == 11 ? 0 : query == 0 ? 14 : query, "Auto proposal")
        try checked(required > 0 && required <= 8192 ? 0 : 14, "Auto proposal")
        var bytes = Data(count: Int(required)); let capacity = required
        let status = bytes.withUnsafeMutableBytes { pixaura_editor_auto_plan(1, context, &value, $0.bindMemory(to: UInt8.self).baseAddress, capacity, &required) }
        try checked(status, "Auto proposal"); return bytes
    }
    public func cancelAuto() throws { gate.lock(); defer { gate.unlock() }; try checked(closed ? 13 : pixaura_editor_auto_cancel(1, context), "Auto proposal") }
    public final class AutoReview {
        fileprivate let owner: String, proposal: AutoProposal, plan: AgentPlan
        public let sourceRevision: String
        public var canonicalBytes: Data { plan.canonicalBytes }
        fileprivate var attempted = false, abandoned = false
        fileprivate init(_ owner: String, _ proposal: AutoProposal, _ plan: AgentPlan, _ source: String) {
            self.owner = owner; self.proposal = proposal; self.plan = plan; sourceRevision = source
        }
    }
    public func prepareAutoReview(_ proposal: AutoProposal) throws -> AutoReview {
        gate.lock(); defer { gate.unlock() }
        try checked(closed ? 13 : proposal.owner != identity ? 9 : 0, "Auto review")
        try checked(applyingReview != nil || cancellingReview ? 8 : 0, "Auto review")
        try checked(attemptedAutoSerial == proposal.packet.request_serial ? 15 : 0, "Auto review")
        let plan = AgentPlan(try agentBytesLocked(2, inspectAutoLocked(proposal)))
        return AutoReview(identity, proposal, plan, try stateLocked().revisionID)
    }
    /// ONLY explicit review Apply may call this trusted application boundary.
    public func applyAutoFromHuman(_ review: AutoReview, onExecuting: () -> Void = {}) throws -> AgentReceipt {
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
#if DEBUG
    public func reviewPreviewTestControl(_ command: UInt32, _ point: UInt32 = 0) -> Int32 {
        gate.lock(); defer { gate.unlock() }; return destroyed ? -1 : reviewPreviewControl(context, command, point)
    }
    public func reviewTestControl(_ command: UInt32, _ point: UInt32 = 0) -> Int32 {
        gate.lock(); defer { gate.unlock() }; return destroyed ? -1 : agentPlatformControl(context, command, point)
    }
    public func autoTestControl(_ command: UInt32, _ value: UInt32 = 0) -> Int32 {
        gate.lock(); defer { gate.unlock() }; return destroyed ? -1 : PixAuraCoreAutoControl(context, command, value)
    }
    public func adviceTestControl(_ command: UInt32, _ value: UInt32 = 0) -> Int32 {
        gate.lock(); defer { gate.unlock() }; return destroyed ? -1 : assistTestControl(context, command, value)
    }
    func agentTestControl(_ command: UInt32, _ point: UInt32 = 0) -> Int32 {
        gate.lock(); defer { gate.unlock() }
        return destroyed ? -1 : agentPlatformControl(context, command, point)
    }
#endif
    public final class ReviewPreviewRequest {
        fileprivate let identity: String, review: AutoReview
        fileprivate var ticket: pixaura_review_preview_ticket
        fileprivate init(_ identity: String, _ review: AutoReview, _ ticket: pixaura_review_preview_ticket) { self.identity = identity; self.review = review; self.ticket = ticket }
    }
    public final class ReviewComparison {
        fileprivate let identity: String
        fileprivate var packet: pixaura_review_preview_pair
        public let before: CGImage, after: CGImage, geometryChanged: Bool, dimensions: String
        fileprivate init(_ identity: String, _ packet: pixaura_review_preview_pair, _ before: CGImage, _ after: CGImage) {
            self.identity = identity; self.packet = packet; self.before = before; self.after = after
            geometryChanged = packet.geometry_changed == 1
            dimensions = "Before \(packet.before_width) × \(packet.before_height); After \(packet.after_width) × \(packet.after_height)"
        }
    }
    public func beginReviewPreview(_ review: AutoReview) throws -> ReviewPreviewRequest {
        gate.lock(); defer { gate.unlock() }
        idle.lock(); let busy = assistCalls != 0; idle.unlock()
        try checked(closed ? 13 : review.owner != identity || review.attempted || review.abandoned ? 9 : busy || applyingReview != nil || cancellingReview ? 8 : 0, "Review preview")
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
        return ReviewComparison(identity, pair, before, after)
    }
    @discardableResult public func installReviewPreview(_ value: ReviewComparison, replace: () -> Void) -> Bool {
        guard gate.try() else { return false }; defer { gate.unlock() }
        var pair = value.packet
        guard !closed, value.identity == identity, pixaura_review_preview_current(1, context, &pair) == 0 else { return false }
        comparison = value; replace(); return true
    }
    public func cancelReviewPreview(_ request: ReviewPreviewRequest) throws {
        gate.lock(); defer { gate.unlock() }; var ticket = request.ticket
        try checked(closed ? 13 : request.identity != identity ? 9 : pixaura_review_preview_cancel(1, context, &ticket), "Review preview cancellation")
        comparison = nil
    }
    public func close() {
        gate.lock()
        comparison = nil
        if !closed { closed = true; _ = pixaura_editor_stop(context) }
        gate.unlock()
        idle.lock(); while renders != 0 || agentCalls != 0 || assistCalls != 0 { idle.wait() }; idle.unlock()
        gate.lock()
        if !destroyed { _ = pixaura_editor_destroy(context); destroyed = true }
        gate.unlock()
    }
    deinit { close() }
}
