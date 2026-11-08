import SwiftUI
import UniformTypeIdentifiers
import PixAuraCore
import UIKit

func presented(_ canonical: String) -> String { String(localized: String.LocalizationValue(canonical)) }

@MainActor
struct ManualEditorScreen: View {
    @StateObject private var model: ManualEditorModel
    private let assistInitially: Bool
    private let autoInitially: Bool
    init(capabilities: AppleCapabilityOwner, assistInitially: Bool = false, autoInitially: Bool = false) {
        self.assistInitially = assistInitially
        self.autoInitially = autoInitially
        _model = StateObject(wrappedValue: ManualEditorModel(capabilities: capabilities))
    }
    @Environment(\.scenePhase) private var scenePhase
    @State private var picking = false
    @State private var category = "geometry"
    @State private var reviewDraftIdentity: String?
    @State private var reviewDrafts: Set<String> = []
    @State private var cropDrafts: [String] = []
    @State private var formatError: String?
    @State private var showingExport = false
    @State private var exportFormat: UInt32 = 1
    @State private var exportQuality = "90"
    @State private var savingFile = false
    @State private var shareItem: ShareItem?
    private let categories = [("geometry", "Geometry"), ("tone_color", "Tone/Color"), ("detail", "Detail"), ("filters_presets", "Filters/Presets")]
    private var selected: ManualTool? {
        guard let index = model.core?.selected, model.tools.indices.contains(index) else { return nil }
        return model.tools[index]
    }
    private var categoryTools: [(offset: Int, element: ManualTool)] {
        model.tools.enumerated().filter { $0.element.category == category }
    }
    private func toolButton(_ index: Int, _ tool: ManualTool) -> some View {
        Button(tool.name) { formatError = nil; model.select(index) }
            .disabled(model.busy || model.core == nil)
            .accessibilityIdentifier(tool.toolID)
            .tint(model.core?.selected == index ? Color.accentColor : Color.secondary)
            .accessibilityAddTraits(model.core?.selected == index ? .isSelected : [])
    }
    private func adviceSetting(_ tool: ManualTool, _ values: [Int32]) -> String {
        zip(tool.parameters, values).map { parameter, value in
            parameter.unit == "millionths_current_extent" ? "\(parameter.name.replacingOccurrences(of: "_ppm", with: "")): \(Double(value) / 10000)%" : parameter.displayValue(value)
        }.joined(separator: ", ")
    }
    private var assistPanel: some View {
        let state = model.assistance
        let choice = state.choices.first { $0.index == state.selected }
        let tool = choice.flatMap { choice in model.tools.indices.contains(choice.toolIndex) ? model.tools[choice.toolIndex] : nil }
        return GroupBox {
            VStack(alignment: .leading, spacing: 8) {
                Text("AI Assist").font(.headline)
                Text("Choose a precise adjustment. Local reference guidance does not analyze your image or apply edits.").font(.caption)
                Menu {
                    ForEach(state.choices, id: \.index) { choice in
                        if model.tools.indices.contains(choice.toolIndex) {
                            let tool = model.tools[choice.toolIndex]
                            Button("\(tool.name) — \(adviceSetting(tool, choice.values))") { model.chooseAdvice(choice.index) }
                                .accessibilityIdentifier("assist-goal-\(tool.toolID)")
                        }
                    }
                } label: { Text(tool?.name ?? "Choose adjustment") }
                    .disabled(state.inFlight || state.choices.isEmpty).accessibilityIdentifier("assist-goal")
                if let tool, let choice { Text("Request: \(tool.name) — \(adviceSetting(tool, choice.values))").accessibilityIdentifier("assist-choice-value") }
                if model.core?.active == true { Text("Finish or cancel your current manual adjustment before requesting advice.").font(.caption) }
                Button("Request advice") { model.requestAdvice() }
                    .disabled(model.core == nil || model.core?.active == true || model.busy || state.inFlight || tool == nil)
                    .accessibilityIdentifier("assist-request")
                if state.status == .requesting {
                    ProgressView("Getting advice…").accessibilityIdentifier("assist-loading")
                    Button("Cancel advice") { model.cancelAdvice() }.accessibilityIdentifier("assist-cancel")
                }
                if let advice = state.advice, model.tools.indices.contains(advice.choice.toolIndex) {
                    let tool = model.tools[advice.choice.toolIndex]
                    VStack(alignment: .leading, spacing: 6) {
                        Text(tool.name).font(.headline)
                        Text(adviceSetting(tool, advice.choice.values))
                        Text("Open the manual tool and choose your value. Nothing has been applied.").font(.caption)
                        HStack {
                            Button("Use suggestion") { model.useSuggestion() }.disabled(model.busy).accessibilityIdentifier("assist-use")
                            Button("Dismiss") { model.dismissAdvice() }.accessibilityIdentifier("assist-dismiss")
                        }
                    }.accessibilityElement(children: .contain).accessibilityIdentifier("assist-recommendation")
                }
                if state.status == .cancelled { Text("Advice cancelled. You can keep editing manually.").accessibilityIdentifier("assist-cancelled") }
                if state.status == .stale { Text("Advice expired after an editing change. Request new guidance when ready.").accessibilityIdentifier("assist-stale") }
                if state.status == .rejected { Text("Advice dismissed. Your image is unchanged.").accessibilityIdentifier("assist-dismissed") }
                if let error = state.error { Text(error).accessibilityIdentifier("assist-error") }
            }.frame(maxWidth: .infinity, alignment: .leading)
        }.accessibilityIdentifier("assist-panel")
    }
    private var autoPanel: some View {
        let state = model.autoProposal
        let choice = state.choices.first { $0.index == state.selected }
        let tool = choice.flatMap { model.tools.indices.contains($0.toolIndex) ? model.tools[$0.toolIndex] : nil }
        return GroupBox {
            VStack(alignment: .leading, spacing: 8) {
                Text("AI Auto-Edit").font(.headline)
                Text("Local reference planning does not analyze your image. Review exact actions, then choose Apply yourself.").font(.caption)
                Menu {
                    ForEach(state.choices, id: \.index) { option in
                        if model.tools.indices.contains(option.toolIndex) {
                            let descriptor = model.tools[option.toolIndex]
                            Button("\(descriptor.name): \(adviceSetting(descriptor, option.values))") { model.chooseAuto(option.index) }
                                .accessibilityIdentifier("auto-goal-\(descriptor.toolID)")
                        }
                    }
                } label: { Text(tool?.name ?? "Choose adjustment") }
                    .disabled(state.inFlight || state.choices.isEmpty).accessibilityIdentifier("auto-goal")
                if let tool, let choice { Text("Request: \(tool.name) — \(adviceSetting(tool, choice.values))").accessibilityIdentifier("auto-choice-value") }
                if model.core?.active == true { Text("Finish or cancel your manual adjustment before requesting a proposal.") }
                Button("Prepare proposal") { model.requestAuto() }
                    .disabled(model.core == nil || model.core?.active == true || model.busy || state.inFlight || tool == nil).accessibilityIdentifier("auto-request")
                if state.status == .requesting || state.status == .proposalReceived || state.status == .validating {
                    ProgressView("Requesting and validating proposal…").accessibilityIdentifier("auto-loading")
                    Button("Cancel proposal") { model.cancelAuto() }.accessibilityIdentifier("auto-cancel")
                }
                if let proposal = state.proposal {
                    VStack(alignment: .leading, spacing: 6) {
                        Text(state.status == .noActions || state.status == .editFailed ? "No executable review — restore original or request a new proposal" : state.status == .reviewReady ? "Ready for human approval" : "Reviewed plan").font(.headline)
                        if let review = state.review {
                            Text("Context: AI Auto-Edit; \(String(describing: state.status)). Provider \(proposal.providerID) (informational only). Planner reference.deterministic.v1.").accessibilityIdentifier("review-context")
                            ReviewComparisonPanel(model: model, state: state.preview)
                            PlanInspectionPanel(inspection: review.inspection)
                            Text("Reviewed source: \(review.sourceRevision.prefix(8)). Apply rechecks the current source.").accessibilityIdentifier("auto-source")
                            Button(String(localized: "Apply reviewed plan")) { model.applyAutoFromHuman(review) }
                                .disabled((reviewDraftIdentity == review.reviewIdentity && !reviewDrafts.isEmpty) || state.inFlight || state.status != .reviewReady || state.preview.status == .previewLoading || state.preview.status == .stale).accessibilityIdentifier("auto-apply")
                                .accessibilityHint("Explicit human approval for this exact reviewed plan")
                        }
                        ReviewEditPanel(model: model, state: state) { review, key, dirty in
                            guard model.autoProposal.review === review else { return }
                            let id = review.reviewIdentity
                            if reviewDraftIdentity != id { reviewDraftIdentity = id; reviewDrafts = [] }
                            if dirty { reviewDrafts.insert(key) } else { reviewDrafts.remove(key) }
                        }
                        Button("Reject proposal") { model.dismissAuto() }.disabled(state.inFlight).accessibilityIdentifier("auto-dismiss")
                    }.accessibilityElement(children: .contain).accessibilityIdentifier("auto-ready")
                }
                if state.status == .rejected { Text("Proposal dismissed. Nothing was applied.").accessibilityIdentifier("auto-rejected") }
                if state.status == .approving || state.status == .executing {
                    ProgressView("Approving and applying through the editing engine…").accessibilityIdentifier("auto-applying")
                    Button("Cancel execution") { model.cancelAuto() }.disabled(state.cancelRequested).accessibilityIdentifier("auto-execution-cancel")
                }
                if state.status == .completed, let receipt = state.receipt {
                    Text("Edit applied. Ordinary revision \(receipt.resultingRevision.prefix(8)). Undo is available in Manual.").accessibilityIdentifier("auto-completed")
                }
                if let result = state.result { ResultReviewPanel(model: model, result: result) }
                if state.status == .cancelled { Text("Proposal cancelled. Nothing was applied.").accessibilityIdentifier("auto-cancelled") }
                if state.status == .stale {
                    Text("Plan inspection stale. Request a new review.").accessibilityIdentifier("review-inspection-stale")
                    Text("Proposal expired after an editing change.").accessibilityIdentifier("auto-stale")
                }
                if let error = state.error { Text(error).accessibilityIdentifier("auto-error") }
                if state.status == .failed { Text("Plan inspection unavailable. No actionable review; request a new proposal explicitly.").accessibilityIdentifier("review-inspection-unsupported") }
            }.frame(maxWidth: .infinity, alignment: .leading)
        }.accessibilityIdentifier("auto-panel")
    }
    private var modeSelector: some View {
        ScrollView(.horizontal) {
            HStack {
                Button("Manual") { model.setAutoMode(false); model.setAssistMode(false) }.accessibilityIdentifier("mode-manual")
                    .tint(model.assistance.enabled || model.autoProposal.enabled ? Color.secondary : Color.accentColor)
                    .accessibilityAddTraits(model.assistance.enabled || model.autoProposal.enabled ? [] : .isSelected)
                Button("AI Assist") { model.setAssistMode(true) }.accessibilityIdentifier("mode-assist")
                    .tint(model.assistance.enabled ? Color.accentColor : Color.secondary)
                    .accessibilityAddTraits(model.assistance.enabled ? .isSelected : [])
                Button("AI Auto-Edit") { model.setAutoMode(true) }.accessibilityIdentifier("mode-auto")
                    .tint(model.autoProposal.enabled ? Color.accentColor : Color.secondary)
                    .accessibilityAddTraits(model.autoProposal.enabled ? .isSelected : [])
            }
        }.accessibilityIdentifier("mode-selector")
    }
    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            if model.assistance.enabled { assistPanel }
            if model.autoProposal.enabled {
                ScrollView { autoPanel }
                    .accessibilityIdentifier("review-scroll")
            }
            if let notice = model.assistance.notice { Text(notice).accessibilityIdentifier("assist-notice") }
            Button(String(localized: "Open JPEG or PNG")) { picking = true }
                .disabled(model.busy || model.exporting)
                .accessibilityIdentifier("open-photo")
                .accessibilityLabel(String(localized: "Open Photo"))
            Group {
                if let image = model.image {
                    Image(decorative: image, scale: 1).resizable().scaledToFit().accessibilityHidden(false).accessibilityLabel("Current manual preview")
                } else {
                    Text(String(localized: "PixAuraAI edits a private copy on this device.")).accessibilityIdentifier("empty-purpose")
                    Text(String(localized: "Open an image to edit locally"))
                }
            }.frame(maxWidth: .infinity).frame(height: 180).accessibilityIdentifier("manual-preview")
            if model.busy { Text("Loading or saving…").accessibilityLabel("Editor busy") }
            if model.previewLoading { Text("Rendering preview; changes are not saved until Apply").accessibilityIdentifier("preview-loading") }
            if !model.autoProposal.enabled { manualControls }
            if model.autoProposal.enabled, let core = model.core { Text(String(format: String(localized: "Committed revisions: %lld"), Int64(core.revisionCount))).accessibilityIdentifier("revision-count") }
        }
        .buttonStyle(.bordered)
        .toolbar {
            // The shell scroll moves this selector off the top after a review
            // scroll. The navigation bar keeps it hittable.
            ToolbarItem(placement: .principal) {
                modeSelector.id(model.autoProposal.enabled || model.assistance.enabled)
            }
            ToolbarItem(placement: .topBarTrailing) {
                Button(String(localized: "Export")) { showingExport = true }
                    .disabled(model.core == nil)
                    .accessibilityIdentifier("export-open")
            }
        }
        .fileImporter(isPresented: $picking, allowedContentTypes: [.jpeg, .png]) { result in
            if case .success(let input) = result { model.importImage(input) }
            if case .failure(let error) = result { model.pickerFailure(error) }
        }
        .sheet(isPresented: $showingExport) { exportSheet }
        .onAppear {
            model.setAutoMode(autoInitially)
            if !autoInitially { model.setAssistMode(assistInitially) }
            #if DEBUG
            if ProcessInfo.processInfo.arguments.contains("--editor-test-image") { model.testImage(); return }
            #endif
            model.reopen()
        }
        .onDisappear { model.close() }
        .onChange(of: scenePhase) { if $0 != .active { model.cancelExport(); model.interrupt() } }
        .onChange(of: model.exportFile) { if $0 != nil { savingFile = true } }
        .onChange(of: model.shareGeneration) { _ in if let url = model.shareURL { shareItem = ShareItem(url: url) } }
        .onChange(of: model.exportStatus) { if let status = $0 { UIAccessibility.post(notification: .announcement, argument: presented(status)) } }
        .onChange(of: model.core?.editID) { _ in cropDrafts = model.core?.values.map(String.init) ?? [] }
        .onChange(of: model.core?.selected) { _ in if let selected { category = selected.category } }
        .onChange(of: model.core?.values) { values in cropDrafts = values?.map(String.init) ?? [] }
        .onChange(of: model.error) { if let error = $0 { UIAccessibility.post(notification: .announcement, argument: presented(error)) } }
        .onChange(of: model.assistance.error) { if let error = $0 { UIAccessibility.post(notification: .announcement, argument: error) } }
    }
    @ViewBuilder private var manualControls: some View {
        ScrollView(.horizontal) {
            HStack {
                ForEach(categories, id: \.0) { key, label in
                    Button(label) { formatError = nil; category = key; if model.core?.active == true { model.action(0) } }
                        .disabled(model.busy)
                        .tint(category == key ? .accentColor : .secondary)
                        .accessibilityAddTraits(category == key ? .isSelected : [])
                }
            }
        }
        if category == "filters_presets" { Text("No presets available. The shipping catalog is empty.") }
        else {
            ScrollView(.horizontal) {
                HStack {
                    ForEach(categoryTools, id: \.offset) { entry in
                        toolButton(entry.offset, entry.element)
                    }
                }
            }
        }
        if let tool = selected, let core = model.core {
            Text(tool.name).font(.headline)
            if tool.parameters.count > 1 {
                ForEach(Array(tool.parameters.enumerated()), id: \.offset) { index, parameter in
                    TextField("\(parameter.name) (\(parameter.minimum)…\(parameter.maximum))", text: Binding(
                        get: { cropDrafts.indices.contains(index) ? cropDrafts[index] : String(core.values[index]) },
                        set: { if cropDrafts.count != core.values.count { cropDrafts = core.values.map(String.init) }; cropDrafts[index] = $0 }
                    )).textFieldStyle(.roundedBorder).accessibilityIdentifier(parameter.name)
                        .disabled(model.busy || core.approvalPending)
                }
                Button("Preview crop") {
                    let draft = cropDrafts.isEmpty ? core.values.map(String.init) : cropDrafts
                    let values = draft.compactMap(Int32.init)
                    if values.count != draft.count { formatError = "Invalid parameter: enter exact whole numbers" }
                    else { formatError = nil; model.update(values) }
                }.disabled(model.busy || core.approvalPending)
                Text("Crop values are millionths of the current extent. Invalid rectangles are rejected.")
            } else {
                let parameter = tool.parameters[0], value = core.values[0]
                Text("\(tool.name): \(parameter.displayValue(value))").accessibilityIdentifier("parameter-value")
                if parameter.unit == "clockwise_quarter_turns" {
                    HStack {
                        ForEach(Array(stride(from: parameter.minimum, through: parameter.maximum, by: parameter.step)), id: \.self) { turn in
                            Button("\(turn * 90)°") { model.update([Int32(turn)]) }.disabled(model.busy || core.approvalPending)
                        }
                    }
                } else {
                    Slider(value: Binding(get: { Double(value) }, set: { model.update([parameter.integerPosition($0)]) }),
                        in: Double(parameter.minimum)...Double(parameter.maximum), step: Double(parameter.step))
                        .disabled(model.busy || core.approvalPending).accessibilityLabel("\(tool.name) value")
                        .accessibilityIdentifier("manual-slider")
                }
            }
            Button("Reset to default") { model.update(tool.parameters.map { Int32($0.defaultValue) }) }
                .disabled(model.busy || core.approvalPending)
        }
        HStack {
            Button(model.core?.approvalPending == true ? String(localized: "Retry Apply") : String(localized: "Apply")) { model.action(1) }
                .disabled(model.core?.active != true || model.busy || model.exporting)
                .accessibilityIdentifier("Apply")
            Button(String(localized: "Cancel")) { formatError = nil; model.action(0) }.disabled(model.core?.active != true || model.busy || model.exporting)
                .accessibilityIdentifier("Cancel")
        }
        HStack {
            Button(String(localized: "Undo")) { model.action(2) }.disabled(model.core?.canUndo != true || model.busy || model.exporting)
                .accessibilityIdentifier("Undo")
            Button(String(localized: "Redo")) { model.action(3) }.disabled(model.core?.canRedo != true || model.busy || model.exporting)
                .accessibilityIdentifier("Redo")
        }
        if let core = model.core { Text(String(format: String(localized: "Committed revisions: %lld"), Int64(core.revisionCount))).accessibilityIdentifier("revision-count") }
        if let error = formatError ?? model.error { Text(presented(error)).accessibilityIdentifier("editor-error") }
        if model.error?.hasPrefix("Preview failure") == true { Button(String(localized: "Retry preview")) { model.retryPreview() } }
        if model.error?.contains("Stale edit") == true { Button(String(localized: "Reopen committed project")) { model.recover() } }
    }
    private var exportSheet: some View {
        exportPanel
            .padding()
            .fileExporter(isPresented: $savingFile, document: exportDocument, contentType: model.exportFileType, defaultFilename: "PixAuraAI") { result in
                savingFile = false
                model.finishFileExport(result)
            }
            .sheet(item: $shareItem) { item in
                ActivityShare(url: item.url) { completed in model.finishShare(completed: completed) }
            }
    }
    private var exportDocument: ExportFile? {
        guard let bytes = model.exportFile else { return nil }
        return ExportFile(bytes: bytes, kind: model.exportFileType)
    }
    private var exportPanel: some View {
        let blocked = model.busy || model.exporting
        return VStack(alignment: .leading, spacing: 8) {
            Text(String(localized: "Export")).font(.headline)
            Text("Saves the committed revision. Export does not change history.").font(.caption)
            HStack {
                Button("JPEG") { exportFormat = 1 }.disabled(blocked).accessibilityIdentifier("JPEG")
                    .tint(exportFormat == 1 ? Color.accentColor : Color.secondary)
                Button("PNG") { exportFormat = 2 }.disabled(blocked).accessibilityIdentifier("PNG")
                    .tint(exportFormat == 2 ? Color.accentColor : Color.secondary)
            }
            if exportFormat == 1 {
                TextField("JPEG quality 1…100", text: $exportQuality)
                    .textFieldStyle(.roundedBorder).disabled(blocked).accessibilityIdentifier("export-quality")
            }
            Button(String(localized: "Save to Photos")) { model.saveToPhotos(format: exportFormat, quality: Int(exportQuality) ?? -1) }
                .disabled(blocked).accessibilityIdentifier("export-save-photos")
            Button(String(localized: "Save to a file")) { model.prepareFileExport(format: exportFormat, quality: Int(exportQuality) ?? -1) }
                .disabled(blocked).accessibilityIdentifier("export-save-file")
            Button(String(localized: "Share")) { model.shareExport(format: exportFormat, quality: Int(exportQuality) ?? -1) }
                .disabled(blocked).accessibilityIdentifier("export-share")
            if model.exporting && model.exportStatus == "Exporting" {
                Button(String(localized: "Cancel export")) { model.cancelExport() }.accessibilityIdentifier("export-cancel")
            }
            if let status = model.exportStatus {
                Text(presented(status)).accessibilityIdentifier("export-status")
            }
        }.accessibilityElement(children: .contain)
    }
}

