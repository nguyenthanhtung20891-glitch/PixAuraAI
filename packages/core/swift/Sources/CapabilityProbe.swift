import CPixAuraCore

public protocol CapabilityProbe {
    func snapshot() -> CapabilitySnapshot?
}

public struct CapabilitySnapshot: Sendable {
    public let diagnosticBytes: [UInt8]
}

// Foundation stub: compile-target identity only. Unknown hardware/pressure is
// explicit; no Apple device probe, policy, certification or editor mutation.
public struct ConservativeCapabilityProbe: CapabilityProbe {
    public init() {}
    public func snapshot() -> CapabilitySnapshot? {
        var profile = pixaura_capability_profile()
        guard pixaura_capability_baseline(PIXAURA_CAPABILITY_VERSION, &profile) == PIXAURA_OK else { return nil }
        var bytes = [UInt8](repeating: 0, count: Int(PIXAURA_CAPABILITY_WIRE_BYTES))
        var required: UInt64 = 0
        let capacity = UInt64(bytes.count)
        let status = bytes.withUnsafeMutableBufferPointer {
            pixaura_capability_write(PIXAURA_CAPABILITY_VERSION, &profile, $0.baseAddress, capacity, &required)
        }
        guard status == PIXAURA_OK, required == capacity else { return nil }
        return CapabilitySnapshot(diagnosticBytes: bytes)
    }
}
