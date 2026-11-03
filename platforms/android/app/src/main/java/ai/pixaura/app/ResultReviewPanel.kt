package ai.pixaura.app

import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
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

@Composable
internal fun ResultReviewPanel(model: ManualEditorModel, result: ResultReviewUi) {
    val label = when (result.state) {
        1 -> "Committed result"
        2 -> "Result review is stale. Ordinary history changed."
        3 -> "Outcome unconfirmed. Ordinary history remains authoritative."
        4 -> "Receipt revision is absent from ordinary history."
        else -> "No committed result."
    }
    val spoken = "$label Previous ${result.previous.ifEmpty { "none" }}. Current ${result.live}. Count ${result.count}."
    Column(Modifier.fillMaxWidth().testTag("result-review"), verticalArrangement = Arrangement.spacedBy(6.dp)) {
        Text("Committed result", style = MaterialTheme.typography.titleSmall)
        Text(label, Modifier.testTag("result-label").semantics { contentDescription = spoken; liveRegion = LiveRegionMode.Polite })
        Text("Ordinary history revision ${result.live.take(8)}. Count ${result.count}.", Modifier.testTag("result-revision"))
        if (result.geometryChanged) Text("Geometry changed. Fitted comparison is not a pixel-aligned split.", Modifier.testTag("result-geometry"))
        result.image?.let {
            val caption = if (result.showing == "previous") "Previous committed revision" else "Current committed result"
            Image(it.asImageBitmap(), caption, Modifier.fillMaxWidth().height(180.dp).testTag("result-viewport"), contentScale = ContentScale.Fit)
        }
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            OutlinedButton(onClick = { model.showResult(true) }, enabled = result.previous.isNotEmpty(), modifier = Modifier.testTag("result-previous")) { Text("Previous") }
            OutlinedButton(onClick = { model.showResult(false) }, modifier = Modifier.testTag("result-current")) { Text("Current Result") }
        }
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            OutlinedButton(onClick = model::undoResult, enabled = result.canUndo, modifier = Modifier.testTag("result-undo").semantics { contentDescription = "Undo document" }) { Text("Undo document") }
            OutlinedButton(onClick = model::redoResult, enabled = result.canRedo, modifier = Modifier.testTag("result-redo").semantics { contentDescription = "Redo document" }) { Text("Redo document") }
        }
        OutlinedButton(onClick = model::backToEditing, modifier = Modifier.testTag("result-back")) { Text("Back to editing") }
    }
}
