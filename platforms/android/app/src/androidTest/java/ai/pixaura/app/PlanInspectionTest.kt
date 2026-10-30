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

// Instrumentation only: existing provider kind-3 admission, never approval.
class InspectionTestHooks {
    private external fun nativeCandidate(context: ByteBuffer, plan: ByteArray, output: ByteBuffer): Int
    fun candidate(editor: ManualEditor, bytes: ByteArray): ManualEditor.AutoProposal {
        val storage = ManualEditor::class.java.getDeclaredField("storage").apply { isAccessible = true }.get(editor) as ByteBuffer
        val packet = ByteBuffer.allocateDirect(328).order(ByteOrder.nativeOrder())
        assertEquals(0, nativeCandidate(storage, bytes, packet))
        fun id(at: Int) = ByteArray(32) { packet.get(at + it) }.takeWhile { it != 0.toByte() }.toByteArray().toString(Charsets.US_ASCII)
        val constructor = ManualEditor.AutoProposal::class.java.declaredConstructors.single { it.parameterCount == 5 }.apply { isAccessible = true }
        // Action presentation is reconstructed by native inspection, not this test wrapper.
        return constructor.newInstance(editor, packet.asReadOnlyBuffer(), emptyList<ManualEditor.AssistChoice>(), id(72), id(104)) as ManualEditor.AutoProposal
    }
}
@OptIn(ExperimentalTestApi::class)
class PlanInspectionTest {
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
    @Test fun allElevenActionsExposeExactNativeValuesAndStaleUnsupportedStates() {
        open(); val owner = editor(); val source = owner.state().revisionId
        for (choice in owner.adviceChoices()) {
            compose.runOnUiThread { model.chooseAuto(choice.index) }; barrier()
            compose.runOnUiThread { model.requestAuto() }; waitFor { model.autoProposal.value.status == AutoStatus.REVIEW_READY && !model.autoProposal.value.inFlight }
            val review = requireNotNull(model.autoProposal.value.review); val inspection = owner.inspectReview(review)
            assertEquals(1, inspection.actions.size); val a = inspection.actions.single()
            try { (inspection.actions as MutableList).clear(); fail("inspection list was mutable") } catch (_: UnsupportedOperationException) { }
            assertEquals(choice.values, a.parameters.map { it.raw })
            compose.onNodeWithTag("review-action-list").assertExists()
            compose.onNodeWithTag("review-plan-count").assertTextContains("1 planned action(s).", substring = true)
            compose.onNodeWithTag("review-action-0").performScrollTo().assertIsDisplayed().assertTextContains("Action 1 of 1: ${a.label}")
            compose.onNodeWithTag("auto-exact-values-0", useUnmergedTree = true).assertTextEquals(a.parameters.joinToString("; ") { "${it.name} = ${it.raw} (${it.unit})" })
            compose.onNodeWithTag("review-action-detail-0", useUnmergedTree = true).assertExists()
            if (a.geometry) compose.onNodeWithTag("review-geometry-warning-0", useUnmergedTree = true).assertExists()
            compose.onNodeWithTag("review-preview-request").assertExists(); compose.onNodeWithTag("auto-apply").assertExists()
            compose.onNodeWithTag("review-action-0").assert(hasClickAction().not())
            assertEquals(source, owner.state().revisionId); assertEquals(1, owner.state().revisionCount)
        }
        compose.runOnUiThread { model.select(4) }; waitFor { model.autoProposal.value.status == AutoStatus.STALE && !model.state.value.busy }
        compose.onNodeWithTag("review-inspection-stale").assertExists(); compose.onNodeWithTag("auto-apply").assertDoesNotExist()
        tap("mode-manual"); compose.onNodeWithText("Cancel", useUnmergedTree = true).performScrollTo().performClick(); waitFor { model.state.value.core?.active == false }
        tap("mode-auto"); barrier(); assertEquals(0, AutoTestHooks().control(owner, 3, 3))
        compose.runOnUiThread { model.requestAuto() }; waitFor { model.autoProposal.value.status == AutoStatus.FAILED && !model.autoProposal.value.inFlight }
        compose.onNodeWithTag("review-inspection-unsupported").assertExists(); compose.onNodeWithTag("review-action-list").assertDoesNotExist(); compose.onNodeWithTag("auto-apply").assertDoesNotExist()
        assertEquals(source, owner.state().revisionId)
    }
    @Test fun eightActionPresentationPreservesNativeOrderPrecisionAndPreviewAssociation() {
        open(); val owner = editor(); barrier()
        val actions = (0..7).joinToString(",") { i -> "{\"action_index\":$i,\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\"milli_ev\":${101+i}},\"tool_id\":\"pixaura.exposure\"}" }
        val bytes = "{\"actions\":[$actions],\"binding\":${owner.agentBinding().toString(Charsets.UTF_8).trim()},\"plan_version\":1}\n".toByteArray()
        val proposal = InspectionTestHooks().candidate(owner, bytes); val review = owner.prepareAutoReview(proposal)
        @Suppress("UNCHECKED_CAST") val mutable = ManualEditorModel::class.java.getDeclaredField("autoMutable").apply { isAccessible = true }.get(model) as MutableStateFlow<AutoUiState>
        compose.runOnUiThread { mutable.value = mutable.value.copy(status = AutoStatus.REVIEW_READY, proposal = proposal, review = review, inFlight = false) }
        compose.onNodeWithTag("review-plan-count").assertTextContains("8 planned action(s).", substring = true)
        for (i in 0..7) {
            compose.onNodeWithTag("review-action-$i").performScrollTo().assertIsDisplayed().assertTextContains("Action ${i+1} of 8: Exposure")
            compose.onNodeWithTag("auto-exact-values-$i", useUnmergedTree = true).assertTextEquals("milli_ev = ${101+i} (thousandths_exposure_stop)")
        }
        compose.onNodeWithTag("review-action-8").assertDoesNotExist(); compose.onNodeWithTag("review-order-warning").assertExists()
        tap("review-preview-request"); waitFor { model.autoProposal.value.preview.status == ReviewPreviewStatus.PREVIEW_READY }
        compose.onNodeWithTag("review-before").assertExists(); compose.onNodeWithTag("auto-apply").assertExists()
        assertEquals(1, owner.state().revisionCount)
    }
}
