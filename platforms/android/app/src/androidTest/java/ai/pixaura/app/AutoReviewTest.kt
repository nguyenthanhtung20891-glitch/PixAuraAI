package ai.pixaura.app

import ai.pixaura.bridge.*
import androidx.compose.ui.test.*
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.security.MessageDigest
import java.util.UUID
import java.util.concurrent.*
import java.util.concurrent.atomic.AtomicBoolean

import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Rule
import org.junit.Test

class AutoReviewTest {
    @get:Rule val compose = createAndroidComposeRule<MainActivity>()
    private val model get() = compose.activity.manualModel
    private val hooks = AgentPlatformHooks()
    private fun waitFor(test: () -> Boolean) = compose.waitUntil(20_000, test)
    private fun owner(): ManualEditor = ManualEditorModel::class.java.getDeclaredField("owner").apply { isAccessible = true }.get(model) as ManualEditor
    private fun mode(tag: String) { compose.onNodeWithTag("mode-selector").performScrollTo(); compose.onNodeWithTag(tag).performScrollTo().performClick() }
    private fun barrier() {
        val field = ManualEditorModel::class.java.getDeclaredField("commands").apply { isAccessible = true }
        (field.get(model) as ExecutorService).submit {}.get(10, TimeUnit.SECONDS)
    }
    private fun open() {
        compose.onNodeWithText("Open Manual editor").performClick()
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val values = instrumentation.context.assets.open("decode/fixtures.json").bufferedReader().use { JSONObject(it.readText()).getJSONObject("png").getJSONArray("bytes") }
        val file = File(instrumentation.targetContext.cacheDir, "review-${UUID.randomUUID()}.png")
        file.writeBytes(ByteArray(values.length()) { values.getInt(it).toByte() })
        compose.runOnUiThread { model.importFile(file) }; waitFor { model.state.value.image != null && !model.state.value.busy }; file.delete()
        mode("mode-auto"); waitFor { model.autoProposal.value.choices.size == 11 }
        compose.onNodeWithTag("auto-goal").performScrollTo().performClick()
        compose.onNodeWithTag("auto-goal-pixaura.exposure").performScrollTo().performClick()
    }
    private fun request(): ManualEditor.AutoReview {
        compose.onNodeWithTag("auto-request").performScrollTo().performClick()
        waitFor { model.autoProposal.value.status == AutoStatus.REVIEW_READY && !model.autoProposal.value.inFlight }
        return requireNotNull(model.autoProposal.value.review)
    }
    private fun status(code: Int, body: () -> Unit) { try { body(); fail("expected $code") } catch (error: EditorFailure) { assertEquals(code, error.status) } }
    @Test fun explicitHumanApplyBindsExactReviewOnceAndSharesManualUndoRedoHistory() {
        open(); val editor = owner(); val source = editor.state().revisionId
        request(); compose.onNodeWithTag("auto-dismiss").performScrollTo().performClick(); assertEquals(1, editor.state().revisionCount)
        val review = request(); val reviewed = review.canonicalBytes()
        compose.onNodeWithTag("auto-exact-values-0").assertTextContains("milli_ev = 500", substring = true)
        status(15) { editor.executeAgentPlan(editor.validateAgentPlan(reviewed)) }
        val copy = review.canonicalBytes(); copy[0] = 120; assertArrayEquals(reviewed, review.canonicalBytes())
        compose.onNodeWithTag("auto-apply").performScrollTo().performClick()
        compose.runOnUiThread { model.applyAutoFromHuman(review) } // duplicate explicit event, never another approval
        waitFor { model.autoProposal.value.status == AutoStatus.COMPLETED && !model.autoProposal.value.inFlight && model.state.value.core?.revisionCount == 2 }
        compose.onNodeWithTag("auto-completed").assertExists()
        val receipt = requireNotNull(model.autoProposal.value.receipt)
        assertEquals(source, receipt.sourceRevision)
        assertEquals(MessageDigest.getInstance("SHA-256").digest(reviewed).joinToString("") { "%02x".format(it.toInt() and 255) }, receipt.planSHA256)
        assertEquals(receipt.resultingRevision, editor.state().revisionId)
        status(15) { editor.applyAutoFromHuman(review) }
        compose.runOnUiThread { model.cancelAuto(); model.dismissAuto() }; assertEquals(AutoStatus.COMPLETED, model.autoProposal.value.status)
        mode("mode-manual")
        compose.onNodeWithText("Undo").performScrollTo().performClick(); waitFor { model.state.value.core?.revisionId == source && !model.state.value.busy }
        compose.onNodeWithText("Redo").performScrollTo().performClick(); waitFor { model.state.value.core?.revisionId == receipt.resultingRevision && !model.state.value.busy }
        compose.runOnUiThread { model.select(4) }; waitFor { model.state.value.core?.active == true && !model.state.value.busy }
        compose.runOnUiThread { model.update(listOf(250)) }; waitFor { model.state.value.core?.values == listOf(250) }
        compose.onNodeWithText("Apply").performScrollTo().performClick(); waitFor { model.state.value.core?.revisionCount == 3 && !model.state.value.busy }
    }
    @Test fun userCancelAndModeDepartureFenceExecutionAndRequireFreshProposal() {
        open(); val editor = owner(); val source = editor.state().revisionId
        for (leaveMode in listOf(false, true)) {
            val review = request(); assertEquals(0, hooks.control(editor, 0, 1))
            try {
                compose.onNodeWithTag("auto-apply").performScrollTo().performClick(); waitFor { hooks.control(editor, 1) == 1 }
                status(15) { editor.applyAutoFromHuman(review) }; status(8) { editor.requestAdvice(0) }
                if (leaveMode) mode("mode-manual") else compose.onNodeWithTag("auto-execution-cancel").performScrollTo().performClick()
                barrier(); hooks.control(editor, 2)
                waitFor { !model.autoProposal.value.inFlight }; barrier()
                assertEquals(source, editor.state().revisionId); assertEquals(1, editor.state().revisionCount)
                status(15) { editor.applyAutoFromHuman(review) }
                if (leaveMode) mode("mode-auto") else { assertEquals(AutoStatus.CANCELLED, model.autoProposal.value.status); compose.onNodeWithTag("auto-cancelled").assertExists() }
            } finally { hooks.control(editor, 2) }
        }
    }
    @Test fun wrapperRejectsWrongProposalStaleSourceAndCancelBeforeNativeAdmission() {
        open(); val editor = owner(); val old = editor.prepareAutoReview(editor.requestAuto(0))
        val proposal = editor.requestAuto(2); status(9) { editor.applyAutoFromHuman(old) }
        val review = editor.prepareAutoReview(proposal)
        status(15) { editor.applyAutoFromHuman(review) { editor.cancelAutoReview(review) } }
        assertEquals(1, editor.state().revisionCount); status(15) { editor.prepareAutoReview(proposal) }
        val stale = editor.prepareAutoReview(editor.requestAuto(0)); editor.select(4); editor.update(listOf(100)); editor.action(1)
        status(9) { editor.applyAutoFromHuman(stale) }
        val fresh = editor.prepareAutoReview(editor.requestAuto(0)); editor.applyAutoFromHuman(fresh)
        assertEquals(3, editor.state().revisionCount)
    }
    @Test fun recreationStopsAndJoinsApplyingOwnerWithoutPublicationOrApprovalRecovery() {
        open(); val editor = owner(); val source = editor.state().revisionId
        val review = request(); assertEquals(0, hooks.control(editor, 0, 1))
        compose.onNodeWithTag("auto-apply").performScrollTo().performClick(); waitFor { hooks.control(editor, 1) == 1 }
        val stopped = AtomicBoolean(false)
        val resume = Thread {
            val limit = System.nanoTime() + TimeUnit.SECONDS.toNanos(10)
            while (System.nanoTime() < limit) {
                try { editor.state() } catch (_: IllegalStateException) { stopped.set(true); break }
                Thread.sleep(1)
            }
            hooks.control(editor, 2)
        }
        try {
            resume.start(); compose.activityRule.scenario.recreate(); resume.join(); assertTrue(stopped.get())
            waitFor { model.state.value.image != null && !model.state.value.busy && !model.autoProposal.value.inFlight }
            assertFalse(model.autoProposal.value.enabled); assertNull(model.autoProposal.value.review)
            assertEquals(source, owner().state().revisionId); assertEquals(1, owner().state().revisionCount)
            status(9) { owner().applyAutoFromHuman(review) }; status(13) { editor.applyAutoFromHuman(review) }
        } finally { hooks.control(editor, 2); if (resume.isAlive) resume.join(10_000) }
    }
}
