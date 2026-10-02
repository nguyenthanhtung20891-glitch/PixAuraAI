import Foundation

enum Destination: String, CaseIterable {
    case home = "Home", editor = "Editor", projects = "Projects", settings = "Settings"
}

enum EditMode: String, CaseIterable {
    case manual = "Manual", assisted = "Assisted", ai = "AI"
}

struct ShellState: Equatable {
    var destination: Destination = .home
    var mode: EditMode = .manual

    mutating func navigate(_ destination: Destination) { self.destination = destination }
    mutating func openEditor(_ mode: EditMode) {
        self.mode = mode
        destination = .editor
    }
    mutating func back() { destination = .home }

    static func restore(destination: String?, mode: String?) -> ShellState {
        ShellState(destination: Destination(rawValue: destination ?? "") ?? .home,
                   mode: EditMode(rawValue: mode ?? "") ?? .manual)
    }
}
