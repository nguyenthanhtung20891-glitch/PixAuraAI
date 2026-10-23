import Foundation
import CoreGraphics
import Darwin
import CPixAuraCore
#if DEBUG
@_silgen_name("pixaura_test_agent_platform_control")
private func agentPlatformControl(_ context: UnsafeMutablePointer<pixaura_editor_context>, _ command: UInt32, _ point: UInt32) -> Int32
@_silgen_name("pixaura_test_assist_control")
private func assistTestControl(_ context: UnsafeMutablePointer<pixaura_editor_context>, _ command: UInt32, _ point: UInt32) -> Int32
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
    private var displayed: CGImage?, committed: CGImage?
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
        let approval = AgentApproval(self); var request = agentRequest()
        try checked(plan.canonicalBytes.withUnsafeBytes {
            pixaura_editor_agent_approve(1, context, &request, $0.bindMemory(to: UInt8.self).baseAddress, UInt64($0.count), &approval.token)
        }, "Agent approval")
        approval.owned = true; return approval
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
        func text<T>(_ value: T) -> String { withUnsafeBytes(of: value) { String(decoding: $0, as: UTF8.self) } }
        return AgentReceipt(planSHA256: text(receipt.plan_sha256), sourceRevision: text(receipt.source_revision), resultingRevision: text(receipt.resulting_revision))
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
        if closed || assistCalls != 0 { let code: Int32 = closed ? 13 : 8; idle.unlock(); gate.unlock(); throw ManualEditorFailure(status: code, boundary: "Advice") }
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
#if DEBUG
    public func adviceTestControl(_ command: UInt32, _ value: UInt32 = 0) -> Int32 {
        gate.lock(); defer { gate.unlock() }; return destroyed ? -1 : assistTestControl(context, command, value)
    }
    func agentTestControl(_ command: UInt32, _ point: UInt32 = 0) -> Int32 {
        gate.lock(); defer { gate.unlock() }
        return destroyed ? -1 : agentPlatformControl(context, command, point)
    }
#endif
    public func close() {
        gate.lock()
        if !closed { closed = true; _ = pixaura_editor_stop(context) }
        gate.unlock()
        idle.lock(); while renders != 0 || agentCalls != 0 || assistCalls != 0 { idle.wait() }; idle.unlock()
        gate.lock()
        if !destroyed { _ = pixaura_editor_destroy(context); destroyed = true }
        gate.unlock()
    }
    deinit { close() }
}
