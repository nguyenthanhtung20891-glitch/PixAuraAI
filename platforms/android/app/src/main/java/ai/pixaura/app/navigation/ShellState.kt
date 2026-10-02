package ai.pixaura.app.navigation

enum class Destination { Home, Editor, Projects, Settings }
enum class EditMode { Manual, Assisted, AI }

// A session is presentation state only; Phase 2 owns real project identity/history.
data class ShellState(
    val destination: Destination = Destination.Home,
    val mode: EditMode = EditMode.Manual,
) {
    fun navigate(destination: Destination) = copy(destination = destination)
    fun openEditor(mode: EditMode) = copy(destination = Destination.Editor, mode = mode)
    fun back() = copy(destination = Destination.Home)

    companion object {
        fun restore(destination: String?, mode: String?) = ShellState(
            Destination.entries.firstOrNull { it.name == destination } ?: Destination.Home,
            EditMode.entries.firstOrNull { it.name == mode } ?: EditMode.Manual,
        )
    }
}
