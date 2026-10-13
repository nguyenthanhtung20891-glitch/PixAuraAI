import SwiftUI
import PixAuraCore

@main
struct PixAuraApp: App {
    private let capabilities = AppleCapabilityOwner()
    @Environment(\.scenePhase) private var scenePhase
    var body: some Scene {
        WindowGroup { ShellView(capabilities: capabilities)
            .onChange(of: scenePhase) { capabilities.setConstrained($0 != .active) }
            .task { capabilities.setConstrained(scenePhase != .active) }
        }
    }
}
