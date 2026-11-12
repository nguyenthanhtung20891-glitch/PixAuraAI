package ai.pixaura.app

import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.horizontalScroll
import androidx.compose.material3.*
import android.content.Intent
import android.os.Build
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.layout.ContentScale
import androidx.lifecycle.compose.LocalLifecycleOwner
import androidx.compose.ui.semantics.*
import androidx.compose.ui.unit.dp
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.res.stringResource
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver
import androidx.lifecycle.compose.collectAsStateWithLifecycle

@Composable
fun ManualEditorScreen(model: ManualEditorModel, assistInitially: Boolean = false, autoInitially: Boolean = false) {
    val ui by model.state.collectAsStateWithLifecycle()
    val assist by model.assistance.collectAsStateWithLifecycle()
    val auto by model.autoProposal.collectAsStateWithLifecycle()
    var category by remember { mutableStateOf("geometry") }
    var formatError by remember { mutableStateOf<String?>(null) }
    var exportFormat by remember { mutableIntStateOf(1) }
    var exportQuality by remember { mutableStateOf("90") }
    val shareGeneration by model.shareGeneration.collectAsStateWithLifecycle()
    val shareLauncher = rememberLauncherForActivityResult(ActivityResultContracts.StartActivityForResult()) { result -> model.finishShare(result.resultCode) }
    val documentLauncher = rememberLauncherForActivityResult(ActivityResultContracts.CreateDocument("image/*")) { uri ->
        model.saveToDocument(uri, exportFormat, exportQuality.toIntOrNull() ?: -1)
    }
    LaunchedEffect(shareGeneration) {
        val intent = model.takeShareIntent() ?: return@LaunchedEffect
        shareLauncher.launch(Intent.createChooser(intent, "Share photo"))
    }
    val lifecycle = LocalLifecycleOwner.current
    val picker = rememberLauncherForActivityResult(ActivityResultContracts.OpenDocument()) { uri ->
        if (uri != null) model.importImage(uri) else model.noteImportCancelled()
    }
    LaunchedEffect(model, assistInitially, autoInitially) { model.setAutoMode(autoInitially); if (!autoInitially) model.setAssistMode(assistInitially); model.openLast() }
    LaunchedEffect(ui.core?.selected) { ui.tools.firstOrNull { it.index == ui.core?.selected }?.let { category = it.category } }
    DisposableEffect(lifecycle, model) {
        val observer = LifecycleEventObserver { _, event -> if (event == Lifecycle.Event.ON_STOP) { model.cancelExport(); model.interrupt() } }
        lifecycle.lifecycle.addObserver(observer)
        onDispose { lifecycle.lifecycle.removeObserver(observer); model.closeEditor() }
    }
    val previewDescription = presented("Image preview")
    val currentPreviewDescription = presented("Current manual preview")
    val photosDescription = presented("Save the committed revision to Photos")
    val fileDescription = presented("Save the committed revision to a file")
    val shareDescription = presented("Share the committed revision")
    Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
        Row(Modifier.horizontalScroll(rememberScrollState()).testTag("mode-selector"), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            FilterChip(selected = !assist.enabled && !auto.enabled, onClick = { model.setAutoMode(false); model.setAssistMode(false) }, label = { Text(presented("Manual")) }, modifier = Modifier.testTag("mode-manual"))
            FilterChip(selected = assist.enabled, onClick = { model.setAssistMode(true) }, label = { Text(presented("AI Assist")) }, modifier = Modifier.testTag("mode-assist"))
            FilterChip(selected = auto.enabled, onClick = { model.setAutoMode(true) }, label = { Text(presented("AI Auto-Edit")) }, modifier = Modifier.testTag("mode-auto"))
        }
        if (assist.enabled) AssistPanel(model, ui, assist)
        if (auto.enabled) AutoProposalPanel(model, ui, auto)
        assist.notice?.let { Text(it, Modifier.testTag("assist-notice").semantics { liveRegion = LiveRegionMode.Polite }) }
        val openPhoto = stringResource(R.string.open_photo)
        val openVisible = stringResource(R.string.open_jpeg_or_png)
        Button(onClick = { picker.launch(arrayOf("image/jpeg", "image/png")) }, enabled = !ui.busy && !ui.exporting, modifier = Modifier.testTag("open-photo").defaultMinSize(minWidth = 48.dp, minHeight = 48.dp).semantics { contentDescription = openPhoto }) { Text(openVisible) }
        Box(Modifier.fillMaxWidth().height(180.dp).semantics { contentDescription = previewDescription }) {
            ui.image?.let { Image(it.asImageBitmap(), currentPreviewDescription, Modifier.fillMaxSize(), contentScale = ContentScale.Fit) }
            if (ui.image == null) Column {
                Text(stringResource(R.string.empty_purpose), Modifier.testTag("empty-purpose"))
                Text(stringResource(R.string.empty_instruction))
            }
        }
        if (ui.busy) Text(presented("Loading or saving…"), Modifier.semantics { liveRegion = LiveRegionMode.Polite })
        if (ui.previewLoading) Text(presented("Rendering preview; changes are not saved until Apply"), Modifier.semantics { liveRegion = LiveRegionMode.Polite })
        if (!auto.enabled) {
        Row(Modifier.horizontalScroll(rememberScrollState()), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            for ((key, label) in listOf("geometry" to "Geometry", "tone_color" to "Tone/Color", "detail" to "Detail", "filters_presets" to "Filters/Presets")) {
                FilterChip(selected = category == key, enabled = !ui.busy, onClick = { formatError = null; category = key; if (ui.core?.active == true) model.action(0) }, label = { Text(presented(label)) })
            }
        }
        if (category == "filters_presets") Text(presented("No presets available. The shipping catalog is empty."))
        else Row(Modifier.horizontalScroll(rememberScrollState()), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            for (tool in ui.tools.filter { it.category == category }) {
                FilterChip(selected = ui.core?.selected == tool.index, enabled = !ui.busy && ui.core != null,
                    onClick = { formatError = null; model.select(tool.index) }, label = { Text(presented(tool.name)) })
            }
        }
        val core = ui.core
        val tool = ui.tools.firstOrNull { it.index == core?.selected }
        if (tool != null && core != null) {
            Text(presented(tool.name), style = MaterialTheme.typography.titleMedium)
            if (tool.parameters.size > 1) {
                val drafts = remember(tool.index, core.values) { mutableStateListOf<String>().apply { addAll(core.values.map { it.toString() }) } }
                for ((i, p) in tool.parameters.withIndex()) {
                    OutlinedTextField(value = drafts[i], onValueChange = { drafts[i] = it }, enabled = !ui.busy && !core.approvalPending,
                        label = { Text("${p.name} (${p.minimum}…${p.maximum})") }, singleLine = true, modifier = Modifier.fillMaxWidth())
                }
                Button(onClick = {
                    val values = drafts.map { it.toIntOrNull() }
                    if (values.any { it == null }) formatError = "Invalid parameter: enter exact whole numbers"
                    else { formatError = null; model.update(values.filterNotNull()) }
                }, enabled = !ui.busy && !core.approvalPending) { Text(presented("Preview crop")) }
                Text(presented("Crop values are millionths of the current extent. Invalid rectangles are rejected."))
            } else {
                val p = tool.parameters.single(); val value = core.values.single()
                val valueDescription = stringResource(R.string.phase11_tool_value_format, presented(tool.name))
                val formattedValue = displayValue(value, p.unit)
                Text("${presented(tool.name)}: $formattedValue", Modifier.semantics { stateDescription = formattedValue })
                if (p.unit == "clockwise_quarter_turns") {
                    Row(Modifier.horizontalScroll(rememberScrollState()), horizontalArrangement = Arrangement.spacedBy(6.dp)) {
                        for (v in p.minimum..p.maximum step p.step) Button(onClick = { model.update(listOf(v)) }, enabled = !ui.busy && !core.approvalPending) { Text("${v * 90}°") }
                    }
                } else {
                    Slider(value = value.toFloat(), onValueChange = { model.update(listOf(integerPosition(it, p.minimum, p.step))) },
                        valueRange = p.minimum.toFloat()..p.maximum.toFloat(), steps = (p.maximum - p.minimum) / p.step - 1,
                        enabled = !ui.busy && !core.approvalPending, modifier = Modifier.testTag("manual-slider").semantics { contentDescription = valueDescription; stateDescription = formattedValue })
                }
            }
            OutlinedButton(onClick = { model.update(tool.parameters.map { it.default }) }, enabled = !ui.busy && !core.approvalPending) { Text(presented("Reset to default")) }
        }
        Row(Modifier.horizontalScroll(rememberScrollState()), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            Button(onClick = { model.action(1) }, enabled = core?.active == true && !ui.busy && !ui.exporting, modifier = Modifier.defaultMinSize(minWidth = 48.dp, minHeight = 48.dp)) { Text(if (core?.approvalPending == true) stringResource(R.string.retry_apply) else stringResource(R.string.apply)) }
            OutlinedButton(onClick = { model.action(0); formatError = null }, enabled = core?.active == true && !ui.busy && !ui.exporting, modifier = Modifier.defaultMinSize(minWidth = 48.dp, minHeight = 48.dp)) { Text(stringResource(R.string.cancel)) }
        }
        Row(Modifier.horizontalScroll(rememberScrollState()), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            OutlinedButton(onClick = { model.action(2) }, enabled = core?.canUndo == true && !ui.busy && !ui.exporting, modifier = Modifier.defaultMinSize(minWidth = 48.dp, minHeight = 48.dp)) { Text(stringResource(R.string.undo)) }
            OutlinedButton(onClick = { model.action(3) }, enabled = core?.canRedo == true && !ui.busy && !ui.exporting, modifier = Modifier.defaultMinSize(minWidth = 48.dp, minHeight = 48.dp)) { Text(stringResource(R.string.redo)) }
        }
        core?.let { Text(stringResource(R.string.committed_revisions, it.revisionCount)) }
        (formatError ?: ui.error)?.let { val shown = presented(it); Text(shown, Modifier.semantics { liveRegion = LiveRegionMode.Assertive }) }
        if (ui.error?.startsWith("Preview failure") == true) Button(onClick = model::retryPreview) { Text(stringResource(R.string.retry_preview)) }
        if (ui.error?.contains("Stale edit") == true) Button(onClick = model::reopenProject) { Text(stringResource(R.string.reopen_project)) }
        }
        if (auto.enabled) ui.core?.let { Text(stringResource(R.string.committed_revisions, it.revisionCount)) }
        if (ui.core != null) {
            Text(presented("Export"), style = MaterialTheme.typography.titleMedium)
            Row(Modifier.horizontalScroll(rememberScrollState()), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                FilterChip(selected = exportFormat == 1, onClick = { exportFormat = 1 }, enabled = !ui.busy && !ui.exporting, label = { Text("JPEG") })
                FilterChip(selected = exportFormat == 2, onClick = { exportFormat = 2 }, enabled = !ui.busy && !ui.exporting, label = { Text("PNG") })
            }
            if (exportFormat == 1) OutlinedTextField(exportQuality, { exportQuality = it.filter(Char::isDigit).take(3) }, enabled = !ui.busy && !ui.exporting,
                label = { Text(presented("JPEG quality 1…100")) }, singleLine = true, modifier = Modifier.fillMaxWidth().testTag("export-quality"))
            Row(Modifier.horizontalScroll(rememberScrollState()), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                if (Build.VERSION.SDK_INT >= 29) Button(onClick = { model.saveToPhotos(exportFormat, exportQuality.toIntOrNull() ?: -1) },
                    enabled = !ui.busy && !ui.exporting, modifier = Modifier.testTag("export-save-photos").defaultMinSize(minWidth = 48.dp, minHeight = 48.dp).semantics { contentDescription = photosDescription }) { Text(presented("Save to Photos")) }
                Button(onClick = { if (model.reserveExport()) documentLauncher.launch(if (exportFormat == 1) "PixAuraAI.jpg" else "PixAuraAI.png") },
                    enabled = !ui.busy && !ui.exporting, modifier = Modifier.testTag("export-save-file").defaultMinSize(minWidth = 48.dp, minHeight = 48.dp).semantics { contentDescription = fileDescription }) { Text(presented("Save to a file")) }
                Button(onClick = { model.shareExport(exportFormat, exportQuality.toIntOrNull() ?: -1) },
                    enabled = !ui.busy && !ui.exporting, modifier = Modifier.testTag("export-share").defaultMinSize(minWidth = 48.dp, minHeight = 48.dp).semantics { contentDescription = shareDescription }) { Text(presented("Share")) }
            }
            if (ui.exporting && ui.exportStatus == "Exporting") OutlinedButton(onClick = model::cancelExport, modifier = Modifier.testTag("export-cancel").defaultMinSize(minWidth = 48.dp, minHeight = 48.dp)) { Text(presented("Cancel export")) }
            ui.exportStatus?.let { val shown = presented(it); Text(shown, Modifier.testTag("export-status").semantics { liveRegion = LiveRegionMode.Polite; contentDescription = shown }) }
        }
    }
}
