package ai.pixaura.app

import ai.pixaura.bridge.ManualTool
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.semantics.*
import androidx.compose.ui.unit.dp

internal fun adviceSetting(tool: ManualTool, values: List<Int>): String =
    tool.parameters.zip(values).joinToString(", ") { (parameter, value) ->
        if (parameter.unit == "millionths_current_extent") "${parameter.name.removeSuffix("_ppm")}: ${value / 10000.0}%"
        else displayValue(value, parameter.unit)
    }

@Composable
internal fun AssistPanel(model: ManualEditorModel, ui: EditorUiState, assist: AssistUiState) {
    var choosing by remember { mutableStateOf(false) }
    val selected = assist.choices.firstOrNull { it.index == assist.selected }
    val selectedTool = selected?.let { choice -> ui.tools.firstOrNull { it.index == choice.toolIndex } }
    Card(Modifier.fillMaxWidth().testTag("assist-panel")) {
        Column(Modifier.padding(12.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
            Text("AI Assist", style = MaterialTheme.typography.titleMedium)
            Text("Choose a precise adjustment. Local reference guidance does not analyze your image or apply edits.", style = MaterialTheme.typography.bodySmall)
            Box {
                OutlinedButton(onClick = { choosing = true }, enabled = !assist.inFlight && assist.choices.isNotEmpty(),
                    modifier = Modifier.testTag("assist-goal")) { Text(selectedTool?.name ?: "Choose adjustment") }
                DropdownMenu(expanded = choosing, onDismissRequest = { choosing = false }) {
                    for (choice in assist.choices) {
                        val tool = ui.tools.firstOrNull { it.index == choice.toolIndex } ?: continue
                        DropdownMenuItem(text = { Column { Text(tool.name); Text(adviceSetting(tool, choice.values), style = MaterialTheme.typography.bodySmall) } },
                            onClick = { choosing = false; model.chooseAdvice(choice.index) }, modifier = Modifier.testTag("assist-goal-${tool.id}"))
                    }
                }
            }
            if (selectedTool != null) Text("Request: ${selectedTool.name} — ${adviceSetting(selectedTool, selected.values)}", Modifier.testTag("assist-choice-value"))
            if (ui.core?.active == true) Text("Finish or cancel your current manual adjustment before requesting advice.", style = MaterialTheme.typography.bodySmall)
            Button(onClick = model::requestAdvice, enabled = ui.core != null && !ui.core.active && !ui.busy && !assist.inFlight && selectedTool != null,
                modifier = Modifier.testTag("assist-request")) { Text("Request advice") }
            if (assist.status == AssistStatus.REQUESTING) {
                LinearProgressIndicator(Modifier.fillMaxWidth())
                Text("Getting advice…", Modifier.testTag("assist-loading").semantics { liveRegion = LiveRegionMode.Polite })
                OutlinedButton(onClick = model::cancelAdvice, modifier = Modifier.testTag("assist-cancel")) { Text("Cancel advice") }
            }
            assist.advice?.let { advice ->
                ui.tools.firstOrNull { it.index == advice.choice.toolIndex }?.let { tool ->
                    Column(Modifier.testTag("assist-recommendation").semantics { contentDescription = "AI Assist recommendation" }, verticalArrangement = Arrangement.spacedBy(6.dp)) {
                        Text(tool.name, style = MaterialTheme.typography.titleSmall)
                        Text(adviceSetting(tool, advice.choice.values))
                        Text("Open the manual tool and choose your value. Nothing has been applied.", style = MaterialTheme.typography.bodySmall)
                        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                            Button(onClick = model::useSuggestion, enabled = !ui.busy, modifier = Modifier.testTag("assist-use")) { Text("Use suggestion") }
                            OutlinedButton(onClick = model::dismissAdvice, modifier = Modifier.testTag("assist-dismiss")) { Text("Dismiss") }
                        }
                    }
                }
            }
            if (assist.status == AssistStatus.CANCELLED) Text("Advice cancelled. You can keep editing manually.", Modifier.testTag("assist-cancelled"))
            if (assist.status == AssistStatus.STALE) Text("Advice expired after an editing change. Request new guidance when ready.", Modifier.testTag("assist-stale"))
            if (assist.status == AssistStatus.REJECTED) Text("Advice dismissed. Your image is unchanged.", Modifier.testTag("assist-dismissed"))
            assist.error?.let { Text(it, Modifier.testTag("assist-error").semantics { liveRegion = LiveRegionMode.Assertive }) }
        }
    }
}
