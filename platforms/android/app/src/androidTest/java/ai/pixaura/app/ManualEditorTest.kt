package ai.pixaura.app

import android.content.Context
import androidx.compose.ui.test.*
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.compose.ui.semantics.SemanticsActions
import androidx.lifecycle.Lifecycle
import androidx.test.platform.app.InstrumentationRegistry
import ai.pixaura.bridge.ManualEditor
import ai.pixaura.bridge.EditorFailure
import java.io.File
import java.util.UUID
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Rule
import org.junit.Test

class ManualEditorTest {
    @get:Rule val compose = createAndroidComposeRule<MainActivity>()
    private val app: Context get() = InstrumentationRegistry.getInstrumentation().targetContext
    private fun fixture(): File {
        val tests = InstrumentationRegistry.getInstrumentation().context
        val json = tests.assets.open("decode/fixtures.json").bufferedReader().use { JSONObject(it.readText()) }
        val values = json.getJSONObject("png").getJSONArray("bytes")
        return File(app.cacheDir, "editor-test-${UUID.randomUUID()}.png").apply { writeBytes(ByteArray(values.length()) { values.getInt(it).toByte() }) }
    }
    /** Waits for the (busy-disabled) category chip, selects it, then waits for its tool chip. */
    private fun selectCategoryAndAwaitTool(category: String, tool: String) {
        compose.waitUntil(20_000) { compose.onAllNodes(hasText(category) and isEnabled()).fetchSemanticsNodes().isNotEmpty() }
        compose.onNode(hasText(category) and isEnabled()).performScrollTo().performClick()
        compose.waitUntil(20_000) { compose.onAllNodesWithText(tool).fetchSemanticsNodes().isNotEmpty() }
    }
    /** The comparison control is disabled while native work is busy; click it only once it is enabled. */
    private fun toggleComparison() {
        compose.waitUntil(20_000) { compose.onAllNodes(hasTestTag("manual-before-after") and isEnabled()).fetchSemanticsNodes().isNotEmpty() }
        compose.onNode(hasTestTag("manual-before-after") and isEnabled()).performScrollTo().performClick()
    }
    /** Native work disables most controls; wait until the control is enabled so a click is never silently dropped. */
    private fun clickEnabled(text: String) {
        compose.waitUntil(20_000) { compose.onAllNodes(hasText(text) and isEnabled()).fetchSemanticsNodes().size == 1 }
        compose.onNode(hasText(text) and isEnabled()).performScrollTo().performClick()
    }
    private fun waitFor(predicate: () -> Boolean) {
        try { compose.waitUntil(20_000, predicate) }
        catch (failure: androidx.compose.ui.test.ComposeTimeoutException) {
            val state = compose.activity.manualModel.state.value
            throw AssertionError("Editor wait failed: error=${state.error}; busy=${state.busy}; preview=${state.previewLoading}; original=${state.showingOriginal}; core=${state.core}; image=${state.image?.width}x${state.image?.height}", failure)
        }
    }
    private fun openFixture(): File {
        compose.onNodeWithText("Open Manual editor").performClick()
        val file = fixture()
        compose.runOnUiThread { compose.activity.manualModel.importFile(file) }
        waitFor { compose.activity.manualModel.state.value.image != null && !compose.activity.manualModel.state.value.busy }
        return file
    }
    @Test fun productionTwelveMegapixelWorkflowKeepsFullExportsAndNativeTrial() {
        val file = File(app.cacheDir, "application-12mp-${UUID.randomUUID()}.png")
        val bitmap = android.graphics.Bitmap.createBitmap(4000, 3000, android.graphics.Bitmap.Config.ARGB_8888)
        try {
            val row = IntArray(4000)
            for (y in 0 until 3000) {
                for (x in row.indices) row[x] = (255 shl 24) or (((x * 13 + y * 7) and 255) shl 16) or (((x * 3 + y * 11) and 255) shl 8) or ((x + y * 5) and 255)
                bitmap.setPixels(row, 0, row.size, 0, y, row.size, 1)
            }
            file.outputStream().use { assertTrue(bitmap.compress(android.graphics.Bitmap.CompressFormat.PNG, 100, it)) }
        } finally { bitmap.recycle() }
        try {
            compose.onNodeWithText("Open Manual editor").performClick()
            val model = compose.activity.manualModel
            compose.runOnUiThread { model.importFile(file) }
            waitFor { model.state.value.image != null && !model.state.value.previewLoading && !model.state.value.busy }
            assertNotNull(model.state.value.commercial)
            compose.onNodeWithTag("commercial-access").assertExists()
            val original = requireNotNull(model.state.value.core).revisionId
            selectCategoryAndAwaitTool("Tone/Color", "Exposure")
            clickEnabled("Exposure")
            waitFor { model.state.value.core?.active == true && !model.state.value.busy }
            compose.onNodeWithContentDescription("Exposure value").performSemanticsAction(SemanticsActions.SetProgress) { it(1000f) }
            waitFor { model.state.value.core?.values == listOf(1000) && !model.state.value.previewLoading }
            clickEnabled("Apply")
            waitFor { model.state.value.core?.revisionCount == 2 && !model.state.value.previewLoading }
            val committed = requireNotNull(model.state.value.core).revisionId
            assertNotEquals(original, committed); assertEquals(1, model.state.value.commercial?.access)
            assertTrue(requireNotNull(model.state.value.commercial).trialStart >= 0)
            toggleComparison()
            waitFor { model.state.value.showingOriginal && !model.state.value.previewLoading }
            compose.onNodeWithTag("manual-before-after").assert(SemanticsMatcher.expectValue(androidx.compose.ui.semantics.SemanticsProperties.StateDescription, "Original source"))
            assertEquals(committed, model.state.value.core?.revisionId); assertEquals(2, model.state.value.core?.revisionCount)
            toggleComparison()
            waitFor { !model.state.value.showingOriginal && !model.state.value.previewLoading }
            compose.onNodeWithTag("manual-before-after").assert(SemanticsMatcher.expectValue(androidx.compose.ui.semantics.SemanticsProperties.StateDescription, "Edited preview"))
            assertEquals(committed, model.state.value.core?.revisionId); assertEquals(2, model.state.value.core?.revisionCount)

            for (format in listOf(1, 2)) {
                val directory = File(app.cacheDir, "export/app-12mp-${UUID.randomUUID()}").apply { assertTrue(mkdirs()) }
                val output = File(directory, if (format == 1) "synthetic.jpg" else "synthetic.png").apply { assertTrue(createNewFile()) }
                try {
                    val uri = androidx.core.content.FileProvider.getUriForFile(app, app.packageName + ".export", output)
                    compose.runOnUiThread { assertTrue(model.reserveExport()); model.saveToDocument(uri, format, 93) }
                    waitFor { !model.state.value.exporting && model.state.value.exportStatus in listOf("Saved", "Export failed") }
                    assertEquals("Saved", model.state.value.exportStatus); assertNull(model.state.value.error)
                    val bounds = android.graphics.BitmapFactory.Options().apply { inJustDecodeBounds = true }
                    android.graphics.BitmapFactory.decodeFile(output.path, bounds)
                    assertEquals(4000, bounds.outWidth); assertEquals(3000, bounds.outHeight); assertTrue(output.length() > 0)
                    val digest = java.security.MessageDigest.getInstance("SHA-256")
                    output.inputStream().use { stream -> val buffer = ByteArray(65536); while (true) { val count = stream.read(buffer); if (count < 0) break; digest.update(buffer, 0, count) } }
                    println("PIXAURA_APPLICATION_12MP=" + JSONObject().put("environment", "Android ARM64 emulator; not physical device qualification")
                        .put("input_width", 4000).put("input_height", 3000).put("format", format)
                        .put("output_width", bounds.outWidth).put("output_height", bounds.outHeight).put("encoded_bytes", output.length())
                        .put("sha256", digest.digest().joinToString("") { "%02x".format(it.toInt() and 255) })
                        .put("native_cap_bytes", model.capabilityStrategy.value?.cap).put("forced_cpu", model.capabilityStrategy.value?.forcedCpu)
                        .put("revision_count", model.state.value.core?.revisionCount).put("trial_access", model.state.value.commercial?.access))
                    assertEquals(committed, model.state.value.core?.revisionId); assertEquals(2, model.state.value.core?.revisionCount)
                } finally { output.delete(); directory.delete() }
            }
            clickEnabled("Undo")
            waitFor { model.state.value.core?.revisionId == original && !model.state.value.previewLoading }
            clickEnabled("Redo")
            waitFor { model.state.value.core?.revisionId == committed && !model.state.value.previewLoading }
            compose.activityRule.scenario.recreate()
            waitFor { compose.activity.manualModel.state.value.core?.revisionId == committed && compose.activity.manualModel.state.value.image != null && !compose.activity.manualModel.state.value.previewLoading }
            assertEquals(2, compose.activity.manualModel.state.value.core?.revisionCount)
        } finally { file.delete() }
    }
    @Test fun sharedOwnerEveryToolChurnHistoryReopenAndFences() {
        val file = fixture(); val root = File(app.noBackupFilesDir, "editor-boundary-${UUID.randomUUID()}").apply { mkdir() }
        val editor = file.inputStream().use { ManualEditor(root.canonicalPath, it, file.length()) }
        try {
            assertEquals(11, editor.tools.size)
            assertEquals(listOf("pixaura.blur", "pixaura.brightness", "pixaura.contrast", "pixaura.crop", "pixaura.exposure", "pixaura.highlights", "pixaura.rotate", "pixaura.saturation", "pixaura.shadows", "pixaura.sharpen", "pixaura.temperature"), editor.tools.map { it.id })
            val base = requireNotNull(editor.render(editor.request())); assertTrue(editor.install(base))
            for (tool in editor.tools) {
                editor.select(tool.index); editor.update(tool.parameters.map { it.default }); editor.action(1)
                assertEquals(1, editor.state().revisionCount)
            }
            val exposure = editor.tools.single { it.name == "Exposure" }
            editor.select(exposure.index)
            repeat(10_001) { editor.update(listOf(if (it == 10_000) 1000 else it % 5001)) }
            assertEquals(listOf(1000), editor.state().values); assertEquals(1, editor.state().revisionCount)
            val candidate = requireNotNull(editor.render(editor.request())); assertTrue(editor.install(candidate)); assertFalse(candidate.bitmap.sameAs(base.bitmap))
            editor.action(1); assertEquals(2, editor.state().revisionCount); assertFalse(editor.install(candidate))
            val committed = requireNotNull(editor.render(editor.request())); assertTrue(editor.install(committed))
            editor.action(2); val undo = requireNotNull(editor.render(editor.request())); assertTrue(undo.bitmap.sameAs(base.bitmap)); assertTrue(editor.install(undo))
            editor.action(3); val redo = requireNotNull(editor.render(editor.request())); assertTrue(redo.bitmap.sameAs(committed.bitmap)); assertTrue(editor.install(redo))
            editor.select(exposure.index); editor.update(listOf(-1000)); val cancelled = requireNotNull(editor.render(editor.request())); assertTrue(editor.install(cancelled))
            editor.action(0); assertFalse(editor.install(cancelled)); assertTrue(requireNotNull(editor.displayedBitmap()).sameAs(committed.bitmap)); assertEquals(2, editor.state().revisionCount)
            editor.select(exposure.index); editor.update(listOf(10)); val switched = requireNotNull(editor.render(editor.request())); editor.select(editor.tools.single { it.name == "Blur" }.index); assertFalse(editor.install(switched))
            editor.update(listOf(500)); val interrupted = requireNotNull(editor.render(editor.request())); editor.action(4); assertFalse(editor.install(interrupted))
            editor.select(editor.tools.single { it.name == "Crop" }.index)
            try { editor.update(listOf(1000000, 500000, 600000, 0)); fail("invalid crop accepted") } catch (e: EditorFailure) { assertEquals(7, e.status) }
            editor.update(listOf(1000000, 500000, 0, 0)); editor.action(0)
            editor.close(); assertTrue(committed.bitmap.width > 0)
            ManualEditor(root.canonicalPath).use { reopened ->
                assertEquals(2, reopened.state().revisionCount); assertTrue(reopened.state().canUndo)
                val image = requireNotNull(reopened.render(reopened.request())); assertTrue(image.bitmap.sameAs(committed.bitmap))
            }
        } finally { editor.close(); file.delete(); root.walkBottomUp().forEach { it.setWritable(true); it.delete() } }
    }
    @Test fun certifyEveryDescriptorValidationCancelCommitUndoRedoAndReopen() {
        val file = fixture()
        try {
            // Independent projects certify every tool against an unchanged source.
            repeat(11) { index ->
                val root = File(app.noBackupFilesDir, "certification-${UUID.randomUUID()}").apply { mkdir() }
                val editor = file.inputStream().use { ManualEditor(root.canonicalPath, it, file.length()) }
                try {
                    val tool = editor.tools[index]
                    val defaults = tool.parameters.map { it.default }
                    fun image() = requireNotNull(editor.render(editor.request())).also { assertTrue(editor.install(it)) }.bitmap
                    val base = image()
                    editor.select(index); editor.update(defaults); image(); editor.action(1)
                    assertEquals(1, editor.state().revisionCount)
                    editor.select(index)
                    for ((parameterIndex, parameter) in tool.parameters.withIndex()) {
                        for (invalid in listOf(parameter.minimum - 1, parameter.maximum + 1)) {
                            val values = defaults.toMutableList().apply { this[parameterIndex] = invalid }
                            try { editor.update(values); fail("${tool.id}/${parameter.name} accepted $invalid") }
                            catch (e: EditorFailure) { assertEquals(7, e.status) }
                            assertEquals(defaults, editor.state().values); assertEquals(1, editor.state().revisionCount)
                        }
                    }
                    val changed = tool.parameters.map { if (it.name == "width_ppm") 500000 else if (tool.parameters.size > 1) it.default else it.maximum }
                    editor.update(changed); image(); editor.action(0)
                    assertEquals(1, editor.state().revisionCount); assertTrue(image().sameAs(base))
                    editor.select(index); editor.update(changed); image(); editor.action(1)
                    assertEquals(2, editor.state().revisionCount)
                    val revision = editor.state().revisionId; val committed = image()
                    try { editor.action(1); fail("double approval accepted") } catch (e: EditorFailure) { assertEquals(13, e.status) }
                    editor.action(2); assertTrue(image().sameAs(base))
                    editor.action(3); assertTrue(image().sameAs(committed)); editor.close()
                    ManualEditor(root.canonicalPath).use { reopened ->
                        assertEquals(revision, reopened.state().revisionId); assertEquals(2, reopened.state().revisionCount)
                        assertTrue(requireNotNull(reopened.render(reopened.request())).bitmap.sameAs(committed))
                        reopened.action(2); assertTrue(requireNotNull(reopened.render(reopened.request())).bitmap.sameAs(base))
                        reopened.action(3); assertTrue(requireNotNull(reopened.render(reopened.request())).bitmap.sameAs(committed))
                    }
                } finally { editor.close(); root.walkBottomUp().forEach { it.setWritable(true); it.delete() } }
            }
        } finally { file.delete() }
    }
    @Test fun composeEditingCommitCancelSwitchHistoryLifecycleAndEmptyPresets() {
        val file = openFixture()
        val model = compose.activity.manualModel
        val original = requireNotNull(model.state.value.image)
        selectCategoryAndAwaitTool("Tone/Color", "Exposure")
        clickEnabled("Exposure")
        waitFor { model.state.value.core?.active == true && !model.state.value.busy }
        compose.onNodeWithContentDescription("Exposure value").performSemanticsAction(SemanticsActions.SetProgress) { it(1000f) }
        waitFor { model.state.value.core?.values == listOf(1000) && !model.state.value.previewLoading }
        assertFalse(requireNotNull(model.state.value.image).sameAs(original))
        compose.onNodeWithText("Undo").assertIsNotEnabled()
        clickEnabled("Apply")
        waitFor { model.state.value.core?.revisionCount == 2 && !model.state.value.previewLoading }
        val committed = requireNotNull(model.state.value.image)
        clickEnabled("Undo")
        waitFor { model.state.value.core?.canRedo == true && !model.state.value.previewLoading }
        assertTrue(requireNotNull(model.state.value.image).sameAs(original))
        clickEnabled("Redo")
        waitFor { model.state.value.core?.canRedo == false && !model.state.value.previewLoading }
        assertTrue(requireNotNull(model.state.value.image).sameAs(committed))
        clickEnabled("Exposure")
        waitFor { model.state.value.core?.active == true && !model.state.value.busy }
        compose.onNodeWithContentDescription("Exposure value").performSemanticsAction(SemanticsActions.SetProgress) { it(-1000f) }
        waitFor { model.state.value.core?.values == listOf(-1000) && !model.state.value.previewLoading }
        clickEnabled("Cancel")
        waitFor { model.state.value.core?.active == false && !model.state.value.previewLoading }
        assertEquals(2, model.state.value.core?.revisionCount); assertTrue(requireNotNull(model.state.value.image).sameAs(committed))
        clickEnabled("Geometry")
        clickEnabled("Rotate")
        waitFor { model.state.value.core?.active == true && !model.state.value.busy }
        clickEnabled("90°")
        waitFor { model.state.value.core?.values == listOf(1) && !model.state.value.previewLoading }
        compose.activityRule.scenario.moveToState(Lifecycle.State.CREATED)
        compose.activityRule.scenario.moveToState(Lifecycle.State.RESUMED)
        waitFor { model.state.value.core?.active == false && !model.state.value.busy }
        assertEquals(2, model.state.value.core?.revisionCount)
        clickEnabled("Filters/Presets")
        compose.onNodeWithText("No presets available. The shipping catalog is empty.").performScrollTo().assertIsDisplayed()
        repeat(3) {
            val previousDisplay = compose.activity.manualModel.state.value.image
            compose.activityRule.scenario.recreate()
            waitFor {
                val restored = compose.activity.manualModel.state.value
                restored.core?.revisionCount == 2 && restored.image != null && restored.image !== previousDisplay &&
                    !restored.busy && !restored.previewLoading
            }
            assertTrue(requireNotNull(compose.activity.manualModel.state.value.image).sameAs(committed))
        }
        file.delete()
    }
}
