package ai.pixaura.app

import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.semantics.*
import androidx.compose.ui.unit.dp

@Composable
internal fun AutoProposalPanel(model: ManualEditorModel, ui: EditorUiState, auto: AutoUiState) {
    var choosing by remember { mutableStateOf(false) }
    val choice = auto.choices.firstOrNull { it.index == auto.selected }
    val tool = choice?.let { value -> ui.tools.firstOrNull { it.index == value.toolIndex } }
    Card(Modifier.fillMaxWidth().testTag("auto-panel")) {
        Column(Modifier.padding(12.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
            Text("AI Auto-Edit", style = MaterialTheme.typography.titleMedium)
            Text("Prepare a proposal only. Local reference planning does not analyze your image. Nothing is applied.", style = MaterialTheme.typography.bodySmall)
            Box {
                OutlinedButton(onClick = { choosing = true }, enabled = !auto.inFlight && auto.choices.isNotEmpty(), modifier = Modifier.testTag("auto-goal")) { Text(tool?.name ?: "Choose adjustment") }
                DropdownMenu(expanded = choosing, onDismissRequest = { choosing = false }) {
                    for (option in auto.choices) {
                        val descriptor = ui.tools.firstOrNull { it.index == option.toolIndex } ?: continue
                        DropdownMenuItem(text = { Text("${descriptor.name}: ${adviceSetting(descriptor, option.values)}") },
                            onClick = { choosing = false; model.chooseAuto(option.index) }, modifier = Modifier.testTag("auto-goal-${descriptor.id}"))
                    }
                }
            }
            if (tool != null) Text("Request: ${tool.name} — ${adviceSetting(tool, choice.values)}", Modifier.testTag("auto-choice-value"))
            if (ui.core?.active == true) Text("Finish or cancel your manual adjustment before requesting a proposal.")
            Button(onClick = model::requestAuto, enabled = ui.core != null && !ui.core.active && !ui.busy && !auto.inFlight && tool != null,
                modifier = Modifier.testTag("auto-request")) { Text("Prepare proposal") }
            if (auto.status in listOf(AutoStatus.REQUESTING, AutoStatus.PROPOSAL_RECEIVED, AutoStatus.VALIDATING)) {
                LinearProgressIndicator(Modifier.fillMaxWidth())
                Text("Requesting and validating proposal…", Modifier.testTag("auto-loading").semantics { liveRegion = LiveRegionMode.Polite })
                OutlinedButton(onClick = model::cancelAuto, modifier = Modifier.testTag("auto-cancel")) { Text("Cancel proposal") }
            }
            auto.proposal?.let { proposal ->
                Column(Modifier.testTag("auto-ready"), verticalArrangement = Arrangement.spacedBy(6.dp)) {
                    Text("Proposal ready", style = MaterialTheme.typography.titleSmall)
                    Text("${proposal.actions.size} planned action(s). Not approved. Nothing has been applied.")
                    for (action in proposal.actions) ui.tools.firstOrNull { it.index == action.toolIndex }?.let { descriptor ->
                        Text("${action.index + 1}. ${descriptor.name}: ${adviceSetting(descriptor, action.values)}")
                    }
                    Text("Review and approval controls are not available yet.", style = MaterialTheme.typography.bodySmall)
                    OutlinedButton(onClick = model::dismissAuto, modifier = Modifier.testTag("auto-dismiss")) { Text("Dismiss proposal") }
                }
            }
            if (auto.status == AutoStatus.REJECTED) Text("Proposal dismissed. Nothing was applied.", Modifier.testTag("auto-rejected"))
            if (auto.status == AutoStatus.CANCELLED) Text("Proposal cancelled. Nothing was applied.", Modifier.testTag("auto-cancelled"))
            if (auto.status == AutoStatus.STALE) Text("Proposal expired after an editing change.", Modifier.testTag("auto-stale"))
            auto.error?.let { Text(it, Modifier.testTag("auto-error").semantics { liveRegion = LiveRegionMode.Assertive }) }
        }
    }
}
