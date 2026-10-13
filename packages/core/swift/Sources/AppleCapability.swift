import Foundation
import Dispatch
import Darwin
import Metal
import CPixAuraCore
#if os(iOS)
import UIKit
import os
#else
import AppKit
#endif

/// Internal runtime integration surface, never document state or certification.
/// Construction of normalized native records is restricted to this module.
public struct AppleCapabilitySnapshot {
    var profile: pixaura_capability_profile
    var observation: pixaura_policy_observation
    /// Preserve the strongest pending signals when a command lane is busy.
    /// This combines observations, never derives an execution strategy.
    public func retainingPressure(from older: AppleCapabilitySnapshot?) -> AppleCapabilitySnapshot {
        guard let older else { return self }
        var result = self
        result.observation.thermal = max(observation.thermal, older.observation.thermal)
        result.observation.memory_pressure = max(observation.memory_pressure, older.observation.memory_pressure)
        if result.observation.thermal <= 1, observation.thermal == 0 || older.observation.thermal == 0 { result.observation.thermal = 0 }
        if result.observation.memory_pressure <= 1, observation.memory_pressure == 0 || older.observation.memory_pressure == 0 { result.observation.memory_pressure = 0 }
        result.observation.constrained |= older.observation.constrained
        result.observation.process_budget_bytes = min(observation.process_budget_bytes, older.observation.process_budget_bytes)
        if older.observation.gpu_availability != 2 { result.observation.gpu_availability = older.observation.gpu_availability }
        return result
    }
}

public struct AppleExecutionStrategy {
    public let tier: UInt32, previewEdge: UInt32, concurrency: UInt32
    public let runtimeCapBytes: UInt64, epoch: UInt64
    public let backend: UInt32, forcedCPU: Bool
    init(_ value: pixaura_execution_strategy) {
        tier = value.tier; previewEdge = value.preview_edge; concurrency = value.concurrency
        runtimeCapBytes = value.runtime_cap_bytes; epoch = value.epoch
        backend = value.backend; forcedCPU = value.forced_cpu != 0
    }
}

protocol AppleCapabilitySignals: AnyObject {
    func facts() -> pixaura_capability_profile
    func readings() -> (UInt32, UInt32, Bool, UInt32, UInt64)
    func start(_ changed: @escaping () -> Void)
    func stop()
    func setConstrained(_ value: Bool)
}
extension AppleCapabilitySignals { func setConstrained(_ value: Bool) {} }

final class SystemAppleCapabilitySignals: AppleCapabilitySignals {
    private let lock = NSLock()
    private var pressure: UInt32 = 0
    private var constrained = true
    private var observers: [NSObjectProtocol] = []
    private var source: DispatchSourceMemoryPressure?
    private var token: NSObject?
    private var changed: (() -> Void)?
    func setConstrained(_ value: Bool) {
        lock.lock(); constrained = value; let callback = changed; lock.unlock(); callback?()
    }
    static func thermal(_ value: ProcessInfo.ThermalState) -> UInt32 {
        switch value { case .nominal: return 1; case .fair: return 2
        case .serious: return 3; case .critical: return 4; @unknown default: return 0 }
    }
    static func memory(_ value: DispatchSource.MemoryPressureEvent) -> UInt32 {
        if value.contains(.critical) { return 4 }
        if value.contains(.warning) { return 3 }
        if value == .normal { return 1 }
        return 0
    }
    private func budget() -> UInt64 {
        #if os(iOS)
        // Advisory remaining allocatable bytes; zero is unknown. Never cached.
        return min(UInt64(os_proc_available_memory()), ProcessInfo.processInfo.physicalMemory)
        #else
        // No equivalent fixed process limit is asserted on macOS.
        return 0
        #endif
    }
    private func backend() -> UInt32 { MTLCreateSystemDefaultDevice() == nil ? 1 : 2 }
    func facts() -> pixaura_capability_profile {
        var value = pixaura_capability_profile()
        _ = pixaura_capability_baseline(1, &value)
        let info = ProcessInfo.processInfo
        value.hardware.logical_cpu_count = (1...1024).contains(info.activeProcessorCount) ? UInt32(info.activeProcessorCount) : 0
        value.hardware.physical_memory_bytes = info.physicalMemory
        value.hardware.runtime_version = UInt32(exactly: info.operatingSystemVersion.majorVersion) ?? 0
        value.hardware.gpu_backend = 2; value.hardware.gpu_availability = backend()
        value.hardware.gpu_validation = 0
        #if targetEnvironment(simulator)
        value.hardware.provenance = 2
        #else
        // Runtime API observations do not establish physical certification.
        value.hardware.provenance = 0
        #endif
        value.runtime.thermal = 1; value.runtime.memory_pressure = 1; value.runtime.constrained = 0
        value.runtime.gpu_availability = value.hardware.gpu_availability
        value.runtime.process_budget_bytes = budget(); value.runtime.sequence = 1
        return value
    }
    func readings() -> (UInt32, UInt32, Bool, UInt32, UInt64) {
        lock.lock(); let memory = pressure, background = constrained; lock.unlock()
        return (Self.thermal(ProcessInfo.processInfo.thermalState), memory, background, backend(), budget())
    }
    func start(_ changed: @escaping () -> Void) {
        lock.lock(); defer { lock.unlock() }
        guard token == nil else { return }
        let identity = NSObject(); token = identity; self.changed = changed
        _ = ProcessInfo.processInfo.thermalState // Apple requires reading before registration.
        func observe(_ name: Notification.Name, _ memory: UInt32? = nil, _ background: Bool? = nil) {
            observers.append(NotificationCenter.default.addObserver(forName: name, object: nil, queue: nil) { [weak self] _ in
                guard let self else { return }
                self.lock.lock()
                guard self.token === identity else { self.lock.unlock(); return }
                if let memory { self.pressure = memory }; if let background { self.constrained = background }
                self.lock.unlock(); changed()
            })
        }
        observe(ProcessInfo.thermalStateDidChangeNotification)
        #if os(iOS)
        // Initial inactive is conservative; only legitimate lifecycle signals clear it.
        observe(UIApplication.didReceiveMemoryWarningNotification, 3)
        observe(UIApplication.didBecomeActiveNotification, nil, false)
        observe(UIApplication.willResignActiveNotification, nil, true)
        observe(UIApplication.didEnterBackgroundNotification, nil, true)
        #else
        observe(NSApplication.didBecomeActiveNotification, nil, false)
        observe(NSApplication.didResignActiveNotification, nil, true)
        #endif
        let pressureSource = DispatchSource.makeMemoryPressureSource(eventMask: [.normal, .warning, .critical], queue: DispatchQueue.global(qos: .utility))
        pressureSource.setEventHandler { [weak self] in
            guard let self else { return }
            self.lock.lock()
            guard self.token === identity else { self.lock.unlock(); return }
            self.pressure = Self.memory(self.source?.data ?? [])
            self.lock.unlock(); changed()
        }
        source = pressureSource; pressureSource.resume()
    }
    func stop() {
        lock.lock(); token = nil; let previous = observers, oldSource = source
        observers = []; source = nil; changed = nil; pressure = 0; lock.unlock()
        previous.forEach(NotificationCenter.default.removeObserver)
        oldSource?.setEventHandler {}; oldSource?.cancel()
    }
    deinit { stop() }
}

