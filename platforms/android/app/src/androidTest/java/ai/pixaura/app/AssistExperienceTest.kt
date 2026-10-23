package ai.pixaura.app

import ai.pixaura.bridge.*
import androidx.compose.ui.test.*
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.nio.ByteBuffer
import java.util.UUID
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Rule
import org.junit.Test

// Debug instrumentation only. Tests resume and join before owner destruction.
class AssistTestHooks {
    private external fun nativeControl(context: ByteBuffer, command: Int, value: Int): Int
    fun control(editor: ManualEditor, command: Int, value: Int = 0): Int {
        val field = ManualEditor::class.java.getDeclaredField("storage").apply { isAccessible = true }
        return nativeControl(field.get(editor) as ByteBuffer, command, value)
    }
}

class AssistExperienceTest {
    @get:Rule val compose = createAndroidComposeRule<MainActivity>()
    private val model get() = compose.activity.manualModel
    private val hooks = AssistTestHooks()
    private fun waitFor(predicate: () -> Boolean) = compose.waitUntil(20_000, predicate)
    private fun owner(): ManualEditor {
        val field = ManualEditorModel::class.java.getDeclaredField("owner").apply { isAccessible = true }
        return field.get(model) as ManualEditor
    }
    private fun open() {
        compose.onNodeWithText("Open Manual editor").performClick()
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val values = instrumentation.context.assets.open("decode/fixtures.json").bufferedReader().use {
            JSONObject(it.readText()).getJSONObject("png").getJSONArray("bytes")
        }
        val file = File(instrumentation.targetContext.cacheDir, "assist-${UUID.randomUUID()}.png")
        file.writeBytes(ByteArray(values.length()) { values.getInt(it).toByte() })
        compose.runOnUiThread { model.importFile(file) }
        waitFor { model.state.value.image != null && !model.state.value.busy }
        file.delete()
    }
    private fun goal() {
        compose.onNodeWithTag("mode-assist").performClick()
        waitFor { model.assistance.value.choices.size == 11 }
        compose.onNodeWithTag("assist-goal").performScrollTo().performClick()
        compose.onNodeWithTag("assist-goal-pixaura.exposure").performScrollTo().performClick()
    }
    private fun request() { compose.onNodeWithTag("assist-request").performScrollTo().performClick() }
    private fun ready() { waitFor { model.assistance.value.status == AssistStatus.PROPOSAL_AVAILABLE } }
    @Test fun adviceDismissNavigationAndOnlyExplicitManualApplyChangesHistory() {
        open(); val revision = model.state.value.core!!.revisionId
        compose.onNodeWithTag("mode-auto").assertIsNotEnabled()
        goal(); request(); ready()
        compose.onNodeWithTag("assist-recommendation").assertExists()
        assertEquals(revision, owner().state().revisionId)
        compose.onNodeWithTag("assist-dismiss").performScrollTo().performClick()
        compose.onNodeWithTag("assist-dismissed").assertExists()
        request(); ready()
        compose.onNodeWithTag("assist-use").performScrollTo().performClick()
        waitFor { !model.state.value.busy && !model.assistance.value.enabled }
        assertEquals(revision, owner().state().revisionId)
        assertEquals(listOf(0), owner().state().values) // recommendation was NOT applied/prefilled
        compose.onNodeWithContentDescription("Exposure value").performScrollTo().performSemanticsAction(
            androidx.compose.ui.semantics.SemanticsActions.SetProgress) { it(1000f) }
        waitFor { model.state.value.core?.values == listOf(1000) }
        compose.onNodeWithText("Apply", useUnmergedTree = true).performScrollTo().performClick()
        waitFor { !model.state.value.busy && model.state.value.core?.revisionCount == 2 }
        compose.onNodeWithText("Undo").performScrollTo().performClick()
        waitFor { !model.state.value.busy && model.state.value.core?.revisionId == revision }
        compose.onNodeWithText("Redo").performScrollTo().performClick()
        waitFor { !model.state.value.busy && model.state.value.core?.revisionId != revision }
        goal(); request(); ready()
        compose.runOnUiThread { model.action(2) }
        waitFor { model.assistance.value.status == AssistStatus.STALE && !model.state.value.busy }
        compose.onNodeWithTag("assist-recommendation").assertDoesNotExist()
        compose.activityRule.scenario.recreate()
        waitFor { model.state.value.image != null && !model.state.value.busy }
        assertFalse(model.assistance.value.enabled)
        assertNull(model.assistance.value.advice)
    }
    @Test fun cancelModeSwitchDuplicateRequestAndFailureNeverDisableManual() {
        open(); goal(); val editor = owner(); val original = editor.state().revisionId
        assertEquals(0, hooks.control(editor, 0, 1))
        try {
            request(); waitFor { hooks.control(editor, 1) == 1 }
            compose.onNodeWithTag("assist-loading").assertExists()
            compose.runOnUiThread { model.requestAdvice() } // duplicate is bounded, not queued
            compose.onNodeWithTag("assist-cancel").performScrollTo().performClick()
            assertEquals(AssistStatus.CANCELLED, model.assistance.value.status)
            compose.onNodeWithTag("mode-manual").performScrollTo().performClick()
            assertEquals(0, hooks.control(editor, 2))
            waitFor { !model.assistance.value.inFlight }
            assertNull(model.assistance.value.advice)
            assertEquals(original, editor.state().revisionId)
        } finally { hooks.control(editor, 2) }
        assertEquals(0, hooks.control(editor, 0, -1))
        for (fault in listOf(2, 3, 4, 5, 1)) {
            assertEquals(0, hooks.control(editor, 3, fault))
            goal(); request()
            waitFor { !model.assistance.value.inFlight && model.assistance.value.status in listOf(AssistStatus.FAILED, AssistStatus.CANCELLED) }
            compose.onNodeWithTag("assist-error").assertExists()
            assertEquals(original, editor.state().revisionId)
        }
        compose.onNodeWithTag("mode-manual").performScrollTo().performClick()
        compose.runOnUiThread { model.select(editor.tools.single { it.id == "pixaura.exposure" }.index) }
        waitFor { !model.state.value.busy && model.state.value.core?.active == true }
        compose.runOnUiThread { model.update(listOf(250)) }
        waitFor { model.state.value.core?.values == listOf(250) }
        compose.runOnUiThread { model.action(1) }
        waitFor { !model.state.value.busy && model.state.value.core?.revisionCount == 2 }
    }
    @Test fun manualSourceChangeAndScreenRecreationDiscardInFlightAdvice() {
        open(); goal(); val editor = owner()
        assertEquals(0, hooks.control(editor, 0, 1))
        try {
            request(); waitFor { hooks.control(editor, 1) == 1 }
            compose.runOnUiThread { model.select(editor.tools.single { it.id == "pixaura.brightness" }.index) }
            waitFor { !model.state.value.busy && model.state.value.core?.active == true }
            compose.runOnUiThread { model.update(listOf(100)) }
            waitFor { model.state.value.core?.values == listOf(100) }
            compose.runOnUiThread { model.action(1) }
            waitFor { !model.state.value.busy && model.state.value.core?.revisionCount == 2 }
            assertEquals(0, hooks.control(editor, 2))
            waitFor { !model.assistance.value.inFlight }
            assertNull(model.assistance.value.advice)
            assertEquals(AssistStatus.STALE, model.assistance.value.status)
        } finally { hooks.control(editor, 2) }
        assertEquals(0, hooks.control(editor, 0, 1))
        request(); waitFor { hooks.control(editor, 1) == 1 }
        // Disposal closes on the command lane, joining native advice. Resuming
        // from the test thread lets teardown finish without blocking the UI.
        val stopped = java.util.concurrent.atomic.AtomicBoolean(false)
        val resume = Thread {
            val deadline = System.nanoTime() + java.util.concurrent.TimeUnit.SECONDS.toNanos(10)
            while (System.nanoTime() < deadline) {
                try { editor.state() }
                catch (failure: EditorFailure) { if (failure.status == 13) { stopped.set(true); break } }
                catch (_: IllegalStateException) { stopped.set(true); break } // existing wrapper close contract
                Thread.sleep(1)
            }
            hooks.control(editor, 2)
        }
        resume.start(); compose.activityRule.scenario.recreate(); resume.join()
        assertTrue("old owner stopped before native request resumed", stopped.get())
        waitFor { model.state.value.image != null && !model.state.value.busy && !model.assistance.value.inFlight }
        assertFalse(model.assistance.value.enabled); assertNull(model.assistance.value.advice)
        assertEquals(2, model.state.value.core!!.revisionCount)
    }
}
