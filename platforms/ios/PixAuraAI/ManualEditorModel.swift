import Foundation
import Combine
import CoreGraphics
import PixAuraCore

enum AssistStatus { case idle, requesting, proposalAvailable, failed, stale, cancelled, rejected }
struct AssistUiState {
    var enabled = false, inFlight = false
    var status: AssistStatus = .idle
    var choices: [ManualEditor.AssistChoice] = []
    var selected = -1
    var advice: ManualEditor.AssistAdvice?
    var error: String?, notice: String?
}

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
    private let advising = DispatchQueue(label: "ai.pixaura.editor.advice", qos: .userInitiated)
    private var assistWorking = false
    private var cancellingAdvice = false
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
    func close() {
        clearPending()
        #if DEBUG
        if ProcessInfo.processInfo.arguments.contains("--assist-test-pause") {
            try? owner?.cancelAdvice(); _ = owner?.adviceTestControl(2)
        }
        #endif
        owner?.close(); owner = nil
    }
    #if DEBUG
    func adviceFixtureFailure() { _ = owner?.adviceTestControl(3, 2) }
    #endif
    func cancelAdvice() {
        lock.lock()
        if cancellingAdvice { lock.unlock(); return }
        cancellingAdvice = true; lock.unlock()
        commands.async {
            try? self.owner?.cancelAdvice()
            #if DEBUG
            if ProcessInfo.processInfo.arguments.contains("--assist-test-pause") { _ = self.owner?.adviceTestControl(2) }
            #endif
            self.lock.lock(); self.cancellingAdvice = false; self.lock.unlock()
        }
    }
    func adviceChoices(_ completion: @escaping @MainActor ([ManualEditor.AssistChoice]) -> Void) {
        commands.async { let values = (try? self.owner?.adviceChoices()) ?? []; DispatchQueue.main.async { completion(values) } }
    }
    func requestAdvice(_ index: Int, completion: @escaping @MainActor (Result<ManualEditor.AssistAdvice, Error>) -> Void) {
        lock.lock()
        if assistWorking { lock.unlock(); DispatchQueue.main.async { completion(.failure(ManualEditorFailure(status: 8, boundary: "Advice"))) }; return }
        assistWorking = true; lock.unlock()
        commands.async {
            guard let editor = self.owner else {
                self.lock.lock(); self.assistWorking = false; self.lock.unlock()
                DispatchQueue.main.async { completion(.failure(ManualEditorFailure(status: 13, boundary: "Advice"))) }; return
            }
            #if DEBUG
            if ProcessInfo.processInfo.arguments.contains("--assist-test-pause") { _ = editor.adviceTestControl(0, 0) }
            #endif
            self.advising.async {
                let result = Result { try editor.requestAdvice(index) }
                self.commands.async {
                    let checked: Result<ManualEditor.AssistAdvice, Error> = Result {
                        guard self.owner === editor else { throw ManualEditorFailure(status: 9, boundary: "Advice") }
                        let advice = try result.get(); try editor.inspectAdvice(advice); return advice
                    }
                    self.lock.lock(); self.assistWorking = false; self.lock.unlock()
                    DispatchQueue.main.async { completion(checked) }
                }
            }
        }
    }
    func useAdvice(_ advice: ManualEditor.AssistAdvice) throws {
        guard let owner else { throw ManualEditorFailure(status: 13, boundary: "Advice") }
        let choice = try owner.inspectAdvice(advice)
        // Existing manual-tool navigation only; no values, preview or Apply.
        try owner.select(choice.toolIndex); try owner.cancelAdvice()
    }
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
    @Published private(set) var assistance = AssistUiState()
    private var assistEpoch: UInt64 = 0
    private var loadingChoices = false
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
            if self.assistance.enabled && self.assistance.choices.isEmpty { self.loadAdviceChoices() }
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
    private func nextAssistEpoch() -> UInt64? {
        guard assistEpoch != UInt64.max else { assistance.status = .failed; assistance.advice = nil; assistance.error = "Advice session ended. Manual editing is available."; return nil }
        assistEpoch += 1; return assistEpoch
    }
    private func invalidateAdvice(_ reason: AssistStatus = .stale) {
        _ = nextAssistEpoch()
        let hadWork = assistance.advice != nil || assistance.inFlight
        assistance.advice = nil; assistance.error = nil; assistance.notice = nil
        assistance.status = assistance.enabled && hadWork ? reason : .idle
        if hadWork { worker.cancelAdvice() }
    }
    func setAssistMode(_ enabled: Bool) {
        invalidateAdvice(.cancelled); assistance.enabled = enabled; assistance.status = .idle
        if enabled { loadAdviceChoices() }
    }
    private func loadAdviceChoices() {
        guard !loadingChoices, assistance.choices.isEmpty, editor != nil else { return }
        loadingChoices = true
        worker.adviceChoices { [weak self] values in
            guard let self else { return }; self.loadingChoices = false
            guard self.assistance.enabled, self.editor != nil else { return }
            self.assistance.choices = values
            if values.isEmpty && self.assistance.enabled { self.assistance.status = .failed; self.assistance.error = "Advice is unavailable. Manual editing is available." }
        }
    }
    func chooseAdvice(_ index: Int) {
        guard !assistance.inFlight else { return }; invalidateAdvice(); assistance.selected = index; assistance.status = .idle
    }
    private func adviceFailure(_ failure: Error) {
        switch (failure as? ManualEditorFailure)?.status {
        case 3, 9: assistance.status = .stale; assistance.error = "This advice has expired. Request new guidance for the current image."
        case 13: assistance.status = .cancelled; assistance.error = "Advice cancelled or stopped. Manual editing is available."
        case 8: assistance.status = .failed; assistance.error = "Advice is busy or resources are limited. You can keep editing manually."
        default: assistance.status = .failed; assistance.error = "Advice could not be validated. You can keep editing manually."
        }
        assistance.advice = nil
    }
    func requestAdvice() {
        guard assistance.enabled, !assistance.inFlight, assistance.choices.indices.contains(assistance.selected), let stamp = nextAssistEpoch() else { return }
        let choice = assistance.choices[assistance.selected]
        assistance.inFlight = true; assistance.status = .requesting; assistance.advice = nil; assistance.error = nil; assistance.notice = nil
        worker.requestAdvice(choice.index) { [weak self] result in
            guard let self else { return }; self.assistance.inFlight = false
            guard self.assistEpoch == stamp, self.assistance.enabled else { return }
            switch result {
            case .success(let advice): self.assistance.advice = advice; self.assistance.status = .proposalAvailable
            case .failure(let failure): self.adviceFailure(failure)
            }
        }
    }
    func cancelAdvice() { invalidateAdvice(.cancelled); assistance.status = assistance.enabled ? .cancelled : .idle }
    func dismissAdvice() { invalidateAdvice(.rejected); assistance.status = assistance.enabled ? .rejected : .idle }
    func useSuggestion() {
        guard let advice = assistance.advice, let stamp = nextAssistEpoch() else { return }
        send { worker in
            do {
                try worker.useAdvice(advice)
                DispatchQueue.main.async { [weak self] in
                    guard let self, self.assistEpoch == stamp else { return }
                    self.assistance.enabled = false; self.assistance.status = .idle; self.assistance.advice = nil
                    self.assistance.notice = "Manual tool opened. Set your preferred value, then choose Apply."
                }
            } catch {
                DispatchQueue.main.async { [weak self] in guard let self, self.assistEpoch == stamp else { return }; self.adviceFailure(error) }
            }
        }
    }
    func reopen() { send { try $0.reopen() } }
    func recover() { invalidateAdvice(); send { $0.close(); try $0.reopen() } }
    func importImage(_ input: URL) { invalidateAdvice(); send { try $0.importImage(input) } }
    func select(_ index: Int) { invalidateAdvice(); send { try $0.select(index) } }
    func update(_ values: [Int32]) { invalidateAdvice(); guard let core else { return }; epoch += 1; previewLoading = true; worker.update(values, editID: core.editID, epoch: epoch) }
    func action(_ action: UInt32) { invalidateAdvice(); send { try $0.action(action) } }
    func interrupt() { invalidateAdvice(.cancelled); send { try $0.action(4) } }
    func close() { setAssistMode(false); send { $0.close() } }
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
            if ProcessInfo.processInfo.arguments.contains("--assist-test-failure") { worker.adviceFixtureFailure() }
        }
    }
    #endif
}
