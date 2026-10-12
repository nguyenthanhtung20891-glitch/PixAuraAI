package ai.pixaura.app

import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.lifecycle.Lifecycle
import android.graphics.Bitmap
import android.graphics.Color
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Assert.*
import org.junit.Rule
import org.junit.Test
import java.io.File
import java.util.UUID
import java.util.concurrent.CountDownLatch
import java.util.concurrent.ExecutorService
import java.util.concurrent.TimeUnit

class AndroidCapabilityLifecycleTest {
    @get:Rule val compose = createAndroidComposeRule<MainActivity>()
    @Test fun applicationOwnerPressureActiveEditBackgroundAndRecreation() {
        val app = InstrumentationRegistry.getInstrumentation().targetContext.applicationContext as PixAuraApplication
        val service = app.capabilityOwner
        val file = File(app.cacheDir, "pressure-${UUID.randomUUID()}.png")
        val bitmap = Bitmap.createBitmap(1024, 512, Bitmap.Config.ARGB_8888)
        try {
            bitmap.eraseColor(Color.rgb(40, 80, 120)); file.outputStream().use { assertTrue(bitmap.compress(Bitmap.CompressFormat.PNG, 100, it)) }
            var model = compose.activity.manualModel
            compose.runOnUiThread { model.importFile(file) }
            compose.waitUntil(20_000) { model.state.value.image != null && !model.state.value.previewLoading && !model.state.value.busy }
            assertEquals(1, service.subscriptionCount())
            val exposure = model.state.value.tools.single { it.name == "Exposure" }
            compose.runOnUiThread { model.select(exposure.index) }
            compose.waitUntil(20_000) { model.state.value.core?.active == true && !model.state.value.busy }
            compose.runOnUiThread { model.update(listOf(1234)) }
            compose.waitUntil(20_000) { model.state.value.core?.values == listOf(1234) && !model.state.value.previewLoading }
            val revision = model.state.value.core?.revisionCount
            val commands = ManualEditorModel::class.java.getDeclaredField("commands").apply { isAccessible = true }.get(model) as ExecutorService
            val blocked = CountDownLatch(1); val release = CountDownLatch(1)
            commands.execute { blocked.countDown(); check(release.await(20, TimeUnit.SECONDS)) }
            try {
                assertTrue(blocked.await(20, TimeUnit.SECONDS))
                service.observeThermal(4); service.observeThermal(1)
                // A later clear notification cannot erase a pending downgrade.
            } finally { release.countDown() }
            compose.waitUntil(20_000) { model.capabilityStrategy.value?.edge == 360 && model.state.value.image?.width == 360 && !model.state.value.previewLoading }
            assertEquals(listOf(1234), model.state.value.core?.values); assertEquals(revision, model.state.value.core?.revisionCount)
            assertTrue(model.state.value.core?.active == true)
            compose.runOnUiThread { model.action(1) }
            compose.waitUntil(20_000) { model.state.value.core?.active == false && !model.state.value.busy }
            assertEquals(requireNotNull(revision) + 1, model.state.value.core?.revisionCount)
            compose.runOnUiThread { model.select(exposure.index) }
            compose.waitUntil(20_000) { model.state.value.core?.active == true && !model.state.value.busy }
            compose.runOnUiThread { model.update(listOf(-1000)) }
            compose.waitUntil(20_000) { model.state.value.core?.values == listOf(-1000) && !model.state.value.previewLoading }
            service.observeTrim(15)
            compose.waitUntil(20_000) { model.capabilityStrategy.value?.tier == 0 }
            compose.runOnUiThread { model.action(0) }
            compose.waitUntil(20_000) { model.state.value.core?.active == false && !model.state.value.busy }
            assertEquals(revision + 1, model.state.value.core?.revisionCount)
            compose.activityRule.scenario.moveToState(Lifecycle.State.CREATED)
            compose.activityRule.scenario.moveToState(Lifecycle.State.RESUMED)
            compose.waitUntil(20_000) { !model.state.value.busy }
            assertEquals(revision + 1, model.state.value.core?.revisionCount)
            repeat(3) {
                compose.activityRule.scenario.recreate(); model = compose.activity.manualModel
                compose.waitUntil(20_000) { model.state.value.image != null && !model.state.value.busy && !model.state.value.previewLoading }
                assertEquals(1, service.subscriptionCount()); assertEquals(revision + 1, model.state.value.core?.revisionCount)
            }
        } finally { bitmap.recycle(); file.delete() }
    }
}
