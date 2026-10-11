import Foundation
import CoreGraphics
import Darwin
import CPixAuraCore

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
    public var errorDescription: String? {
        let meaning: String
        switch status {
        case 7: meaning = "Invalid parameter"
        case 3, 9: meaning = "Stale edit; reopen the committed project"
        case 8, 15: meaning = "Resource/admission failure"
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
    private var closed = false, renders = 0
    private var displayed: CGImage?, committed: CGImage?
    private var requestedGeneration: UInt64 = 0
    public private(set) var tools: [ManualTool] = []
    private func checked(_ status: Int32, _ boundary: String) throws {
        if status != 0 { throw ManualEditorFailure(status: status, boundary: boundary) }
    }
    public init(root: URL, importing input: URL? = nil) throws {
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
                pixaura_editor_open(1, context, path.baseAddress, UInt64(path.count), seed.baseAddress, UInt64(seed.count),
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
    public func close() {
        gate.lock()
        if !closed {
            closed = true; _ = pixaura_editor_stop(context)
            gate.unlock()
            idle.lock(); while renders != 0 { idle.wait() }; idle.unlock()
            gate.lock()
            _ = pixaura_editor_destroy(context)
        }
        gate.unlock()
    }
    deinit { close() }
}