private struct ShareItem: Identifiable { let id = UUID(); let url: URL }
private struct ExportFile: FileDocument {
    static var readableContentTypes: [UTType] { [.jpeg, .png] }
    var bytes: Data
    var kind: UTType
    init(bytes: Data, kind: UTType) { self.bytes = bytes; self.kind = kind }
    init(configuration _: ReadConfiguration) throws { bytes = Data(); kind = .jpeg }
    func fileWrapper(configuration _: WriteConfiguration) throws -> FileWrapper { FileWrapper(regularFileWithContents: bytes) }
}
private struct ActivityShare: UIViewControllerRepresentable {
    let url: URL
    let done: (Bool) -> Void
    func makeUIViewController(context _: Context) -> UIActivityViewController {
        let controller = UIActivityViewController(activityItems: [url], applicationActivities: nil)
        controller.completionWithItemsHandler = { _, completed, _, _ in done(completed) }
        if let popover = controller.popoverPresentationController {
            popover.sourceView = controller.view
            popover.sourceRect = CGRect(x: 1, y: 1, width: 1, height: 1)
            popover.permittedArrowDirections = []
        }
        return controller
    }
    func updateUIViewController(_: UIActivityViewController, context _: Context) {}
}

@MainActor
private struct ReviewComparisonPanel: View {
    @ObservedObject var model: ManualEditorModel
    let state: ReviewPreviewUi
    @State private var after = false
    @State private var split = false
    @State private var zoom = 1.0
    @State private var panX = 0.0
    @State private var panY = 0.0
    @State private var divider = 0.5
    private func reset() { after = false; split = false; zoom = 1; panX = 0; panY = 0; divider = 0.5 }
    private func picture(_ image: CGImage, _ size: CGSize) -> some View {
        Image(decorative: image, scale: 1, orientation: .up).resizable().aspectRatio(contentMode: .fit)
            .frame(width: size.width, height: size.height).scaleEffect(CGFloat(zoom))
            .offset(x: CGFloat(panX) * size.width * CGFloat(zoom - 1) / 2,
                    y: CGFloat(panY) * size.height * CGFloat(zoom - 1) / 2)
    }
    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Before / After Preview").font(.headline)
            Text("Disposable display projection. Preview does not apply edits or guarantee Apply will succeed.").font(.caption)
            Button("Preview reviewed plan") { model.requestReviewPreview() }
                .disabled(state.status == .previewLoading).accessibilityIdentifier("review-preview-request")
            if state.status == .previewLoading {
                ProgressView("Preparing preview…").accessibilityIdentifier("review-preview-loading")
                Button(String(localized: "Cancel preview")) { model.cancelReviewPreview() }.accessibilityIdentifier("review-preview-cancel")
            }
            if let pair = state.comparison {
                Text(presented(split ? "Before / After Preview split" : after ? "After Preview — not committed" : "Before — bound source revision"))
                    .accessibilityIdentifier(after ? "review-after" : "review-before")
                Text(pair.dimensions).font(.caption)
                GeometryReader { geometry in
                    ZStack {
                        picture(after && !split ? pair.after : pair.before, geometry.size)
                        if split {
                            picture(pair.after, geometry.size).mask(alignment: .trailing) {
                                Rectangle().frame(width: geometry.size.width * CGFloat(1 - divider))
                            }
                        }
                    }.frame(width: geometry.size.width, height: geometry.size.height).clipped()
                }.frame(height: 240).accessibilityIdentifier("review-viewport")
                HStack {
                    Button(after ? "Show Before" : "Show After Preview") { after.toggle(); split = false }.accessibilityIdentifier("review-toggle")
                    Button("Split") { split.toggle() }.disabled(pair.geometryChanged).accessibilityIdentifier("review-split")
                }
                if pair.geometryChanged {
                    Text("Geometry changed. Aspect-preserving fit/toggle is available; spatial split is unavailable.").font(.caption)
                } else {
                    if split { Slider(value: $divider, in: 0...1).accessibilityLabel("Comparison split position").accessibilityIdentifier("review-divider") }
                    Text("Shared view zoom: \(zoom, specifier: "%.1f")×")
                    Slider(value: $zoom, in: 1...4).accessibilityLabel("Comparison zoom").accessibilityIdentifier("review-zoom")
                    Slider(value: $panX, in: -1...1).accessibilityLabel("Comparison horizontal pan").accessibilityIdentifier("review-pan-x")
                    Slider(value: $panY, in: -1...1).accessibilityLabel("Comparison vertical pan").accessibilityIdentifier("review-pan-y")
                }
                Button("Reset comparison to fit") { reset() }.accessibilityIdentifier("review-reset")
            }
            if state.status == .stale { Text("Preview is stale. Request a freshly validated proposal.").accessibilityIdentifier("review-preview-stale") }
            if state.status == .cancelled { Text(presented("Preview cancelled. The Plan remains inspectable if current.")).accessibilityIdentifier("review-preview-cancelled") }
            if let error = state.error { Text(error).accessibilityIdentifier("review-preview-failed") }
        }.accessibilityElement(children: .contain).accessibilityIdentifier("review-preview")
            .onChange(of: state.comparison.map { ObjectIdentifier($0) }) { _ in reset() }
    }
}

