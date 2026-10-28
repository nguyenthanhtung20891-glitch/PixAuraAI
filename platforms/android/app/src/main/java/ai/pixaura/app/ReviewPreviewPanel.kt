package ai.pixaura.app

import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clipToBounds
import androidx.compose.ui.draw.drawWithContent
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.graphics.drawscope.clipRect
import androidx.compose.ui.graphics.graphicsLayer
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.layout.onSizeChanged
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.semantics.*
import androidx.compose.ui.unit.IntSize
import androidx.compose.ui.unit.dp

@Composable
internal fun ReviewPreviewPanel(model: ManualEditorModel, state: ReviewPreviewUi) {
    var after by remember(state.comparison) { mutableStateOf(false) }
    var split by remember(state.comparison) { mutableStateOf(false) }
    var zoom by remember(state.comparison) { mutableFloatStateOf(1f) }
    var panX by remember(state.comparison) { mutableFloatStateOf(0f) }
    var panY by remember(state.comparison) { mutableFloatStateOf(0f) }
    var divider by remember(state.comparison) { mutableFloatStateOf(0.5f) }
    var extent by remember { mutableStateOf(IntSize.Zero) }
    Column(Modifier.fillMaxWidth().testTag("review-preview"), verticalArrangement = Arrangement.spacedBy(6.dp)) {
        Text("Before / After Preview", style = MaterialTheme.typography.titleSmall)
        Text("Disposable display projection. Preview does not apply edits or guarantee Apply will succeed.", style = MaterialTheme.typography.bodySmall)
        Button(onClick = model::requestReviewPreview, enabled = state.status != ReviewPreviewStatus.PREVIEW_LOADING,
            modifier = Modifier.testTag("review-preview-request")) { Text("Preview reviewed plan") }
        if (state.status == ReviewPreviewStatus.PREVIEW_LOADING) {
            LinearProgressIndicator(Modifier.fillMaxWidth())
            Text("Preparing preview…", Modifier.testTag("review-preview-loading").semantics { liveRegion = LiveRegionMode.Polite })
            OutlinedButton(onClick = { model.cancelReviewPreview() }, modifier = Modifier.testTag("review-preview-cancel")) { Text("Cancel preview") }
        }
        state.comparison?.let { pair ->
            Text(if (split) "Before / After Preview split" else if (after) "After Preview — not committed" else "Before — bound source revision",
                Modifier.testTag(if (after) "review-after" else "review-before").semantics { liveRegion = LiveRegionMode.Polite })
            Text(pair.dimensions, style = MaterialTheme.typography.bodySmall)
            Box(Modifier.fillMaxWidth().height(240.dp).clipToBounds().onSizeChanged { extent = it }.testTag("review-viewport")) {
                val transform = Modifier.matchParentSize().graphicsLayer {
                    scaleX = zoom; scaleY = zoom
                    translationX = panX * extent.width * (zoom - 1f) / 2f
                    translationY = panY * extent.height * (zoom - 1f) / 2f
                }
                Image((if (after && !split) pair.after else pair.before).asImageBitmap(),
                    if (after && !split) "After Preview" else "Before", transform, contentScale = ContentScale.Fit)
                if (split) Box(Modifier.matchParentSize().drawWithContent { clipRect(left = size.width * divider) { this@drawWithContent.drawContent() } }) {
                    Image(pair.after.asImageBitmap(), "After Preview split", transform, contentScale = ContentScale.Fit)
                }
            }
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                OutlinedButton(onClick = { after = !after; split = false }, modifier = Modifier.testTag("review-toggle")) { Text(if (after) "Show Before" else "Show After Preview") }
                OutlinedButton(onClick = { split = !split }, enabled = !pair.geometryChanged, modifier = Modifier.testTag("review-split")) { Text("Split") }
            }
            if (pair.geometryChanged) Text("Geometry changed. Aspect-preserving fit/toggle is available; spatial split is unavailable.")
            else {
                if (split) Slider(divider, { divider = it }, valueRange = 0f..1f, modifier = Modifier.testTag("review-divider").semantics { contentDescription = "Comparison split position" })
                Text("Shared view zoom: ${zoom.toInt()}×")
                Slider(zoom, { zoom = it }, valueRange = 1f..4f, modifier = Modifier.testTag("review-zoom").semantics { contentDescription = "Comparison zoom" })
                Slider(panX, { panX = it }, valueRange = -1f..1f, modifier = Modifier.testTag("review-pan-x").semantics { contentDescription = "Comparison horizontal pan" })
                Slider(panY, { panY = it }, valueRange = -1f..1f, modifier = Modifier.testTag("review-pan-y").semantics { contentDescription = "Comparison vertical pan" })
            }
            OutlinedButton(onClick = { zoom = 1f; panX = 0f; panY = 0f; divider = 0.5f; split = false; after = false }, modifier = Modifier.testTag("review-reset")) { Text("Reset comparison to fit") }
        }
        if (state.status == ReviewPreviewStatus.STALE) Text("Preview is stale. Request a freshly validated proposal.", Modifier.testTag("review-preview-stale"))
        if (state.status == ReviewPreviewStatus.CANCELLED) Text("Preview cancelled. The Plan remains inspectable if current.", Modifier.testTag("review-preview-cancelled"))
        state.error?.let { Text(it, Modifier.testTag("review-preview-failed").semantics { liveRegion = LiveRegionMode.Polite }) }
    }
}
