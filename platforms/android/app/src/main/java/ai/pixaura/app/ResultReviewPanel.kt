package ai.pixaura.app

import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
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
    val spoken = "$label Previous ${observed.previous.ifEmpty { "none" }}. Current ${observed.live}. Count ${observed.count}."
    Column(Modifier.fillMaxWidth().testTag("result-review"), verticalArrangement = Arrangement.spacedBy(6.dp)) {
        Text(presented("Committed result"), style = MaterialTheme.typography.titleSmall)
        Text(label, Modifier.testTag("result-label").semantics { contentDescription = spoken; liveRegion = LiveRegionMode.Polite })
        Text("Ordinary history revision ${observed.live.take(8)}. Count ${observed.count}.", Modifier.testTag("result-revision"))
        if (observed.geometryChanged) Text("Geometry changed. Fitted comparison is not a pixel-aligned split.", Modifier.testTag("result-geometry"))
        observed.image?.let {
            val caption = if (observed.showing == "previous") "Previous committed revision" else "Current committed result"
            Image(it.asImageBitmap(), caption, Modifier.fillMaxWidth().height(180.dp).testTag("result-viewport"), contentScale = ContentScale.Fit)
        }
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            OutlinedButton(onClick = { model.showResult(true) }, enabled = observed.previous.isNotEmpty(), modifier = Modifier.testTag("result-previous")) { Text("Previous") }
            OutlinedButton(onClick = { model.showResult(false) }, modifier = Modifier.testTag("result-current")) { Text("Current Result") }
        }
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            val undoDocument = presented("Undo document")
            val redoDocument = presented("Redo document")
            OutlinedButton(onClick = model::undoResult, enabled = observed.canUndo, modifier = Modifier.testTag("result-undo").defaultMinSize(minWidth = 48.dp, minHeight = 48.dp).semantics { contentDescription = undoDocument }) { Text(undoDocument) }
            OutlinedButton(onClick = model::redoResult, enabled = observed.canRedo, modifier = Modifier.testTag("result-redo").defaultMinSize(minWidth = 48.dp, minHeight = 48.dp).semantics { contentDescription = redoDocument }) { Text(redoDocument) }
        }
        OutlinedButton(onClick = model::backToEditing, modifier = Modifier.testTag("result-back")) { Text("Back to editing") }
    }
}
