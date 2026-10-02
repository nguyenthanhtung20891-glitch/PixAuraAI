package ai.pixaura.app

import androidx.lifecycle.SavedStateHandle
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import ai.pixaura.app.navigation.Destination
import ai.pixaura.app.navigation.EditMode
import ai.pixaura.app.navigation.ShellState
import ai.pixaura.bridge.CoreProbe
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

class ShellViewModel(private val savedState: SavedStateHandle) : ViewModel() {
    private val mutableState = MutableStateFlow(ShellState.restore(savedState["destination"], savedState["mode"]))
    val state = mutableState.asStateFlow()
    private val mutableProbe = MutableStateFlow("Checking shared core…")
    val probe = mutableProbe.asStateFlow()

    init {
        viewModelScope.launch {
            mutableProbe.value = withContext(Dispatchers.Default) {
                try {
                    val version = CoreProbe().nativeAbiVersion()
                    if (version == 1) "Shared core ready · ABI $version" else "Shared core unavailable"
                } catch (_: LinkageError) {
                    "Shared core unavailable"
                }
            }
        }
    }

    fun navigate(destination: Destination) = update(state.value.navigate(destination))
    fun openEditor(mode: EditMode) = update(state.value.openEditor(mode))
    fun back() = update(state.value.back())

    private fun update(next: ShellState) {
        savedState["destination"] = next.destination.name
        savedState["mode"] = next.mode.name
        mutableState.value = next
    }
}
