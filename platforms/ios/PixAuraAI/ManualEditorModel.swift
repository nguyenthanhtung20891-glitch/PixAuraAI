import Foundation
import Combine
import CoreGraphics
import PixAuraCore

/// Application work lanes only. Core owns descriptors/gesture/history; queues
/// retain one latest integer payload and one latest preview, never slider tasks.
private final class EditorWorker {
    private let capabilities: AppleCapabilityOwner
    private var subscription: UUID?
    private var latestCapability: AppleCapabilitySnapshot?
    private var applyingCapability = false
    init(capabilities: AppleCapabilityOwner) {
        self.capabilities = capabilities
        subscription = try? capabilities.subscribe { [weak self] value in self?.receiveCapability(value) }
    }
    deinit { if let subscription { capabilities.unsubscribe(subscription) }; owner?.close() }
    private func receiveCapability(_ value: AppleCapabilitySnapshot) {
        lock.lock(); latestCapability = value.retainingPressure(from: latestCapability)
        let schedule = !applyingCapability; applyingCapability = true; lock.unlock()
        if schedule { commands.async { self.drainCapability() } }
    }
    private func drainCapability() {
        lock.lock(); let value = latestCapability; latestCapability = nil; lock.unlock()
        if let value, let owner {
            do {
                let old = try owner.strategy()
                let new = try owner.applyCapability(value)
                // A selected but untouched gesture has no accepted preview request.
                if old.epoch != new.epoch, owner.displayedImage() != nil {
                    let state = try owner.state()
                    if !state.active || state.values != owner.tools[state.selected].parameters.map({ Int32($0.defaultValue) }) { try preview(refreshFacts: false) }
                }
            } catch {
                // Older callbacks can lose to a fresh request snapshot; other
                // failures remain visible through the established error path.
                if (error as? ManualEditorFailure)?.status != 9 {
                    let message = error.localizedDescription
                    DispatchQueue.main.async { self.previewError?(owner, message) }
                }
            }
        }
        lock.lock(); let more = latestCapability != nil; if !more { applyingCapability = false }; lock.unlock()
        if more { commands.async { self.drainCapability() } }
    }
    private func initialCapability() throws -> AppleCapabilitySnapshot {
        lock.lock(); let pending = latestCapability; latestCapability = nil; lock.unlock()
        return try capabilities.snapshot().retainingPressure(from: pending)
    }
    let commands = DispatchQueue(label: "ai.pixaura.editor.commands", qos: .userInitiated)
    private let rendering = DispatchQueue(label: "ai.pixaura.editor.preview", qos: .userInitiated)
    private let lock = NSLock()
    private var owner: ManualEditor?
    private var latestValues: (String, [Int32], UInt64)?
    private var updating = false
    private var latestPreview: (ManualEditor, ManualEditor.Request)?
    private var renderingNow = false
    private var latestCandidate: (ManualEditor, ManualEditor.Candidate)?
    private var installing = false
    var refresh: (@MainActor (UInt64, ManualEditor?, ManualEditorState?, CGImage?, String?) -> Void)?
    var candidate: (@MainActor (ManualEditor, ManualEditor.Candidate) -> Void)?
    var previewError: (@MainActor (ManualEditor, String) -> Void)?
    private func emit(_ epoch: UInt64, _ error: String? = nil) {
        let editor = owner
        let state = try? editor?.state(), image = editor?.displayedImage()
        DispatchQueue.main.async { self.refresh?(epoch, editor, state, image, error) }
    }
    func send(_ epoch: UInt64, _ work: @escaping (EditorWorker) throws -> Void) {
        commands.async { do { try work(self); self.emit(epoch) } catch { self.emit(epoch, error.localizedDescription) } }
    }
    private func projectRoot() throws -> URL {
        let base = try FileManager.default.url(for: .applicationSupportDirectory, in: .userDomainMask, appropriateFor: nil, create: true)
        var root = base.appendingPathComponent("manual-projects", isDirectory: true)
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true,
            attributes: [.protectionKey: FileProtectionType.completeUntilFirstUserAuthentication])
        var policy = URLResourceValues(); policy.isExcludedFromBackup = true; try root.setResourceValues(policy)
        return root
    }
    func reopen() throws {
        if owner == nil, let name = UserDefaults.standard.string(forKey: "manual-project"), UUID(uuidString: name) != nil {
            owner = try ManualEditor(root: projectRoot().appendingPathComponent(name), capability: initialCapability())
        }
        if owner != nil { try preview() }
    }
    func importImage(_ input: URL) throws {
        let scoped = input.startAccessingSecurityScopedResource()
        defer { if scoped { input.stopAccessingSecurityScopedResource() } }
        let name = UUID().uuidString
        var directory = try projectRoot().appendingPathComponent(name, isDirectory: true)
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: false,
            attributes: [.protectionKey: FileProtectionType.completeUntilFirstUserAuthentication])
        var policy = URLResourceValues(); policy.isExcludedFromBackup = true; try directory.setResourceValues(policy)
        let replacement = try ManualEditor(root: directory, importing: input, capability: initialCapability())
        clearPending(); owner?.close(); owner = replacement
        UserDefaults.standard.set(name, forKey: "manual-project"); try preview()
    }
    private func clearPending() { lock.lock(); latestValues = nil; latestPreview = nil; latestCandidate = nil; lock.unlock() }
    func select(_ index: Int) throws { clearPending(); try owner?.select(index) }
    func action(_ action: UInt32) throws {
        lock.lock(); let final = latestValues; latestValues = nil; latestPreview = nil; lock.unlock()
        if action == 1, let final { try owner?.update(final.1, editID: final.0) }
        try owner?.action(action)
        if action != 4 { try preview() }
    }
    func close() { clearPending(); owner?.close(); owner = nil }
    func update(_ values: [Int32], editID: String, epoch: UInt64) {
        lock.lock(); latestValues = (editID, values, epoch)
        let schedule = !updating; updating = true; lock.unlock()
        if schedule { commands.async { self.drainValues() } }
    }
    private func drainValues() {
        lock.lock(); let values = latestValues; latestValues = nil; lock.unlock()
        do { if let values { try owner?.update(values.1, editID: values.0); emit(values.2); try preview() } }
        catch { if let values { emit(values.2, error.localizedDescription) } }
        lock.lock(); let more = latestValues != nil; if !more { updating = false }; lock.unlock()
        if more { commands.async { self.drainValues() } }
    }
    func retry() throws {
        guard let owner else { return }
        let state = try owner.state()
        if state.active && !state.approvalPending { try owner.update(state.values) }
        try preview()
    }
    private func preview(refreshFacts: Bool = true) throws {
        guard let owner else { return }
        if refreshFacts { try owner.applyCapability(initialCapability()) }
        let request = try owner.request()
        lock.lock(); latestPreview = (owner, request)
        let schedule = !renderingNow; renderingNow = true; lock.unlock()
        if schedule { rendering.async { self.drainPreview() } }
    }
    private func drainPreview() {
        while true {
            lock.lock(); let work = latestPreview; latestPreview = nil
            if work == nil { renderingNow = false }; lock.unlock()
            guard let work else { return }
            do {
                if let result = try work.0.render(work.1) {
                    lock.lock(); latestCandidate = (work.0, result)
                    let schedule = !installing; installing = true; lock.unlock()
                    // The command that requested this render must publish its
                    // owner/state before even a tiny preview can reach the UI.
                    // One mailbox notification remains bounded during churn.
                    if schedule { commands.async { DispatchQueue.main.async { self.installLatest() } } }
                }
            } catch {
                if work.0.isRequested(work.1) && (error as? ManualEditorFailure)?.status != 13 {
                    commands.async {
                        guard work.0.isRequested(work.1) else { return }
                        DispatchQueue.main.async { self.previewError?(work.0, "Preview failure: \(error.localizedDescription)") }
                    }
                }
            }
        }
    }
    @MainActor private func installLatest() {
        lock.lock(); let work = latestCandidate; latestCandidate = nil; installing = false; lock.unlock()
        if let work { candidate?(work.0, work.1) }
    }
}

