package ai.pixaura.app

import ai.pixaura.bridge.*
import androidx.compose.ui.test.*
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.nio.ByteBuffer
import java.util.UUID
import java.util.concurrent.atomic.AtomicBoolean
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Rule
import org.junit.Test

class AutoTestHooks {
    private external fun nativeControl(context: ByteBuffer, command: Int, value: Int): Int
    fun control(editor: ManualEditor, command: Int, value: Int = 0): Int {
        val field = ManualEditor::class.java.getDeclaredField("storage").apply { isAccessible = true }
        return nativeControl(field.get(editor) as ByteBuffer, command, value)
    }
}
class AutoProposalTest {
    @get:Rule val compose = createAndroidComposeRule<MainActivity>()
    private val model get() = compose.activity.manualModel
    private val hooks = AutoTestHooks()
    private fun waitFor(predicate: () -> Boolean) = compose.waitUntil(20_000, predicate)
    private fun owner(): ManualEditor = ManualEditorModel::class.java.getDeclaredField("owner").apply { isAccessible = true }.get(model) as ManualEditor
    private fun mode(tag: String) {
        compose.onNodeWithTag("mode-selector").performScrollTo()
        compose.onNodeWithTag(tag).performScrollTo().performClick()
    }
    private fun open() {
        compose.onNodeWithText("Open Manual editor").performClick()
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val values = instrumentation.context.assets.open("decode/fixtures.json").bufferedReader().use { JSONObject(it.readText()).getJSONObject("png").getJSONArray("bytes") }
        val file = File(instrumentation.targetContext.cacheDir, "auto-${UUID.randomUUID()}.png")
        file.writeBytes(ByteArray(values.length()) { values.getInt(it).toByte() })
        compose.runOnUiThread { model.importFile(file) }
        waitFor { model.state.value.image != null && !model.state.value.busy }; file.delete()
        mode("mode-auto"); waitFor { model.autoProposal.value.choices.size == 11 }
        compose.onNodeWithTag("auto-goal").performScrollTo().performClick()
        compose.onNodeWithTag("auto-goal-pixaura.exposure").performScrollTo().performClick()
    }
    private fun request() { compose.onNodeWithTag("auto-request").performScrollTo().performClick() }
    private fun ready() = waitFor { model.autoProposal.value.status == AutoStatus.REVIEW_READY }
    @Test fun admittedReferenceProviderAllGoalsRepeatWithoutAuthority() {
        open(); val editor = owner(); val revision = editor.state().revisionId
        val choices = editor.adviceChoices(); assertEquals(11, choices.size)
        for (choice in choices) {
            val first = editor.requestAuto(choice.index)
            assertEquals("reference.assistance.v1", first.providerId)
            assertEquals(1, first.actions.size)
            assertEquals(choice.values, first.actions[0].values)
            val bytes = editor.inspectAuto(first)
            assertArrayEquals(bytes, editor.validateAgentPlan(bytes).canonicalBytes())
            editor.cancelAuto()
            val repeated = editor.requestAuto(choice.index)
            assertArrayEquals(bytes, editor.inspectAuto(repeated))
            try { editor.executeAgentPlan(editor.validateAgentPlan(bytes)); fail("provider minted approval") }
            catch (e: EditorFailure) { assertEquals(15, e.status) }
            editor.cancelAuto()
            assertEquals(revision, editor.state().revisionId)
            assertEquals(1, editor.state().revisionCount)
        }
    }
    @Test fun realProposalSummaryIsInertDismissibleAndNeverAnApprovalControl() {
        open(); val editor = owner(); val revision = editor.state().revisionId
        request(); ready(); compose.onNodeWithTag("auto-ready").assertExists()
        assertEquals(revision, editor.state().revisionId); assertEquals(1, editor.state().revisionCount)
        val proposal = requireNotNull(model.autoProposal.value.proposal)
        assertEquals(1, proposal.actions.size); assertEquals(listOf(500), proposal.actions[0].values)
        val bytes = editor.inspectAuto(proposal); assertArrayEquals(bytes, editor.validateAgentPlan(bytes).canonicalBytes())
        try { editor.executeAgentPlan(editor.validateAgentPlan(bytes)); fail("proposal gained approval") } catch (e: EditorFailure) { assertEquals(15, e.status) }
        for (label in listOf("Apply", "Approve", "Execute", "Run Plan", "Commit")) compose.onNodeWithText(label).assertDoesNotExist()
        compose.onNodeWithTag("auto-dismiss").performScrollTo().performClick()
        compose.onNodeWithTag("auto-rejected").assertExists(); assertEquals(revision, editor.state().revisionId)
        request(); ready(); mode("mode-manual"); mode("mode-auto")
        compose.onNodeWithTag("auto-ready").assertDoesNotExist()
        assertEquals(AutoStatus.IDLE, model.autoProposal.value.status)
        request(); ready(); compose.runOnUiThread { model.select(4) }
        waitFor { !model.state.value.busy && model.autoProposal.value.status == AutoStatus.STALE }
        compose.onNodeWithTag("auto-ready").assertDoesNotExist()
        mode("mode-manual"); compose.runOnUiThread { model.update(listOf(250)) }
        waitFor { model.state.value.core?.values == listOf(250) }; compose.onNodeWithText("Apply").performScrollTo().performClick()
        waitFor { !model.state.value.busy && model.state.value.core?.revisionCount == 2 }
        compose.onNodeWithText("Undo").performScrollTo().performClick(); waitFor { model.state.value.core?.revisionId == revision && !model.state.value.busy }
        compose.onNodeWithText("Redo").performScrollTo().performClick(); waitFor { model.state.value.core?.revisionId != revision && !model.state.value.busy }
        mode("mode-assist"); waitFor { model.assistance.value.enabled }
        compose.onNodeWithTag("assist-panel").assertExists()
    }
    @Test fun cancellationFailuresBusyModeSwitchAndTeardownNeverPublishOrExecute() {
        open(); val editor = owner(); val revision = editor.state().revisionId
        assertEquals(0, hooks.control(editor, 0, 1))
        try {
            request(); waitFor { hooks.control(editor, 1) == 1 }
            compose.onNodeWithTag("auto-loading").assertExists()
            compose.runOnUiThread { model.requestAuto() }
            try { editor.requestAdvice(0); fail("shared provider not BUSY") } catch (e: EditorFailure) { assertEquals(8, e.status) }
            compose.onNodeWithTag("auto-cancel").performScrollTo().performClick()
            mode("mode-manual"); hooks.control(editor, 2); waitFor { !model.autoProposal.value.inFlight }
            assertNull(model.autoProposal.value.proposal); assertEquals(revision, editor.state().revisionId)
        } finally { hooks.control(editor, 2) }
        hooks.control(editor, 0, -1)
        for (fault in listOf(2, 3, 4, 6, 7, 8, 9, 10, 12)) {
            hooks.control(editor, 3, fault); mode("mode-auto"); request()
            waitFor { !model.autoProposal.value.inFlight && model.autoProposal.value.status == AutoStatus.FAILED }
            compose.onNodeWithTag("auto-error").assertExists(); assertEquals(revision, editor.state().revisionId)
        }
        hooks.control(editor, 3, 0); hooks.control(editor, 0, 2); request(); waitFor { hooks.control(editor, 1) == 1 }
        val stopped = AtomicBoolean(false)
        val resume = Thread {
            val end = System.nanoTime() + java.util.concurrent.TimeUnit.SECONDS.toNanos(10)
            while (System.nanoTime() < end) {
                try { editor.state() } catch (_: IllegalStateException) { stopped.set(true); break }
                Thread.sleep(1)
            }
            hooks.control(editor, 2)
        }
        resume.start(); compose.activityRule.scenario.recreate(); resume.join(); assertTrue(stopped.get())
        waitFor { model.state.value.image != null && !model.state.value.busy && !model.autoProposal.value.inFlight }
        assertFalse(model.autoProposal.value.enabled); assertNull(model.autoProposal.value.proposal)
        assertEquals(revision, owner().state().revisionId)
    }
}
