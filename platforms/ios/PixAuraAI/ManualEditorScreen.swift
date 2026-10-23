import SwiftUI
import UniformTypeIdentifiers
import PixAuraCore
import UIKit

@MainActor
struct ManualEditorScreen: View {
    @StateObject private var model: ManualEditorModel
    private let assistInitially: Bool
    init(capabilities: AppleCapabilityOwner, assistInitially: Bool = false) {
        self.assistInitially = assistInitially
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
    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            ScrollView(.horizontal) {
                HStack {
                    Button("Manual") { model.setAssistMode(false) }.accessibilityIdentifier("mode-manual")
                        .tint(model.assistance.enabled ? Color.secondary : Color.accentColor)
                        .accessibilityAddTraits(model.assistance.enabled ? [] : .isSelected)
                    Button("AI Assist") { model.setAssistMode(true) }.accessibilityIdentifier("mode-assist")
                        .tint(model.assistance.enabled ? Color.accentColor : Color.secondary)
                        .accessibilityAddTraits(model.assistance.enabled ? .isSelected : [])
                    Button("AI Auto-Edit") { }.disabled(true).accessibilityLabel("AI Auto-Edit unavailable").accessibilityIdentifier("mode-auto")
                }
            }.accessibilityIdentifier("mode-selector")
            if model.assistance.enabled { assistPanel }
            if let notice = model.assistance.notice { Text(notice).accessibilityIdentifier("assist-notice") }
            Button("Open JPEG or PNG") { picking = true }.disabled(model.busy)
            Group {
                if let image = model.image {
                    Image(decorative: image, scale: 1).resizable().scaledToFit().accessibilityHidden(false).accessibilityLabel("Current manual preview")
                } else { Text("Open an image to edit locally") }
            }.frame(maxWidth: .infinity).frame(height: 180).accessibilityIdentifier("manual-preview")
            if model.busy { Text("Loading or saving…").accessibilityLabel("Editor busy") }
            if model.previewLoading { Text("Rendering preview; changes are not saved until Apply").accessibilityIdentifier("preview-loading") }
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
        .buttonStyle(.bordered)
        .fileImporter(isPresented: $picking, allowedContentTypes: [.jpeg, .png]) { result in
            if case .success(let input) = result { model.importImage(input) }
            if case .failure(let error) = result { model.pickerFailure(error) }
        }
        .onAppear {
            model.setAssistMode(assistInitially)
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
