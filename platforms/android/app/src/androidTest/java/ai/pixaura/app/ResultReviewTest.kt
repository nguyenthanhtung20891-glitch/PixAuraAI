package ai.pixaura.app

import ai.pixaura.bridge.*
import androidx.compose.ui.semantics.SemanticsProperties
import androidx.compose.ui.semantics.getOrNull
import androidx.compose.ui.test.*
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.util.UUID
import java.util.concurrent.ExecutorService
import java.util.concurrent.TimeUnit
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Rule
import org.junit.Test

@OptIn(ExperimentalTestApi::class)
class ResultReviewTest {
    // The real UI looper must deliver ordinary-history updates. A paused test
    // dispatcher keeps the first "Committed result" composition forever.
    @get:Rule val compose = createAndroidComposeRule<MainActivity>()
    private val model get() = compose.activity.manualModel
    private fun waitFor(test: () -> Boolean) = compose.waitUntil(20_000, test)
    private fun editor() = ManualEditorModel::class.java.getDeclaredField("owner").apply { isAccessible = true }.get(model) as ManualEditor
    private fun barrier() { (ManualEditorModel::class.java.getDeclaredField("commands").apply { isAccessible = true }.get(model) as ExecutorService).submit {}.get(10, TimeUnit.SECONDS) }
    private fun tap(tag: String) { compose.onNodeWithTag(tag).performScrollTo().performClick() }
    @Test fun applyShowsCommittedOrdinaryRevisionAndUndoRedoBackToEditing() {
        compose.onNodeWithText("Open Manual editor").performClick()
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val values = instrumentation.context.assets.open("decode/fixtures.json").bufferedReader().use { JSONObject(it.readText()).getJSONObject("png").getJSONArray("bytes") }
        val file = File(instrumentation.targetContext.cacheDir, "result-${UUID.randomUUID()}.png")
        file.writeBytes(ByteArray(values.length()) { values.getInt(it).toByte() })
        compose.runOnUiThread { model.importFile(file) }; waitFor { model.state.value.image != null && !model.state.value.busy }; file.delete()
        tap("mode-auto"); waitFor { model.autoProposal.value.choices.size == 11 }
        compose.runOnUiThread { model.chooseAuto(2) }; barrier(); tap("auto-request")
        waitFor { model.autoProposal.value.review != null && !model.autoProposal.value.inFlight }
        tap("auto-apply"); waitFor { model.autoProposal.value.status == AutoStatus.COMPLETED && model.autoProposal.value.result != null }
        val result = requireNotNull(model.autoProposal.value.result)
        assertEquals(1, result.state); assertEquals(2, result.count); assertTrue(result.publicationVisible)
        assertEquals(2, editor().state().revisionCount); assertEquals(result.result, editor().state().revisionId)
        compose.onNodeWithTag("result-label").performScrollTo().assertTextContains("Committed result")
        compose.onNodeWithTag("auto-completed").assertExists()
        tap("result-previous"); waitFor { model.autoProposal.value.result?.showing == "previous" && model.autoProposal.value.result?.image != null }
        tap("result-current"); waitFor { model.autoProposal.value.result?.showing == "current" && model.autoProposal.value.result?.image != null }
        tap("result-undo"); waitFor {
            val review = model.autoProposal.value.result
            review?.state == 2 && review.canRedo && review.live == result.previous
        }
        compose.waitForIdle()
        assertEquals(2, editor().state().revisionCount); assertEquals(result.previous, editor().state().revisionId)
        assertLabelContains("stale")
        tap("result-redo"); waitFor {
            val review = model.autoProposal.value.result
            review?.state == 1 && review.canUndo && review.live == result.result
        }
        compose.waitForIdle()
        assertLabelContains("Committed result")
        assertEquals(2, editor().state().revisionCount); assertEquals(result.result, editor().state().revisionId)
        tap("result-back"); waitFor { !model.autoProposal.value.enabled && model.autoProposal.value.result == null }
        compose.onNodeWithTag("result-review").assertDoesNotExist()
    }
    private fun assertLabelContains(expected: String) {
        val deadline = System.nanoTime() + TimeUnit.SECONDS.toNanos(20)
        var seen = ""
        while (System.nanoTime() < deadline) {
            compose.waitForIdle()
            InstrumentationRegistry.getInstrumentation().waitForIdleSync()
            seen = compose.onAllNodesWithTag("result-label").fetchSemanticsNodes().joinToString(" | ") { node ->
                val text = node.config.getOrNull(SemanticsProperties.Text)?.joinToString { it.text }.orEmpty()
                val spoken = node.config.getOrNull(SemanticsProperties.ContentDescription)?.joinToString().orEmpty()
                "$text / $spoken"
            }
            if (seen.contains(expected)) return
            Thread.sleep(50)
        }
        fail("result label '$seen' did not contain '$expected'")
    }
}
