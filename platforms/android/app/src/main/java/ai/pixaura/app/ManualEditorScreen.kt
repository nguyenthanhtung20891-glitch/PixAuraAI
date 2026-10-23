package ai.pixaura.app

import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.Image
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.horizontalScroll
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.asImageBitmap
import androidx.compose.ui.layout.ContentScale
import androidx.lifecycle.compose.LocalLifecycleOwner
import androidx.compose.ui.semantics.*
import androidx.compose.ui.unit.dp
import androidx.compose.ui.platform.testTag
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver
import androidx.lifecycle.compose.collectAsStateWithLifecycle

@Composable
fun ManualEditorScreen(model: ManualEditorModel, assistInitially: Boolean = false) {
    val ui by model.state.collectAsStateWithLifecycle()
    val assist by model.assistance.collectAsStateWithLifecycle()
    var category by remember { mutableStateOf("geometry") }
    var formatError by remember { mutableStateOf<String?>(null) }
    val lifecycle = LocalLifecycleOwner.current
    val picker = rememberLauncherForActivityResult(ActivityResultContracts.OpenDocument()) { uri ->
        if (uri != null) model.importImage(uri)
    }
    LaunchedEffect(model, assistInitially) { model.setAssistMode(assistInitially); model.openLast() }
    LaunchedEffect(ui.core?.selected) { ui.tools.firstOrNull { it.index == ui.core?.selected }?.let { category = it.category } }
    DisposableEffect(lifecycle, model) {
        val observer = LifecycleEventObserver { _, event -> if (event == Lifecycle.Event.ON_STOP) model.interrupt() }
        lifecycle.lifecycle.addObserver(observer)
        onDispose { lifecycle.lifecycle.removeObserver(observer); model.closeEditor() }
    }
    Column(verticalArrangement = Arrangement.spacedBy(10.dp)) {
        Row(Modifier.horizontalScroll(rememberScrollState()).testTag("mode-selector"), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            FilterChip(selected = !assist.enabled, onClick = { model.setAssistMode(false) }, label = { Text("Manual") }, modifier = Modifier.testTag("mode-manual"))
            FilterChip(selected = assist.enabled, onClick = { model.setAssistMode(true) }, label = { Text("AI Assist") }, modifier = Modifier.testTag("mode-assist"))
            FilterChip(selected = false, enabled = false, onClick = {}, label = { Text("AI Auto-Edit") }, modifier = Modifier.testTag("mode-auto").semantics { contentDescription = "AI Auto-Edit unavailable" })
        }
        if (assist.enabled) AssistPanel(model, ui, assist)
        assist.notice?.let { Text(it, Modifier.testTag("assist-notice").semantics { liveRegion = LiveRegionMode.Polite }) }
        Button(onClick = { picker.launch(arrayOf("image/jpeg", "image/png")) }, enabled = !ui.busy) { Text("Open JPEG or PNG") }
        Box(Modifier.fillMaxWidth().height(180.dp).semantics { contentDescription = "Image preview" }) {
            ui.image?.let { Image(it.asImageBitmap(), "Current manual preview", Modifier.fillMaxSize(), contentScale = ContentScale.Fit) }
            if (ui.image == null) Text("Open an image to edit locally")
        }
        if (ui.busy) Text("Loading or saving…", Modifier.semantics { liveRegion = LiveRegionMode.Polite })
        if (ui.previewLoading) Text("Rendering preview; changes are not saved until Apply", Modifier.semantics { liveRegion = LiveRegionMode.Polite })
        Row(Modifier.horizontalScroll(rememberScrollState()), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            for ((key, label) in listOf("geometry" to "Geometry", "tone_color" to "Tone/Color", "detail" to "Detail", "filters_presets" to "Filters/Presets")) {
                FilterChip(selected = category == key, enabled = !ui.busy, onClick = { formatError = null; category = key; if (ui.core?.active == true) model.action(0) }, label = { Text(label) })
            }
        }
        if (category == "filters_presets") Text("No presets available. The shipping catalog is empty.")
        else Row(Modifier.horizontalScroll(rememberScrollState()), horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            for (tool in ui.tools.filter { it.category == category }) {
                FilterChip(selected = ui.core?.selected == tool.index, enabled = !ui.busy && ui.core != null,
                    onClick = { formatError = null; model.select(tool.index) }, label = { Text(tool.name) })
            }
        }
        val core = ui.core
        val tool = ui.tools.firstOrNull { it.index == core?.selected }
        if (tool != null && core != null) {
            Text(tool.name, style = MaterialTheme.typography.titleMedium)
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
                }, enabled = !ui.busy && !core.approvalPending) { Text("Preview crop") }
                Text("Crop values are millionths of the current extent. Invalid rectangles are rejected.")
            } else {
                val p = tool.parameters.single(); val value = core.values.single()
                Text("${tool.name}: ${displayValue(value, p.unit)}", Modifier.semantics { stateDescription = "${tool.name} value $value" })
                if (p.unit == "clockwise_quarter_turns") {
                    Row(Modifier.horizontalScroll(rememberScrollState()), horizontalArrangement = Arrangement.spacedBy(6.dp)) {
                        for (v in p.minimum..p.maximum step p.step) Button(onClick = { model.update(listOf(v)) }, enabled = !ui.busy && !core.approvalPending) { Text("${v * 90}°") }
                    }
                } else {
                    Slider(value = value.toFloat(), onValueChange = { model.update(listOf(integerPosition(it, p.minimum, p.step))) },
                        valueRange = p.minimum.toFloat()..p.maximum.toFloat(), steps = (p.maximum - p.minimum) / p.step - 1,
                        enabled = !ui.busy && !core.approvalPending, modifier = Modifier.semantics { contentDescription = "${tool.name} value" })
                }
            }
            OutlinedButton(onClick = { model.update(tool.parameters.map { it.default }) }, enabled = !ui.busy && !core.approvalPending) { Text("Reset to default") }
        }
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            Button(onClick = { model.action(1) }, enabled = core?.active == true && !ui.busy) { Text(if (core?.approvalPending == true) "Retry Apply" else "Apply") }
            OutlinedButton(onClick = { model.action(0); formatError = null }, enabled = core?.active == true && !ui.busy) { Text("Cancel") }
        }
        Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
            OutlinedButton(onClick = { model.action(2) }, enabled = core?.canUndo == true && !ui.busy) { Text("Undo") }
            OutlinedButton(onClick = { model.action(3) }, enabled = core?.canRedo == true && !ui.busy) { Text("Redo") }
        }
        core?.let { Text("Committed revisions: ${it.revisionCount}") }
        (formatError ?: ui.error)?.let { Text(it, Modifier.semantics { liveRegion = LiveRegionMode.Assertive }) }
        if (ui.error?.startsWith("Preview failure") == true) Button(onClick = model::retryPreview) { Text("Retry preview") }
        if (ui.error?.contains("Stale edit") == true) Button(onClick = model::reopenProject) { Text("Reopen committed project") }
    }
}