// Read-only native records; labels never become execution identities.
private struct ReviewValueControl: View {
    let model: ManualEditorModel
    let expectedIdentity: String
    let action: Int, parameter: Int, label: String, enabled: Bool
    let raw: Int32, onDraft: (Bool) -> Void
    @State private var text: String
    init(model: ManualEditorModel, expectedIdentity: String, action: Int, parameter: Int, label: String, raw: Int32, enabled: Bool, onDraft: @escaping (Bool) -> Void) {
        self.model = model; self.expectedIdentity = expectedIdentity; self.action = action; self.parameter = parameter; self.label = label; self.enabled = enabled; self.raw = raw; self.onDraft = onDraft; _text = State(initialValue: String(raw))
    }
    var body: some View {
        TextField(label, text: Binding(get: { text }, set: { if $0.count <= 11 { text = $0; onDraft(Int32($0) != raw) } }))
            .keyboardType(.numbersAndPunctuation).disabled(!enabled).accessibilityLabel(label).accessibilityIdentifier("review-edit-value-\(action)-\(parameter)")
        if Int32(text) != raw { Text("Unvalidated input — Validate value or revert this field before Apply.").accessibilityIdentifier("review-edit-draft-\(action)-\(parameter)") }
        Button("Validate value for action \(action + 1)") { if let raw = Int32(text) { model.editReview(1, action: UInt32(action), parameter: UInt32(parameter), value: raw, expectedIdentity: expectedIdentity) } }
            .disabled(!enabled || Int32(text) == nil).accessibilityIdentifier("review-edit-change-\(action)-\(parameter)")
    }
}
private struct ReviewEditPanel: View {
    let model: ManualEditorModel, state: AutoUiState
    let onDraft: (ManualEditor.AutoReview, String, Bool) -> Void
    private var editable: Bool { [AutoStatus.reviewReady, .noActions, .editFailed].contains(state.status) }
    private var enabled: Bool { editable && !state.inFlight && state.preview.status != .previewLoading && state.preview.status != .stale }
    private var dirty: String {
        switch state.status == .editFailed ? 4 : state.editSession?.state { case 2: return "EDITED"; case 3: return "NO_ACTIONS — nothing to apply"; case 4: return "EDIT_FAILED — restore required"; default: return "ORIGINAL" }
    }
    var body: some View {
        VStack(alignment: .leading) {
            let binding = state.review?.reviewIdentity ?? state.editSession?.bindingIdentity
            Text(dirty).accessibilityIdentifier("review-edit-state")
            if editable && state.inFlight { Text("Validating review edit…").accessibilityIdentifier("review-edit-loading") }
            if let review = state.review {
                ForEach(Array(review.inspection.actions.enumerated()), id: \.offset) { index, action in
                    ForEach(Array(action.parameters.enumerated()), id: \.offset) { parameterIndex, parameter in
                        ReviewValueControl(model: model, expectedIdentity: review.reviewIdentity, action: index, parameter: parameterIndex,
                            label: "Requested raw integer for Action \(index + 1) of \(review.inspection.actions.count): \(action.label), \(parameter.name); canonical value \(parameter.raw) \(parameter.unit), allowed \(parameter.minimum) to \(parameter.maximum)", raw: parameter.raw, enabled: enabled,
                            onDraft: { onDraft(review, "\(index)-\(parameterIndex)", $0) })
                    }
                    Button("Remove action \(index + 1): \(action.label)") { model.editReview(2, action: UInt32(index), expectedIdentity: review.reviewIdentity) }.disabled(!enabled).accessibilityIdentifier("review-edit-remove-\(index)")
                    if index > 0 {
                        Button("Move action \(index + 1): \(action.label) to position \(index)") { model.editReview(3, action: UInt32(index), destination: UInt32(index - 1), expectedIdentity: review.reviewIdentity) }.disabled(!enabled).accessibilityIdentifier("review-edit-up-\(index)")
                    }
                    if index + 1 < review.inspection.actions.count {
                        Button("Move action \(index + 1): \(action.label) to position \(index + 2)") { model.editReview(3, action: UInt32(index), destination: UInt32(index + 1), expectedIdentity: review.reviewIdentity) }.disabled(!enabled).accessibilityIdentifier("review-edit-down-\(index)")
                    }
                }.id(review.reviewIdentity)
            }
            if state.editSession != nil {
                Button(String(localized: "Restore original review content — not document undo")) { if let binding { model.editReview(4, expectedIdentity: binding) } }.disabled(!enabled).accessibilityIdentifier("review-edit-restore")
            }
        }.accessibilityElement(children: .contain).accessibilityIdentifier("review-edit-controls")
    }
}
private struct ResultReviewPanel: View {
    let model: ManualEditorModel
    let result: ResultReviewUi
    private var label: String {
        switch result.state {
        case 1: return presented("Committed result")
        case 2: return presented("Result review is stale. Ordinary history changed.")
        case 3: return presented("Outcome unconfirmed. Ordinary history remains authoritative.")
        case 4: return presented("Receipt revision is absent from ordinary history.")
        default: return presented("No committed result.")
        }
    }
    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text(String(localized: "Committed result")).font(.headline)
            Text(label).accessibilityIdentifier("result-label")
            Text("Ordinary history revision \(result.live.prefix(8)). Count \(result.count).").accessibilityIdentifier("result-revision")
            if result.geometryChanged { Text("Geometry changed. Fitted comparison is not a pixel-aligned split.").accessibilityIdentifier("result-geometry") }
            if let image = result.image {
                Image(decorative: image, scale: 1).resizable().scaledToFit().frame(maxHeight: 180)
                    .accessibilityLabel(result.showing == "previous" ? "Previous committed revision" : "Current committed result")
                    .accessibilityIdentifier("result-viewport")
            }
            HStack {
                Button("Previous") { model.showResult(previous: true) }.disabled(result.previous.isEmpty).accessibilityIdentifier("result-previous")
                Button("Current Result") { model.showResult(previous: false) }.accessibilityIdentifier("result-current")
            }
            HStack {
                Button(String(localized: "Undo document")) { model.undoResult() }.disabled(!result.canUndo).accessibilityIdentifier("result-undo")
                Button(String(localized: "Redo document")) { model.redoResult() }.disabled(!result.canRedo).accessibilityIdentifier("result-redo")
            }
            Button("Back to editing") { model.backToEditing() }.accessibilityIdentifier("result-back")
        }.accessibilityElement(children: .contain)
            .accessibilityLabel("\(label) Previous \(result.previous.isEmpty ? "none" : result.previous). Current \(result.live). Count \(result.count).")
            .accessibilityIdentifier("result-review")
    }
}
private struct PlanInspectionPanel: View {
    let inspection: ManualEditor.PlanInspection
    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("\(inspection.actions.count) planned action(s). Human approval required; inspection applied nothing.").accessibilityIdentifier("review-plan-count")
            if inspection.warnings & 1 != 0 {
                Text("Multiple actions execute in this exact order. Order matters.").accessibilityIdentifier("review-order-warning")
            }
            ForEach(inspection.actions, id: \.index) { action in
                let identity = "\(action.toolID); operation v\(action.operationVersion); parameters v\(action.parameterVersion)"
                let exact = action.parameters.map { "\($0.name) = \($0.raw) (\($0.unit))" }.joined(separator: "; ")
                let details = action.parameters.map { "\($0.name): range \($0.minimum) to \($0.maximum); reference \($0.neutral)" }.joined(separator: "; ")
                VStack(alignment: .leading, spacing: 3) {
                    Text("Action \(action.index + 1) of \(inspection.actions.count): \(action.label)")
                    Text(identity)
                    Text(exact).accessibilityIdentifier("auto-exact-values-\(action.index)")
                    Text(details).accessibilityIdentifier("review-action-detail-\(action.index)")
                    if action.geometry {
                        Text("Geometry change: dimensions or orientation may change. Use fit/toggle comparison.").accessibilityIdentifier("review-geometry-warning-\(action.index)")
                    }
                }.accessibilityElement(children: .contain)
                    .accessibilityLabel("Action \(action.index + 1) of \(inspection.actions.count): \(action.label); \(identity); \(exact); \(details); \(action.geometry ? "Geometry change" : "No geometry change")")
                    .accessibilityIdentifier("review-action-\(action.index)")
            }
        }.accessibilityElement(children: .contain).accessibilityIdentifier("review-action-list")
    }
}
