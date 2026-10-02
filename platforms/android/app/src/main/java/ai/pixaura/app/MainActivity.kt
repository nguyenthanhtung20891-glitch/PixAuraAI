package ai.pixaura.app

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.BackHandler
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import androidx.lifecycle.viewmodel.compose.viewModel
import ai.pixaura.app.navigation.Destination
import ai.pixaura.app.navigation.EditMode
import ai.pixaura.app.ui.PixAuraTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            val model: ShellViewModel = viewModel()
            val state by model.state.collectAsStateWithLifecycle()
            val probe by model.probe.collectAsStateWithLifecycle()
            BackHandler(enabled = state.destination != Destination.Home) { model.back() }
            PixAuraTheme {
                Scaffold { padding ->
                    Column(
                        modifier = Modifier.fillMaxSize().padding(padding).padding(24.dp)
                            .verticalScroll(rememberScrollState()),
                        verticalArrangement = Arrangement.spacedBy(16.dp),
                    ) {
                        Text("PixAuraAI", style = androidx.compose.material3.MaterialTheme.typography.headlineLarge)
                        Text(state.destination.name, style = androidx.compose.material3.MaterialTheme.typography.headlineMedium)
                        Text(probe)
                        when (state.destination) {
                            Destination.Home -> {
                                Text("Local-first photo editing. Application shell preview.")
                                for (mode in EditMode.entries) {
                                    Button(onClick = { model.openEditor(mode) }) { Text("Open ${mode.name} editor") }
                                }
                                Button(onClick = { model.navigate(Destination.Projects) }) { Text("Open Projects") }
                                Button(onClick = { model.navigate(Destination.Settings) }) { Text("Open Settings") }
                            }
                            Destination.Editor -> Text("${state.mode.name} session · Editing tools arrive in a later phase.")
                            Destination.Projects -> Text("Your projects will appear here.")
                            Destination.Settings -> Text("Offline by default. No account, analytics or photo access requested.")
                        }
                        if (state.destination != Destination.Home) {
                            Button(onClick = { model.back() }) { Text("Back to Home") }
                        }
                    }
                }
            }
        }
    }
}
