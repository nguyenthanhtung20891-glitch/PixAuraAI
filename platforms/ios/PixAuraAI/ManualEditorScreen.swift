import SwiftUI
import UniformTypeIdentifiers
import PixAuraCore
import UIKit

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
    @State private var cropDrafts: [String] = []
    @State private var formatError: String?
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
                        Text(state.status == .reviewReady ? "Ready for human approval" : "Reviewed plan").font(.headline)
                        Text("\(proposal.actions.count) planned action(s). " + (state.status == .reviewReady ? "Human approval required; proposal creation applied nothing." : "Awaiting the authoritative native publication result."))
                        ForEach(proposal.actions, id: \.index) { action in
                            if model.tools.indices.contains(action.toolIndex) {
                                let descriptor = model.tools[action.toolIndex]
                                Text("\(action.index + 1). \(descriptor.name): \(adviceSetting(descriptor, action.values))")
                                Text(zip(descriptor.parameters, action.values).map { parameter, value in "\(parameter.name) = \(value) (\(parameter.unit))" }.joined(separator: "; "))
                                    .accessibilityIdentifier("auto-exact-values-\(action.index)")
                            }
                        }
                        if let review = state.review {
                            ReviewComparisonPanel(model: model, state: state.preview)
                            Text("Reviewed source: \(review.sourceRevision.prefix(8)). Apply rechecks the current source.").accessibilityIdentifier("auto-source")
                            Button("Apply reviewed plan") { model.applyAutoFromHuman(review) }
                                .disabled(state.inFlight || state.status != .reviewReady || state.preview.status == .previewLoading || state.preview.status == .stale).accessibilityIdentifier("auto-apply")
                                .accessibilityHint("Explicit human approval for this exact reviewed plan")
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
                if state.status == .cancelled { Text("Proposal cancelled. Nothing was applied.").accessibilityIdentifier("auto-cancelled") }
                if state.status == .stale { Text("Proposal expired after an editing change.").accessibilityIdentifier("auto-stale") }
                if let error = state.error { Text(error).accessibilityIdentifier("auto-error") }
            }.frame(maxWidth: .infinity, alignment: .leading)
        }.accessibilityIdentifier("auto-panel")
    }
    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
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
            if model.assistance.enabled { assistPanel }
            if model.autoProposal.enabled { autoPanel }
            if let notice = model.assistance.notice { Text(notice).accessibilityIdentifier("assist-notice") }
            Button("Open JPEG or PNG") { picking = true }.disabled(model.busy)
            Group {
                if let image = model.image {
                    Image(decorative: image, scale: 1).resizable().scaledToFit().accessibilityHidden(false).accessibilityLabel("Current manual preview")
                } else { Text("Open an image to edit locally") }
            }.frame(maxWidth: .infinity).frame(height: 180).accessibilityIdentifier("manual-preview")
            if model.busy { Text("Loading or saving…").accessibilityLabel("Editor busy") }
            if model.previewLoading { Text("Rendering preview; changes are not saved until Apply").accessibilityIdentifier("preview-loading") }
            if !model.autoProposal.enabled {
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
                Button(model.core?.approvalPending == true ? "Retry Apply" : "Apply") { model.action(1) }
                    .disabled(model.core?.active != true || model.busy)
                Button("Cancel") { formatError = nil; model.action(0) }.disabled(model.core?.active != true || model.busy)
            }
            HStack {
                Button("Undo") { model.action(2) }.disabled(model.core?.canUndo != true || model.busy)
                Button("Redo") { model.action(3) }.disabled(model.core?.canRedo != true || model.busy)
            }
            if let core = model.core { Text("Committed revisions: \(core.revisionCount)").accessibilityIdentifier("revision-count") }
            if let error = formatError ?? model.error { Text(error).accessibilityIdentifier("editor-error") }
            if model.error?.hasPrefix("Preview failure") == true { Button("Retry preview") { model.retryPreview() } }
            if model.error?.contains("Stale edit") == true { Button("Reopen committed project") { model.recover() } }
            }
            if model.autoProposal.enabled, let core = model.core { Text("Committed revisions: \(core.revisionCount)").accessibilityIdentifier("revision-count") }
        }
        .buttonStyle(.bordered)
        .fileImporter(isPresented: $picking, allowedContentTypes: [.jpeg, .png]) { result in
            if case .success(let input) = result { model.importImage(input) }
            if case .failure(let error) = result { model.pickerFailure(error) }
        }
        .onAppear {
            model.setAutoMode(autoInitially)
            if !autoInitially { model.setAssistMode(assistInitially) }
            #if DEBUG
            if ProcessInfo.processInfo.arguments.contains("--editor-test-image") { model.testImage(); return }
            #endif
            model.reopen()
        }
        .onDisappear { model.close() }
        .onChange(of: scenePhase) { if $0 != .active { model.interrupt() } }
        .onChange(of: model.core?.editID) { _ in cropDrafts = model.core?.values.map(String.init) ?? [] }
        .onChange(of: model.core?.selected) { _ in if let selected { category = selected.category } }
        .onChange(of: model.core?.values) { values in cropDrafts = values?.map(String.init) ?? [] }
        .onChange(of: model.error) { if let error = $0 { UIAccessibility.post(notification: .announcement, argument: error) } }
        .onChange(of: model.assistance.error) { if let error = $0 { UIAccessibility.post(notification: .announcement, argument: error) } }
    }
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
                Button("Cancel preview") { model.cancelReviewPreview() }.accessibilityIdentifier("review-preview-cancel")
            }
            if let pair = state.comparison {
                Text(split ? "Before / After Preview split" : after ? "After Preview — not committed" : "Before — bound source revision")
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
            if state.status == .cancelled { Text("Preview cancelled. The Plan remains inspectable if current.").accessibilityIdentifier("review-preview-cancelled") }
            if let error = state.error { Text(error).accessibilityIdentifier("review-preview-failed") }
        }.accessibilityElement(children: .contain).accessibilityIdentifier("review-preview")
            .onChange(of: state.comparison.map { ObjectIdentifier($0) }) { _ in reset() }
    }
}
