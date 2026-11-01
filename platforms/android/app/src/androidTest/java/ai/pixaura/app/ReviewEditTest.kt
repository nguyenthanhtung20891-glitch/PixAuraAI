package ai.pixaura.app

import ai.pixaura.bridge.*
import androidx.compose.ui.test.*
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.util.UUID
import java.util.concurrent.ExecutorService
import java.util.concurrent.TimeUnit
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.test.StandardTestDispatcher
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Rule
import org.junit.Test


@OptIn(ExperimentalTestApi::class)
class ReviewEditTest {
    @get:Rule val compose = createAndroidComposeRule<MainActivity>(effectContext = StandardTestDispatcher())
    private val model get() = compose.activity.manualModel
    private fun waitFor(test: () -> Boolean) = compose.waitUntil(20_000, test)
    private fun editor() = ManualEditorModel::class.java.getDeclaredField("owner").apply { isAccessible = true }.get(model) as ManualEditor
    private fun barrier() { (ManualEditorModel::class.java.getDeclaredField("commands").apply { isAccessible = true }.get(model) as ExecutorService).submit {}.get(10, TimeUnit.SECONDS) }
    private fun tap(tag: String) { if (tag.startsWith("mode-")) compose.onNodeWithTag("mode-selector").performScrollTo(); compose.onNodeWithTag(tag).performScrollTo().performClick() }
    private fun open() {
        compose.onNodeWithText("Open Manual editor").performClick()
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val values = instrumentation.context.assets.open("decode/fixtures.json").bufferedReader().use { JSONObject(it.readText()).getJSONObject("png").getJSONArray("bytes") }
        val file = File(instrumentation.targetContext.cacheDir, "inspection-${UUID.randomUUID()}.png")
        file.writeBytes(ByteArray(values.length()) { values.getInt(it).toByte() })
        compose.runOnUiThread { model.importFile(file) }; waitFor { model.state.value.image != null && !model.state.value.busy }; file.delete()
        tap("mode-auto"); waitFor { model.autoProposal.value.choices.size == 11 }
    }
    private fun request() { compose.runOnUiThread { model.chooseAuto(2) }; barrier(); tap("auto-request"); waitFor { model.autoProposal.value.review != null && !model.autoProposal.value.inFlight } }
    private fun settled() = waitFor { !model.autoProposal.value.inFlight }
    @Test fun exactValueRestoreNoActionsPreviewInvalidationAndHumanApply() {
        open(); request(); val owner = editor(); val original = requireNotNull(model.autoProposal.value.review); val bytes = original.canonicalBytes()
        tap("review-preview-request"); waitFor { model.autoProposal.value.preview.status == ReviewPreviewStatus.PREVIEW_READY }
        val pair = requireNotNull(model.autoProposal.value.preview.comparison)
        compose.onNodeWithTag("review-edit-value-0-0").performScrollTo().performTextReplacement("123")
        compose.onNodeWithTag("auto-apply").assertIsNotEnabled()
        compose.onNodeWithTag("review-edit-draft-0-0").assertExists()
        tap("review-edit-change-0-0"); settled()
        val edited = requireNotNull(model.autoProposal.value.review)
        assertNotSame(original, edited); assertEquals(123, edited.inspection.actions[0].parameters[0].raw); assertArrayEquals(bytes, original.canonicalBytes())
        compose.onNodeWithTag("auto-exact-values-0").assertTextContains("milli_ev = 123", substring = true)
        compose.onNodeWithTag("review-edit-state").assertTextContains("EDITED", substring = true)
        assertFalse(owner.installReviewPreview(pair) {}); assertEquals(1, owner.state().revisionCount)
        try { owner.applyAutoFromHuman(original); fail("old review applied") } catch (_: EditorFailure) { }
        tap("review-edit-restore"); settled(); assertArrayEquals(bytes, requireNotNull(model.autoProposal.value.review).canonicalBytes())
        tap("review-edit-remove-0"); settled(); assertNull(model.autoProposal.value.review)
        compose.onNodeWithTag("review-edit-state").assertTextContains("NO_ACTIONS", substring = true)
        compose.onNodeWithTag("auto-apply").assertDoesNotExist(); compose.onNodeWithTag("review-preview-request").assertDoesNotExist()
        tap("review-edit-restore"); settled()
        compose.onNodeWithTag("review-edit-value-0-0").performScrollTo().performTextReplacement("-250"); tap("review-edit-change-0-0"); settled()
        assertEquals(-250, requireNotNull(model.autoProposal.value.review).inspection.actions[0].parameters[0].raw)
        tap("auto-apply"); waitFor { model.autoProposal.value.status == AutoStatus.COMPLETED && !model.autoProposal.value.inFlight }
        assertEquals(2, owner.state().revisionCount)
    }
    @Test fun invalidRawValueRetiresReviewAndNativeMultiActionOrderCannotBeUiPatched() {
        open(); request(); val owner = editor(); val old = requireNotNull(model.autoProposal.value.review)
        compose.onNodeWithTag("review-edit-value-0-0").performScrollTo().performTextReplacement("999999"); tap("review-edit-change-0-0"); settled()
        assertNull(model.autoProposal.value.review); compose.onNodeWithTag("auto-apply").assertDoesNotExist()
        compose.onNodeWithTag("review-edit-state").assertTextContains("EDIT_FAILED", substring = true)
        try { owner.inspectReview(old); fail("invalid edit restored old review") } catch (_: EditorFailure) { }
        tap("review-edit-restore"); settled(); assertEquals(1, owner.state().revisionCount)
        val binding = owner.agentBinding().toString(Charsets.UTF_8).trim()
        val actions = (0..7).joinToString(",") {
            if (it == 3) "{\"action_index\":3,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"height_ppm\":800000,\"width_ppm\":800000,\"x_ppm\":100000,\"y_ppm\":100000},\"tool_id\":\"pixaura.crop\"}"
            else "{\"action_index\":$it,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":${101+it}},\"tool_id\":\"pixaura.exposure\"}"
        }
        val proposal = InspectionTestHooks().candidate(owner, "{\"actions\":[$actions],\"binding\":$binding,\"plan_version\":1}\n".toByteArray())
        val review = owner.prepareAutoReview(proposal)
        @Suppress("UNCHECKED_CAST") val flow = ManualEditorModel::class.java.getDeclaredField("autoMutable").apply { isAccessible = true }.get(model) as MutableStateFlow<AutoUiState>
        compose.runOnUiThread { flow.value = flow.value.copy(proposal = proposal, review = review, editSession = null, preview = ReviewPreviewUi()) }
        tap("review-edit-down-0"); settled()
        assertEquals(listOf(102,101,103,800000,105,106,107,108), requireNotNull(model.autoProposal.value.review).inspection.actions.map { it.parameters[0].raw })
        compose.onNodeWithTag("auto-exact-values-0").assertTextContains("milli_ev = 102", substring = true)
        tap("review-edit-remove-3"); settled(); assertEquals(7, requireNotNull(model.autoProposal.value.review).inspection.actions.size)
        assertFalse(requireNotNull(model.autoProposal.value.review).inspection.actions.any { it.geometry })
        tap("review-edit-restore"); settled(); assertArrayEquals(review.canonicalBytes(), requireNotNull(model.autoProposal.value.review).canonicalBytes())
        assertEquals(1, owner.state().revisionCount)
    }
}
