package ai.pixaura.app

import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.semantics.*
import androidx.compose.ui.unit.dp

@Composable
internal fun AutoProposalPanel(model: ManualEditorModel, ui: EditorUiState, auto: AutoUiState) {
    val approvalDescription = presented("Apply this exact reviewed plan with human approval")
    var choosing by remember { mutableStateOf(false) }
    var drafts by remember(auto.review?.reviewIdentity) { mutableStateOf(emptySet<String>()) }
    val choice = auto.choices.firstOrNull { it.index == auto.selected }
    val tool = choice?.let { value -> ui.tools.firstOrNull { it.index == value.toolIndex } }
    Card(Modifier.fillMaxWidth().testTag("auto-panel")) {
        Column(Modifier.padding(12.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
            Text(presented("AI Auto-Edit"), style = MaterialTheme.typography.titleMedium)
            Text(presented("Local reference planning does not analyze your image. Review exact actions, then choose Apply yourself."), style = MaterialTheme.typography.bodySmall)
            Box {
                OutlinedButton(onClick = { choosing = true }, enabled = !auto.inFlight && auto.choices.isNotEmpty(), modifier = Modifier.testTag("auto-goal")) { Text(presented(tool?.name ?: "Choose adjustment")) }
                DropdownMenu(expanded = choosing, onDismissRequest = { choosing = false }) {
                    for (option in auto.choices) {
                        val descriptor = ui.tools.firstOrNull { it.index == option.toolIndex } ?: continue
                        DropdownMenuItem(text = { Text("${descriptor.name}: ${adviceSetting(descriptor, option.values)}") },
                            onClick = { choosing = false; model.chooseAuto(option.index) }, modifier = Modifier.testTag("auto-goal-${descriptor.id}"))
                    }
                }
            }
            if (tool != null) Text(presentedFormat(R.string.phase11_context_request, presented(tool.name), adviceSetting(tool, choice.values)), Modifier.testTag("auto-choice-value"))
            if (ui.core?.active == true) Text(presented("Finish or cancel your manual adjustment before requesting a proposal."))
            Button(onClick = model::requestAuto, enabled = ui.core != null && !ui.core.active && !ui.busy && !auto.inFlight && tool != null,
                modifier = Modifier.testTag("auto-request")) { Text(presented("Prepare proposal")) }
            if (auto.status in listOf(AutoStatus.REQUESTING, AutoStatus.PROPOSAL_RECEIVED, AutoStatus.VALIDATING)) {
                LinearProgressIndicator(Modifier.fillMaxWidth())
                Text(presented("Requesting and validating proposal…"), Modifier.testTag("auto-loading").semantics { liveRegion = LiveRegionMode.Polite })
                OutlinedButton(onClick = model::cancelAuto, modifier = Modifier.testTag("auto-cancel")) { Text(presented("Cancel proposal")) }
            }
            auto.proposal?.let { proposal ->
                Column(Modifier.testTag("auto-ready"), verticalArrangement = Arrangement.spacedBy(6.dp)) {
                    Text(presented(if (auto.status in listOf(AutoStatus.NO_ACTIONS, AutoStatus.EDIT_FAILED)) "No executable review — restore original or request a new proposal" else if (auto.status == AutoStatus.REVIEW_READY) "Ready for human approval" else "Reviewed plan"), style = MaterialTheme.typography.titleSmall)
                    auto.review?.let { review ->
                        Text(presentedFormat(R.string.phase11_context_context, auto.status.toString(), proposal.providerId), Modifier.testTag("review-context"))
                        ReviewPreviewPanel(model, auto.preview)
                        PlanInspectionPanel(review.inspection)
                        Text(presentedFormat(R.string.phase11_context_source, review.sourceRevision.take(8)), Modifier.testTag("auto-source"))
                        Button(onClick = { model.applyAutoFromHuman(review) }, enabled = drafts.isEmpty() && auto.status == AutoStatus.REVIEW_READY && !auto.inFlight && auto.preview.status !in listOf(ReviewPreviewStatus.PREVIEW_LOADING, ReviewPreviewStatus.STALE),
                            modifier = Modifier.testTag("auto-apply").defaultMinSize(minWidth = 48.dp, minHeight = 48.dp).semantics { contentDescription = approvalDescription }) { Text(stringResource(R.string.apply_reviewed_plan)) }
                    }
                    ReviewEditPanel(model, auto) { key, dirty ->
                        if (model.autoProposal.value.review?.reviewIdentity == auto.review?.reviewIdentity) drafts = if (dirty) drafts + key else drafts - key
                    }
                    OutlinedButton(onClick = model::dismissAuto, enabled = !auto.inFlight, modifier = Modifier.testTag("auto-dismiss")) { Text(presented("Reject proposal")) }
                }
            }
            if (auto.status in listOf(AutoStatus.APPROVING, AutoStatus.EXECUTING)) {
                LinearProgressIndicator(Modifier.fillMaxWidth())
                Text(presented("Approving and applying through the editing engine…"), Modifier.testTag("auto-applying").semantics { liveRegion = LiveRegionMode.Polite })
                OutlinedButton(onClick = model::cancelAuto, enabled = !auto.cancelRequested, modifier = Modifier.testTag("auto-execution-cancel")) { Text(presented("Cancel execution")) }
            }
            if (auto.status == AutoStatus.COMPLETED && auto.receipt != null) {
                Text(presentedFormat(R.string.phase11_context_completed, auto.receipt.resultingRevision.take(8)), Modifier.testTag("auto-completed").semantics { liveRegion = LiveRegionMode.Polite })
            }
            auto.result?.let { ResultReviewPanel(model, it) }
            if (auto.status == AutoStatus.REJECTED) Text(presented("Proposal dismissed. Nothing was applied."), Modifier.testTag("auto-rejected"))
            if (auto.status == AutoStatus.CANCELLED) Text(presented("Proposal cancelled. Nothing was applied."), Modifier.testTag("auto-cancelled"))
            if (auto.status == AutoStatus.STALE) {
                Text(presented("Plan inspection stale. Request a new review."), Modifier.testTag("review-inspection-stale"))
                Text(presented("Proposal expired after an editing change."), Modifier.testTag("auto-stale"))
            }
            auto.error?.let { Text(presented(it), Modifier.testTag("auto-error").semantics { liveRegion = LiveRegionMode.Assertive }) }
            if (auto.status == AutoStatus.FAILED) Text(presented("Plan inspection unavailable. No actionable review; request a new proposal explicitly."), Modifier.testTag("review-inspection-unsupported"))
        }
    }
}
