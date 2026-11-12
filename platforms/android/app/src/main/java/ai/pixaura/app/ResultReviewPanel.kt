package ai.pixaura.app

import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.FlowRow
import androidx.compose.foundation.layout.ExperimentalLayoutApi
import androidx.compose.foundation.layout.defaultMinSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.semantics.LiveRegionMode
import androidx.compose.ui.semantics.contentDescription
import androidx.compose.ui.semantics.liveRegion
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.unit.dp
import androidx.lifecycle.compose.collectAsStateWithLifecycle

@OptIn(ExperimentalLayoutApi::class)
@Composable
internal fun ResultReviewPanel(model: ManualEditorModel, result: ResultReviewUi) {
    val observed = model.autoProposal.collectAsStateWithLifecycle().value.result ?: result
    val label = presented(when (observed.state) {
        1 -> "Committed result"
        2 -> "Result review is stale. Ordinary history changed."
        3 -> "Outcome unconfirmed. Ordinary history remains authoritative."
        4 -> "Receipt revision is absent from ordinary history."
        else -> "No committed result."
    })
    val spoken = presentedFormat(R.string.phase11_result_spoken, label, if (observed.previous.isEmpty()) presented("none") else observed.previous, observed.live, observed.count)
    Column(Modifier.fillMaxWidth().testTag("result-review"), verticalArrangement = Arrangement.spacedBy(6.dp)) {
        Text(presented("Committed result"), style = MaterialTheme.typography.titleSmall)
        Text(label, Modifier.testTag("result-label").semantics { contentDescription = spoken; liveRegion = LiveRegionMode.Polite })
        Text(presentedFormat(R.string.phase11_result_revision, observed.live.take(8), observed.count), Modifier.testTag("result-revision"))
        if (observed.geometryChanged) Text(presented("Geometry changed. Fitted comparison is not a pixel-aligned split."), Modifier.testTag("result-geometry"))
        observed.image?.let {
            val caption = presented(if (observed.showing == "previous") "Previous committed revision" else "Current committed result")
            Image(it.asImageBitmap(), caption, Modifier.fillMaxWidth().height(180.dp).testTag("result-viewport"), contentScale = ContentScale.Fit)
        }
        FlowRow(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
            OutlinedButton(onClick = { model.showResult(true) }, enabled = observed.previous.isNotEmpty(), modifier = Modifier.testTag("result-previous")) { Text(presented("Previous")) }
            OutlinedButton(onClick = { model.showResult(false) }, modifier = Modifier.testTag("result-current")) { Text(presented("Current Result")) }
        }
        FlowRow(Modifier.fillMaxWidth(), horizontalArrangement = Arrangement.spacedBy(8.dp), verticalArrangement = Arrangement.spacedBy(8.dp)) {
            val undoDocument = presented("Undo document")
            val redoDocument = presented("Redo document")
            OutlinedButton(onClick = model::undoResult, enabled = observed.canUndo, modifier = Modifier.testTag("result-undo").defaultMinSize(minWidth = 48.dp, minHeight = 48.dp).semantics { contentDescription = undoDocument }) { Text(undoDocument) }
            OutlinedButton(onClick = model::redoResult, enabled = observed.canRedo, modifier = Modifier.testTag("result-redo").defaultMinSize(minWidth = 48.dp, minHeight = 48.dp).semantics { contentDescription = redoDocument }) { Text(redoDocument) }
        }
        OutlinedButton(onClick = model::backToEditing, modifier = Modifier.testTag("result-back")) { Text(presented("Back to editing")) }
    }
}
