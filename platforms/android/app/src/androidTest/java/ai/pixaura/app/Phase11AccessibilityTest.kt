package ai.pixaura.app

import android.app.Application
import androidx.activity.compose.LocalActivityResultRegistryOwner
import android.content.res.Configuration
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.runtime.CompositionLocalProvider
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalConfiguration
import androidx.compose.ui.platform.testTag
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.platform.LocalDensity
import androidx.compose.ui.semantics.SemanticsActions
import androidx.compose.ui.test.*
import androidx.compose.ui.test.junit4.createComposeRule
import androidx.compose.ui.unit.Density
import androidx.compose.ui.unit.dp
import androidx.test.platform.app.InstrumentationRegistry
import ai.pixaura.app.ui.PixAuraTheme
import java.io.File
import java.util.Locale
import java.util.UUID
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Rule
import org.junit.Test

class Phase11AccessibilityTest {
    @get:Rule val compose = createComposeRule()
    @Test fun englishLargeTextAndAdjustableInput() = checkJourney("en", "Exposure", "Exposure value", "Share the committed revision")
    @Test fun vietnameseLargeTextAndAdjustableInput() = checkJourney("vi", "Phơi sáng", "Giá trị Phơi sáng", "Chia sẻ bản sửa đã lưu")

    private fun checkJourney(language: String, exposure: String, sliderLabel: String, shareLabel: String) {
        val app = InstrumentationRegistry.getInstrumentation().targetContext
        val config = Configuration(app.resources.configuration).apply { setLocale(Locale.forLanguageTag(language)); fontScale = 2f }
        val localized = app.createConfigurationContext(config)
        lateinit var model: ManualEditorModel
        compose.runOnUiThread { model = ManualEditorModel(app.applicationContext as Application) }
        compose.setContent {
            val registry = checkNotNull(LocalActivityResultRegistryOwner.current)
            CompositionLocalProvider(LocalActivityResultRegistryOwner provides registry, LocalContext provides localized, LocalConfiguration provides config,
                LocalDensity provides Density(app.resources.displayMetrics.density, 2f)) {
                PixAuraTheme { Column(Modifier.verticalScroll(rememberScrollState()).testTag("phase11-scroll")) { ManualEditorScreen(model) } }
            }
        }
        val tests = InstrumentationRegistry.getInstrumentation().context
        val bytes = tests.assets.open("decode/fixtures.json").bufferedReader().use { JSONObject(it.readText()) }.getJSONObject("png").getJSONArray("bytes")
        val input = File(app.cacheDir, "phase11-ax-${UUID.randomUUID()}.png").apply { writeBytes(ByteArray(bytes.length()) { bytes.getInt(it).toByte() }) }
        try {
            compose.runOnUiThread { model.importFile(input) }
            compose.waitUntil(20_000) { !model.state.value.busy && model.state.value.core?.revisionCount == 1 }
            compose.onNodeWithText(if (language == "vi") "Tông/Màu" else "Tone/Color").performScrollTo().performClick()
            compose.onNodeWithText(exposure).performScrollTo().performClick()
            compose.waitUntil(20_000) { model.state.value.core?.selected == 4 && !model.state.value.busy }
            compose.onNodeWithContentDescription(sliderLabel).performScrollTo().performSemanticsAction(SemanticsActions.SetProgress) { it(100f) }
            compose.waitUntil(20_000) { model.state.value.core?.values == listOf(100) }
            assertEquals(1, model.state.value.core!!.revisionCount)
            compose.runOnUiThread { model.action(0) }
            compose.waitUntil(20_000) { !model.state.value.busy && model.state.value.core?.active == false }
            compose.runOnUiThread { model.select(4) }
            compose.waitUntil(20_000) { !model.state.value.busy && model.state.value.core?.active == true }
            compose.runOnUiThread { model.update(listOf(Int.MAX_VALUE)) }
            compose.waitUntil(20_000) { model.state.value.error != null }
            compose.onNodeWithText(if (language == "vi") "Tham số không hợp lệ (core 7)" else "Invalid parameter (core 7)").performScrollTo().assertIsDisplayed()
            compose.runOnUiThread { model.action(0) }
            compose.waitUntil(20_000) { !model.state.value.busy && model.state.value.core?.active == false }
            val ownerField = ManualEditorModel::class.java.getDeclaredField("owner").apply { isAccessible = true }
            val owner = ownerField.get(model) as ai.pixaura.bridge.ManualEditor
            val assistHooks = AssistTestHooks()
            val autoHooks = AutoTestHooks()
            compose.runOnUiThread { model.setAssistMode(true) }
            compose.waitUntil(20_000) { model.assistance.value.choices.size == 11 }
            compose.onNodeWithTag("assist-goal").performScrollTo().performClick()
            compose.onNodeWithTag("assist-goal-pixaura.exposure").performScrollTo().performClick()
            assertEquals(0, assistHooks.control(owner, 3, 2))
            compose.onNodeWithTag("assist-request").performScrollTo().performClick()
            compose.waitUntil(20_000) { !model.assistance.value.inFlight && model.assistance.value.error != null }
            compose.onNodeWithTag("assist-error").performScrollTo().assertTextEquals(if (language == "vi") "Không thể xác thực gợi ý. Bạn có thể tiếp tục chỉnh sửa thủ công." else "Advice could not be validated. You can keep editing manually.")
            assertEquals(0, assistHooks.control(owner, 3, 0))
            compose.runOnUiThread { model.setAutoMode(true) }
            compose.waitUntil(20_000) { model.autoProposal.value.choices.size == 11 }
            compose.onNodeWithTag("auto-goal").performScrollTo().performClick()
            compose.onNodeWithTag("auto-goal-pixaura.exposure").performScrollTo().performClick()
            assertEquals(0, autoHooks.control(owner, 3, 2))
            compose.onNodeWithTag("auto-request").performScrollTo().performClick()
            compose.waitUntil(20_000) { !model.autoProposal.value.inFlight && model.autoProposal.value.error != null }
            compose.onNodeWithTag("auto-error").performScrollTo().assertTextEquals(if (language == "vi") "Không thể xác thực đề xuất. Chưa áp dụng thay đổi nào." else "Proposal could not be validated. Nothing was applied.")
            assertEquals(0, autoHooks.control(owner, 3, 0))
            compose.runOnUiThread { model.setAutoMode(false) }
            compose.waitUntil(20_000) { !model.state.value.busy && !model.autoProposal.value.enabled }
            assertEquals(1, model.state.value.core!!.revisionCount)
            for (tag in listOf("export-save-photos", "export-save-file", "export-share")) {
                // Each target must be reachable from the top of the outer page;
                // an inner horizontal row must not steal vertical scroll-to.
                compose.onNodeWithTag("phase11-scroll").performSemanticsAction(SemanticsActions.ScrollBy) { it(0f, -10000f) }
                compose.onNodeWithTag(tag).performScrollTo().assertIsDisplayed().assertWidthIsAtLeast(48.dp).assertHeightIsAtLeast(48.dp)
            }
            compose.onNodeWithTag("export-share").assertContentDescriptionEquals(shareLabel)
            assertEquals(1, model.state.value.core!!.revisionCount)
        } finally { compose.runOnUiThread { model.closeEditor() }; input.delete() }
    }
}
