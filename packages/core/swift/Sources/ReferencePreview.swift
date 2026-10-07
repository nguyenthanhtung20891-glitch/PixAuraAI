import Foundation
import CoreGraphics
import CPixAuraCore

/// One-shot, off-main-thread consumer. The image owns a derived Data copy;
/// it does not retain the native handle or authorize document changes.
public enum ReferencePreview {
    public static func copyImage(context: inout pixaura_decode_context,
                                 preview: inout pixaura_decode_handle) -> CGImage? {
        var metadata = pixaura_preview_metadata()
        guard pixaura_preview_query(&context, &preview, &metadata) == 0,
              metadata.pixel_format == 1, metadata.width > 0, metadata.height > 0,
              metadata.width <= 1024, metadata.height <= 1024,
              metadata.row_stride == UInt64(metadata.width) * 4,
              metadata.image_bytes == metadata.row_stride * UInt64(metadata.height) else { return nil }
        var bytes = Data(count: Int(metadata.image_bytes))
        let status = bytes.withUnsafeMutableBytes {
            pixaura_preview_copy(&context, &preview, 0,
                                 $0.bindMemory(to: UInt8.self).baseAddress, metadata.image_bytes)
        }
        guard status == 0, let provider = CGDataProvider(data: bytes as CFData),
              let space = CGColorSpace(name: CGColorSpace.sRGB) else { return nil }
        return CGImage(width: Int(metadata.width), height: Int(metadata.height),
                       bitsPerComponent: 8, bitsPerPixel: 32,
                       bytesPerRow: Int(metadata.row_stride), space: space,
                       bitmapInfo: CGBitmapInfo.byteOrder32Big.union(CGBitmapInfo(rawValue: CGImageAlphaInfo.last.rawValue)),
                       provider: provider, decode: nil, shouldInterpolate: false, intent: .defaultIntent)
    }
}

/// Synchronous off-main-thread harness. All begin/cancel/install calls use this
/// owner, so the eligibility check and displayed-reference replacement are atomic
/// relative to supersession. Native storage stays at a stable address until join.
public final class InteractivePreview {
    public struct Ticket {
        fileprivate let owner: Data
        fileprivate var native: pixaura_preview_ticket
        public var generation: UInt64 { native.generation }
    }
    public struct Candidate {
        public let ticket: Ticket
        public let image: CGImage
        fileprivate init(ticket: Ticket, image: CGImage) { self.ticket = ticket; self.image = image }
    }
    private let identity: Data
    private let context: UnsafeMutablePointer<pixaura_decode_context>
    private let gate = NSCondition()
    private var active = 0
    private var closed = false
    private var requested: Ticket?
    private var displayed: Candidate?
    public init?(identity: [UInt8]) {
        let storage = UnsafeMutablePointer<pixaura_decode_context>.allocate(capacity: 1)
        storage.initialize(to: pixaura_decode_context())
        var limits = pixaura_decode_limits()
        let status = identity.withUnsafeBufferPointer {
            pixaura_decode_default_limits(1, &limits) == 0
                ? pixaura_decode_context_init(1, storage, UInt32(MemoryLayout<pixaura_decode_context>.size), $0.baseAddress, UInt64($0.count), &limits) : 1
        }
        if status != 0 { storage.deinitialize(count: 1); storage.deallocate(); return nil }
        context = storage
        self.identity = Data(identity)
    }
    /// Application services prepare a source only while no calls can access the
    /// owner. Borrowed context must never escape this closure or race lifecycle.
    public func prepare<T>(_ body: (UnsafeMutablePointer<pixaura_decode_context>) -> T) -> T {
        gate.lock(); defer { gate.unlock() }
        precondition(!closed && active == 0 && requested == nil)
        return body(context)
    }
    public func begin() -> Ticket? {
        gate.lock(); defer { gate.unlock() }
        guard !closed else { return nil }
        var native = pixaura_preview_ticket()
        guard pixaura_preview_begin(context, 1, &native) == 0 else { return nil }
        let ticket = Ticket(owner: identity, native: native)
        requested = ticket; return ticket
    }
    public func cancel(_ ticket: Ticket) -> Bool {
        gate.lock(); defer { gate.unlock() }
        guard !closed, ticket.owner == identity else { return false }
        var native = ticket.native
        return pixaura_preview_cancel(context, &native) == 0
    }
    public func render(_ ticket: Ticket, working: pixaura_decode_handle, request: pixaura_preview_request) -> Candidate? {
        gate.lock()
        guard !closed, ticket.owner == identity, active < 2 else { gate.unlock(); return nil }
        active += 1; gate.unlock()
        defer { gate.lock(); active -= 1; gate.broadcast(); gate.unlock() }
        var source = working, native = ticket.native, parameters = request, preview = pixaura_decode_handle()
        guard pixaura_preview_render_interactive(context, &source, &native, &parameters, &preview) == 0 else { return nil }
        defer { _ = pixaura_preview_release(context, &preview) }
        guard let image = ReferencePreview.copyImage(context: &context.pointee, preview: &preview) else { return nil }
        return Candidate(ticket: ticket, image: image)
    }
    public func install(_ candidate: Candidate) -> Bool {
        gate.lock(); defer { gate.unlock() }
        guard !closed, candidate.ticket.owner == identity else { return false }
        var ticket = candidate.ticket.native
        guard pixaura_preview_current(context, &ticket, nil) == 0 else { return false }
        displayed = candidate; return true
    }
    public var requestedGeneration: UInt64? { gate.lock(); defer { gate.unlock() }; return requested?.generation }
    public var displayedGeneration: UInt64? { gate.lock(); defer { gate.unlock() }; return displayed?.ticket.generation }
    public var displayedImage: CGImage? { gate.lock(); defer { gate.unlock() }; return displayed?.image }
    public func close() {
        gate.lock(); defer { gate.unlock() }
        if closed { return }
        closed = true; _ = pixaura_preview_stop(context)
        while active != 0 { gate.wait() }
        _ = pixaura_decode_context_destroy(context)
    }
    deinit { close(); context.deinitialize(count: 1); context.deallocate() }
}
