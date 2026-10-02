import Foundation
import Combine
import PixAuraCore

@MainActor
final class ShellModel: ObservableObject {
    @Published private(set) var state = ShellState()
    @Published private(set) var probe = "Checking shared core…"

    func checkCore() async {
        let version = await Task.detached(priority: .userInitiated) {
            CoreProbe.abiVersion()
        }.value
        probe = version == 1 ? "Shared core ready · ABI 1" : "Shared core unavailable"
    }

    func navigate(_ destination: Destination) { state.navigate(destination) }
    func openEditor(_ mode: EditMode) { state.openEditor(mode) }
    func back() { state.back() }
    func restore(destination: String, mode: String) {
        state = ShellState.restore(destination: destination, mode: mode)
    }
}