/// One caller-owned service. At most eight subscriptions; no Activity/scene/view
/// references, scheduler, native context or historical snapshots are retained.
public final class AppleCapabilityOwner {
    private let gate = NSRecursiveLock()
    private let signals: AppleCapabilitySignals
    private let profile: pixaura_capability_profile
    private var subscribers: [UUID: (AppleCapabilitySnapshot) -> Void] = [:]
    private var token: NSObject?, closed = false
    private var sequence: UInt64 = 0
    public convenience init() { self.init(signals: SystemAppleCapabilitySignals()) }
    init(signals: AppleCapabilitySignals) { self.signals = signals; profile = signals.facts() }
    public func setConstrained(_ value: Bool) { signals.setConstrained(value) }
    public func snapshot() throws -> AppleCapabilitySnapshot {
        gate.lock(); defer { gate.unlock() }
        guard !closed, sequence < UInt64.max else { throw ManualEditorFailure(status: 8, boundary: "Capability") }
        sequence += 1
        let reading = signals.readings()
        var observation = pixaura_policy_observation()
        observation.version = 1; observation.struct_size = UInt32(MemoryLayout<pixaura_policy_observation>.size)
        observation.thermal = reading.0; observation.memory_pressure = reading.1
        observation.constrained = reading.2 ? 1 : 0; observation.gpu_availability = reading.3
        observation.gpu_validation = 0; observation.process_budget_bytes = reading.4; observation.sequence = sequence
        return AppleCapabilitySnapshot(profile: profile, observation: observation)
    }
    public func subscribe(_ receive: @escaping (AppleCapabilitySnapshot) -> Void) throws -> UUID {
        gate.lock(); defer { gate.unlock() }
        guard !closed, subscribers.count < 8 else { throw ManualEditorFailure(status: 8, boundary: "Capability") }
        let id = UUID(); subscribers[id] = receive
        if token == nil {
            let identity = NSObject(); token = identity
            signals.start { [weak self] in
                guard let self else { return }
                self.gate.lock(); defer { self.gate.unlock() }
                guard self.token === identity, let value = try? self.snapshot() else { return }
                // Subscribers must only hand off bounded work, never call native/render here.
                for callback in Array(self.subscribers.values) { callback(value) }
            }
        }
        return id
    }
    public func unsubscribe(_ id: UUID) {
        gate.lock(); defer { gate.unlock() }; subscribers.removeValue(forKey: id)
        if subscribers.isEmpty { token = nil; signals.stop() }
    }
    public func close() {
        gate.lock(); defer { gate.unlock() }; closed = true; token = nil
        subscribers.removeAll(); signals.stop()
    }
    deinit { close() }
}
