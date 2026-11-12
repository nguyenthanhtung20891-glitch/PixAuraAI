package ai.pixaura.app

import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.semantics.*
import androidx.compose.ui.unit.dp

/** Raw integer proposals only. Native validates target, bounds and complete Plan. */
@Composable
internal fun ReviewEditPanel(model: ManualEditorModel, state: AutoUiState, onDraft: (String, Boolean) -> Unit) {
    val review = state.review
    val binding = review?.reviewIdentity ?: state.editSession?.bindingIdentity
    val editable = state.status in listOf(AutoStatus.REVIEW_READY, AutoStatus.NO_ACTIONS, AutoStatus.EDIT_FAILED)
    val enabled = editable && !state.inFlight && state.preview.status !in listOf(ReviewPreviewStatus.PREVIEW_LOADING, ReviewPreviewStatus.STALE)
    Column(Modifier.testTag("review-edit-controls")) {
        val dirty = when (if (state.status == AutoStatus.EDIT_FAILED) 4 else state.editSession?.state) { 2 -> "EDITED"; 3 -> "NO_ACTIONS — nothing to apply"; 4 -> "EDIT_FAILED — restore required"; else -> "ORIGINAL" }
        Text(dirty, Modifier.testTag("review-edit-state"))
        if (editable && state.inFlight) Text(presented("Validating review edit…"), Modifier.testTag("review-edit-loading").semantics { liveRegion = LiveRegionMode.Polite })
        review?.inspection?.actions?.forEachIndexed { index, action ->
            action.parameters.forEachIndexed { parameterIndex, parameter ->
                var text by remember(review.reviewIdentity, index, parameterIndex) { mutableStateOf(parameter.raw.toString()) }
                val label = presentedFormat(R.string.phase11_raw_request, index + 1, review.inspection.actions.size, action.label, parameter.name, parameter.raw, parameter.unit, parameter.minimum, parameter.maximum)
                OutlinedTextField(value = text, onValueChange = { if (it.length <= 11) { text = it; onDraft("$index-$parameterIndex", it.toIntOrNull() != parameter.raw) } }, enabled = enabled, singleLine = true,
                    label = { Text(label) }, modifier = Modifier.testTag("review-edit-value-$index-$parameterIndex").semantics { contentDescription = label })
                if (text.toIntOrNull() != parameter.raw) Text(presented("Unvalidated input — Validate value or revert this field before Apply."), Modifier.testTag("review-edit-draft-$index-$parameterIndex"))
                Button(onClick = { text.toIntOrNull()?.let { model.editReview(1, index, parameterIndex, it, expectedIdentity = review.reviewIdentity) } }, enabled = enabled && text.toIntOrNull() != null,
                    modifier = Modifier.testTag("review-edit-change-$index-$parameterIndex")) { Text(presentedFormat(R.string.phase11_validate_action, index + 1)) }
            }
            OutlinedButton(onClick = { model.editReview(2, index, expectedIdentity = review.reviewIdentity) }, enabled = enabled, modifier = Modifier.testTag("review-edit-remove-$index")) { Text(presentedFormat(R.string.phase11_remove_action, index + 1, action.label)) }
            if (index > 0) OutlinedButton(onClick = { model.editReview(3, index, destination = index - 1, expectedIdentity = review.reviewIdentity) }, enabled = enabled,
                modifier = Modifier.testTag("review-edit-up-$index")) { Text(presentedFormat(R.string.phase11_move_action, index + 1, action.label, index)) }
            if (index + 1 < review.inspection.actions.size) OutlinedButton(onClick = { model.editReview(3, index, destination = index + 1, expectedIdentity = review.reviewIdentity) }, enabled = enabled,
                modifier = Modifier.testTag("review-edit-down-$index")) { Text(presentedFormat(R.string.phase11_move_action, index + 1, action.label, index + 2)) }
        }
        if (state.editSession != null) OutlinedButton(onClick = { binding?.let { model.editReview(4, expectedIdentity = it) } }, enabled = enabled,
            modifier = Modifier.testTag("review-edit-restore").defaultMinSize(minWidth = 48.dp, minHeight = 48.dp)) { Text(presented("Restore original review content — not document undo")) }
    }
}
