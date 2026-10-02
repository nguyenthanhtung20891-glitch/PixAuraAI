package ai.pixaura.app

import ai.pixaura.app.navigation.Destination
import ai.pixaura.app.navigation.EditMode
import ai.pixaura.app.navigation.ShellState
import org.junit.Assert.assertEquals
import org.junit.Test

class ShellStateTest {
    @Test fun navigationPreservesSessionAndBackReturnsHome() {
        val initial = ShellState()
        val editing = initial.openEditor(EditMode.Assisted)
        assertEquals(Destination.Home, initial.destination)
        assertEquals(EditMode.Assisted, editing.navigate(Destination.Settings).back().mode)
        assertEquals(Destination.Home, editing.back().destination)
        assertEquals(EditMode.AI, editing.openEditor(EditMode.AI).mode)
    }

    @Test fun restoreRejectsUnknownRoutesAndModes() {
        assertEquals(ShellState(), ShellState.restore("Import", "unknown"))
        assertEquals(ShellState(Destination.Editor, EditMode.AI), ShellState.restore("Editor", "AI"))
    }
}
