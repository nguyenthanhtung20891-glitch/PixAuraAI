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
    private fun waitFor(predicate: () -> Boolean) = compose.waitUntil(20_000, predicate)
    private fun openFixture(): File {
        compose.onNodeWithText("Open Manual editor").performClick()
        val file = fixture()
        compose.runOnUiThread { compose.activity.manualModel.importFile(file) }
        waitFor { compose.activity.manualModel.state.value.image != null && !compose.activity.manualModel.state.value.busy }
        return file
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
    @Test fun composeEditingCommitCancelSwitchHistoryLifecycleAndEmptyPresets() {
        val file = openFixture()
        val model = compose.activity.manualModel
        val original = requireNotNull(model.state.value.image)
        compose.onNodeWithText("Tone/Color").performClick()
        compose.onNodeWithText("Exposure").performScrollTo().performClick()
        waitFor { model.state.value.core?.active == true && !model.state.value.busy }
        compose.onNodeWithContentDescription("Exposure value").performSemanticsAction(SemanticsActions.SetProgress) { it(1000f) }
        waitFor { model.state.value.core?.values == listOf(1000) && !model.state.value.previewLoading }
        assertFalse(requireNotNull(model.state.value.image).sameAs(original))
        compose.onNodeWithText("Undo").assertIsNotEnabled()
        compose.onNodeWithText("Apply").performScrollTo().performClick()
        waitFor { model.state.value.core?.revisionCount == 2 && !model.state.value.previewLoading }
        val committed = requireNotNull(model.state.value.image)
        compose.onNodeWithText("Undo").performScrollTo().performClick()
        waitFor { model.state.value.core?.canRedo == true && !model.state.value.previewLoading }
        assertTrue(requireNotNull(model.state.value.image).sameAs(original))
        compose.onNodeWithText("Redo").performScrollTo().performClick()
        waitFor { model.state.value.core?.canRedo == false && !model.state.value.previewLoading }
        assertTrue(requireNotNull(model.state.value.image).sameAs(committed))
        compose.onNodeWithText("Exposure").performScrollTo().performClick()
        waitFor { model.state.value.core?.active == true && !model.state.value.busy }
        compose.onNodeWithContentDescription("Exposure value").performSemanticsAction(SemanticsActions.SetProgress) { it(-1000f) }
        waitFor { model.state.value.core?.values == listOf(-1000) && !model.state.value.previewLoading }
        compose.onNodeWithText("Cancel").performScrollTo().performClick()
        waitFor { model.state.value.core?.active == false && !model.state.value.previewLoading }
        assertEquals(2, model.state.value.core?.revisionCount); assertTrue(requireNotNull(model.state.value.image).sameAs(committed))
        compose.onNodeWithText("Geometry").performScrollTo().performClick()
        compose.onNodeWithText("Rotate").performScrollTo().performClick()
        waitFor { model.state.value.core?.active == true && !model.state.value.busy }
        compose.onNodeWithText("90°").performScrollTo().performClick()
        waitFor { model.state.value.core?.values == listOf(1) && !model.state.value.previewLoading }
        compose.activityRule.scenario.moveToState(Lifecycle.State.CREATED)
        compose.activityRule.scenario.moveToState(Lifecycle.State.RESUMED)
        waitFor { model.state.value.core?.active == false && !model.state.value.busy }
        assertEquals(2, model.state.value.core?.revisionCount)
        compose.onNodeWithText("Filters/Presets").performScrollTo().performClick()
        compose.onNodeWithText("No presets available. The shipping catalog is empty.").performScrollTo().assertIsDisplayed()
        compose.activityRule.scenario.recreate()
        waitFor { compose.activity.manualModel.state.value.core?.revisionCount == 2 && compose.activity.manualModel.state.value.image != null }
        assertTrue(requireNotNull(compose.activity.manualModel.state.value.image).sameAs(committed))
        file.delete()
    }
}