@MainActor
final class ManualEditorModel: ObservableObject {
    @Published private(set) var tools: [ManualTool] = []
    @Published private(set) var core: ManualEditorState?
    @Published private(set) var image: CGImage?
    @Published private(set) var busy = false
    @Published private(set) var error: String?
    @Published private(set) var previewLoading = false
    private let worker: EditorWorker
    private var editor: ManualEditor?
    private var epoch: UInt64 = 0
    init(capabilities: AppleCapabilityOwner = AppleCapabilityOwner()) {
        worker = EditorWorker(capabilities: capabilities)
        worker.refresh = { [weak self] epoch, editor, state, image, error in
            guard let self, self.epoch == epoch else { return }
            self.editor = editor; self.tools = editor?.tools ?? []
            self.core = state; self.image = image
            self.busy = false; self.error = error
            if self.core?.active != true || error != nil { self.previewLoading = false }
        }
        worker.candidate = { [weak self] editor, candidate in
            guard let self, self.editor === editor else { return }
            if !editor.install(candidate, replace: { self.image = $0; self.previewLoading = false
                if self.error?.hasPrefix("Preview failure") == true { self.error = nil }
            }) { self.previewLoading = false }
        }
        worker.previewError = { [weak self] editor, error in
            guard let self, self.editor === editor else { return }
            self.previewLoading = false; self.error = error
        }
    }
    private func send(_ work: @escaping (EditorWorker) throws -> Void) { epoch += 1; busy = true; error = nil; worker.send(epoch, work) }
    func reopen() { send { try $0.reopen() } }
    func recover() { send { $0.close(); try $0.reopen() } }
    func importImage(_ input: URL) { send { try $0.importImage(input) } }
    func select(_ index: Int) { send { try $0.select(index) } }
    func update(_ values: [Int32]) { guard let core else { return }; epoch += 1; previewLoading = true; worker.update(values, editID: core.editID, epoch: epoch) }
    func action(_ action: UInt32) { send { try $0.action(action) } }
    func interrupt() { send { try $0.action(4) } }
    func close() { send { $0.close() } }
    func retryPreview() { previewLoading = true; send { try $0.retry() } }
    func pickerFailure(_ failure: Error) { error = failure.localizedDescription }
    #if DEBUG
    private static var fixtureOpened = false
    func testImage() {
        if Self.fixtureOpened { reopen(); return }
        Self.fixtureOpened = true
        send { worker in
            let url = FileManager.default.temporaryDirectory.appendingPathComponent("editor-ui-fixture.png")
            let fixture = Data(base64Encoded: "iVBORw0KGgoAAAANSUhEUgAAAAIAAAADCAYAAAC56t6BAAAAHElEQVR4nGP4z8DwHwwZGP6DAQMjE/N/Fla2BgCpRQuLxbxPlAAAAABJRU5ErkJggg==")!
            try fixture.write(to: url); defer { try? FileManager.default.removeItem(at: url) }
            try worker.importImage(url)
        }
    }
    #endif
}
