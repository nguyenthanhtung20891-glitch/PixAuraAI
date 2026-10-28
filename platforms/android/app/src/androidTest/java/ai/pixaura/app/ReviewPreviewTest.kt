package ai.pixaura.app

import ai.pixaura.bridge.*
import androidx.compose.ui.test.*
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.nio.ByteBuffer
import java.util.UUID
import java.util.concurrent.*
import java.util.concurrent.atomic.AtomicBoolean
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Rule
import org.junit.Test

class ReviewPreviewTestHooks {
    private external fun nativeControl(context: ByteBuffer, command: Int, value: Int): Int
    fun control(editor: ManualEditor, command: Int, value: Int = 0): Int {
        val field = ManualEditor::class.java.getDeclaredField("storage").apply { isAccessible = true }
        return nativeControl(field.get(editor) as ByteBuffer, command, value)
    }
}
class ReviewPreviewTest {
    @get:Rule val compose = createAndroidComposeRule<MainActivity>()
    private val model get() = compose.activity.manualModel
    private val hooks = ReviewPreviewTestHooks()
    private fun waitFor(test: () -> Boolean) = compose.waitUntil(20_000, test)
    private fun owner(): ManualEditor = ManualEditorModel::class.java.getDeclaredField("owner").apply { isAccessible = true }.get(model) as ManualEditor
    private fun tap(tag: String) {
        // Chips live inside a horizontal scroller, itself inside the editor's
        // vertical viewport. Resolve the containing row first, as in the retained
        // mode-transition harness; scrolling only the chip can leave it offscreen.
        if (tag.startsWith("mode-")) compose.onNodeWithTag("mode-selector").performScrollTo()
        compose.onNodeWithTag(tag).performScrollTo().performClick()
        if (tag.startsWith("mode-")) compose.onNodeWithTag(tag).assertIsSelected()
    }
    private fun barrier() {
        val field = ManualEditorModel::class.java.getDeclaredField("commands").apply { isAccessible = true }
        (field.get(model) as ExecutorService).submit {}.get(10, TimeUnit.SECONDS)
    }
    private fun drained(): Boolean = !(ManualEditorModel::class.java.getDeclaredField("reviewRendering").apply { isAccessible = true }.get(model) as AtomicBoolean).get()
    private fun open() {
        compose.onNodeWithText("Open Manual editor").performClick()
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val values = instrumentation.context.assets.open("decode/fixtures.json").bufferedReader().use { JSONObject(it.readText()).getJSONObject("png").getJSONArray("bytes") }
        val file = File(instrumentation.targetContext.cacheDir, "preview-${UUID.randomUUID()}.png")
        file.writeBytes(ByteArray(values.length()) { values.getInt(it).toByte() })
        compose.runOnUiThread { model.importFile(file) }; waitFor { model.state.value.image != null && !model.state.value.busy }; file.delete()
        tap("mode-auto"); waitFor { model.autoProposal.value.choices.size == 11 }
        tap("auto-goal"); tap("auto-goal-pixaura.exposure"); tap("auto-request")
        waitFor { model.autoProposal.value.review != null && !model.autoProposal.value.inFlight }
    }
    @Test fun previewIsDisposableComparisonAndApplyRemainsExplicit() {
        open(); val editor = owner(); val source = editor.state().revisionId
        tap("review-preview-request"); waitFor { model.autoProposal.value.preview.status == ReviewPreviewStatus.PREVIEW_READY }
        assertEquals(source, editor.state().revisionId); assertEquals(1, editor.state().revisionCount)
        compose.onNodeWithTag("review-before").assertExists()
        tap("review-toggle"); compose.onNodeWithTag("review-after").assertExists()
        tap("review-split"); compose.onNodeWithTag("review-divider").assertExists()
        tap("review-reset"); compose.onNodeWithTag("review-before").assertExists()
        val pair = requireNotNull(model.autoProposal.value.preview.comparison)
        assertFalse(pair.geometryChanged); assertEquals(2, pair.before.width); assertEquals(3, pair.before.height)
        assertEquals(0, hooks.control(editor, 5)) // native pair released after bounded platform copy
        tap("auto-apply")
        waitFor { model.autoProposal.value.status == AutoStatus.COMPLETED && !model.autoProposal.value.inFlight }
        assertEquals(2, editor.state().revisionCount)
        tap("mode-manual"); compose.onNodeWithText("Undo").performScrollTo().performClick()
        waitFor { model.state.value.core?.revisionId == source && !model.state.value.busy }
    }
    @Test fun deterministicCancelFailureAndModeDepartureNeverInstallLatePixels() {
        open(); val editor = owner(); val source = editor.state().revisionId
        assertEquals(0, hooks.control(editor, 0, 0))
        try {
            tap("review-preview-request"); waitFor { hooks.control(editor, 1) == 1 }
            compose.onNodeWithTag("auto-apply").assertIsNotEnabled()
            tap("review-preview-cancel"); barrier(); hooks.control(editor, 2)
            waitFor { model.autoProposal.value.preview.status == ReviewPreviewStatus.CANCELLED && drained() }; barrier()
            assertNull(model.autoProposal.value.preview.comparison); assertEquals(source, editor.state().revisionId)
        } finally { hooks.control(editor, 2); hooks.control(editor, 0, -1) }
        // Bounded resource failure is honest and leaves the exact Plan inspectable.
        assertEquals(0, hooks.control(editor, 3, 2))
        tap("review-preview-request"); waitFor { model.autoProposal.value.preview.status == ReviewPreviewStatus.PREVIEW_FAILED && drained() }
        assertEquals(1, editor.state().revisionCount); compose.onNodeWithTag("auto-apply").assertIsEnabled()
        hooks.control(editor, 3, 0); hooks.control(editor, 0, 2)
        try {
            tap("review-preview-request"); waitFor { hooks.control(editor, 1) == 1 }
            tap("mode-manual"); barrier(); hooks.control(editor, 2); barrier()
            assertNull(model.autoProposal.value.preview.comparison); assertFalse(model.autoProposal.value.enabled)
            assertEquals(1, editor.state().revisionCount)
            compose.runOnUiThread { model.select(4) }; waitFor { model.state.value.core?.active == true && !model.state.value.busy }
        } finally { hooks.control(editor, 2) }
    }
    @Test fun wrapperRejectsStaleAndWrongOwnerAndGeometryUsesFitToggle() {
        open(); val editor = owner(); val review = requireNotNull(model.autoProposal.value.review)
        val ticket = editor.beginReviewPreview(review); val pair = editor.renderReviewPreview(ticket)
        assertTrue(editor.installReviewPreview(pair) {})
        editor.select(4); editor.update(listOf(100)); editor.action(1)
        assertFalse(editor.installReviewPreview(pair) {})
        // The pre-copy current check must preserve the native status, rather
        // than turning a stale/cancelled record into an allocation failure.
        val native = ManualEditor::class.java.getDeclaredMethod("nativeReviewImage", ByteBuffer::class.java, ByteBuffer::class.java, Integer.TYPE).apply { isAccessible = true }
        val storage = ManualEditor::class.java.getDeclaredField("storage").apply { isAccessible = true }.get(editor)
        val packet = ManualEditor.ReviewComparison::class.java.getDeclaredField("packet").apply { isAccessible = true }.get(pair)
        val failure = native.invoke(editor, storage, packet, 0) as IntArray
        assertEquals(13, failure[0]); assertEquals(3, failure.size)
        val rotated = editor.prepareAutoReview(editor.requestAuto(10))
        val rotation = editor.renderReviewPreview(editor.beginReviewPreview(rotated))
        assertTrue(rotation.geometryChanged); assertTrue(editor.installReviewPreview(rotation) {})
        editor.action(2); assertFalse(editor.installReviewPreview(rotation) {})
        compose.activityRule.scenario.recreate()
        waitFor { model.state.value.image != null && !model.state.value.busy }
        assertNull(model.autoProposal.value.preview.comparison)
        assertFalse(owner().installReviewPreview(pair) {})
    }
    @Test fun recreationStopsAndJoinsPreviewBeforeDestroyWithoutRestoringPixels() {
        open(); val editor = owner(); val source = editor.state().revisionId
        assertEquals(0, hooks.control(editor, 0, 0))
        tap("review-preview-request"); waitFor { hooks.control(editor, 1) == 1 }
        val stopped = AtomicBoolean(false)
        val drain = Thread {
            val limit = System.nanoTime() + TimeUnit.SECONDS.toNanos(10)
            while (System.nanoTime() < limit) {
                try { editor.state() } catch (_: IllegalStateException) { stopped.set(true); break }
                Thread.yield()
            }
            hooks.control(editor, 2)
        }
        try {
            drain.start(); compose.activityRule.scenario.recreate(); drain.join(10_000)
            assertFalse(drain.isAlive); assertTrue(stopped.get())
            waitFor { model.state.value.image != null && !model.state.value.busy }
            assertNull(model.autoProposal.value.preview.comparison); assertNull(model.autoProposal.value.review)
            assertEquals(source, owner().state().revisionId); assertEquals(1, owner().state().revisionCount)
        } finally { hooks.control(editor, 2); if (drain.isAlive) drain.join(10_000) }
    }
}
