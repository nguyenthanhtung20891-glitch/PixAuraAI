import SwiftUI

@MainActor
struct ShellView: View {
    @StateObject private var model = ShellModel()
    @SceneStorage("destination") private var savedDestination = Destination.home.rawValue
    @SceneStorage("mode") private var savedMode = EditMode.manual.rawValue

    var body: some View {
        NavigationStack(path: Binding(
            get: { model.state.destination == .home ? [] : [model.state.destination] },
            set: { model.navigate($0.last ?? .home) }
        )) {
            ScrollView {
                VStack(alignment: .leading, spacing: 16) {
                    Text("Local-first photo editing. Application shell preview.")
                    Text(model.probe).accessibilityIdentifier("core-status")
                    ForEach(EditMode.allCases, id: \.self) { mode in
                        Button("Open \(mode.rawValue) editor") { model.openEditor(mode) }
                    }
                    Button("Open Projects") { model.navigate(.projects) }
                    Button("Open Settings") { model.navigate(.settings) }
                }
                .padding()
                .buttonStyle(.bordered)
                .controlSize(.large)
            }
            .navigationTitle("PixAuraAI")
            .navigationDestination(for: Destination.self) { destination in
                ScrollView {
                    VStack(alignment: .leading, spacing: 16) {
                        Text(model.probe)
                        switch destination {
                        case .editor:
                            Text("\(model.state.mode.rawValue) session · Editing tools arrive in a later phase.")
                        case .projects:
                            Text("Your projects will appear here.")
                        case .settings:
                            Text("Offline by default. No account, analytics or photo access requested.")
                        case .home:
                            Text("Home")
                        }
                        Button("Back to Home") { model.back() }
                            .buttonStyle(.bordered).controlSize(.large)
                    }.padding()
                }
                .navigationTitle(destination.rawValue)
            }
        }
        .task {
            #if DEBUG
            if ProcessInfo.processInfo.arguments.contains("--reset-shell-navigation") {
                savedDestination = Destination.home.rawValue
                savedMode = EditMode.manual.rawValue
            }
            #endif
            model.restore(destination: savedDestination, mode: savedMode)
            await model.checkCore()
        }
        .onChange(of: model.state.destination) { savedDestination = $0.rawValue }
        .onChange(of: model.state.mode) { savedMode = $0.rawValue }
    }
}
