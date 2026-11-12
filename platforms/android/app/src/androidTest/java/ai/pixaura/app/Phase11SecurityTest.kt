package ai.pixaura.app

import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.util.UUID
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Rule
import org.junit.Test

class Phase11SecurityTest {
    @get:Rule val compose = createAndroidComposeRule<MainActivity>()
    private val app get() = InstrumentationRegistry.getInstrumentation().targetContext
    private fun waitFor(check: () -> Boolean) = compose.waitUntil(20_000, check)

    @Test fun cacheNamesCannotEscapeOrRemoveTheCurrentExportOnRejection() {
        val token = UUID.randomUUID().toString()
        val marker = File(app.cacheDir, "export/phase11-$token.png")
        val outside = File(app.cacheDir, "phase11-escape-$token.png")
        ExportDestination.cacheUri(app, marker.name, byteArrayOf(1, 2, 3))
        try {
            for (name in listOf("../${outside.name}", "", ".", "..", "/absolute.png", "..\\escape.png", "bad\u0000.png", "bad\n.png", "a".repeat(256))) {
                assertThrows(IllegalArgumentException::class.java) { ExportDestination.cacheUri(app, name, byteArrayOf(9)) }
                assertTrue("invalid name must preserve the current export", marker.exists())
                assertFalse("name escaped export cache", outside.exists())
            }
        } finally { marker.delete(); outside.delete() }
    }

    @Test fun failedImportsRemoveOnlyUnpublishedRootsAndKeepManualHistory() {
        val tests = InstrumentationRegistry.getInstrumentation().context
        val values = tests.assets.open("decode/fixtures.json").bufferedReader().use { JSONObject(it.readText()) }.getJSONObject("png").getJSONArray("bytes")
        val input = File(app.cacheDir, "phase11-${UUID.randomUUID()}.png").apply { writeBytes(ByteArray(values.length()) { values.getInt(it).toByte() }) }
        val invalid = File(app.cacheDir, "phase11-invalid-${UUID.randomUUID()}.png").apply { writeBytes(byteArrayOf(0, 1, 2, 3)) }
        val model = compose.activity.manualModel
        try {
            compose.runOnUiThread { model.importFile(input) }
            waitFor { !model.state.value.busy && model.state.value.core?.revisionCount == 1 }
            compose.runOnUiThread { model.select(4) }; waitFor { !model.state.value.busy && model.state.value.core?.active == true }
            compose.runOnUiThread { model.update(listOf(100)); model.action(1) }; waitFor { model.state.value.core?.revisionCount == 2 }
            val revision = model.state.value.core!!.revisionId
            val projects = File(app.noBackupFilesDir, "manual-projects")
            val before = projects.list()!!.toSet()
            repeat(3) {
                compose.runOnUiThread { model.importFile(invalid) }
                waitFor { !model.state.value.busy && model.state.value.error != null }
                assertEquals(before, projects.list()!!.toSet())
                assertEquals(revision, model.state.value.core!!.revisionId)
                assertEquals(2, model.state.value.core!!.revisionCount)
                assertArrayEquals(byteArrayOf(0, 1, 2, 3), invalid.readBytes())
            }
            compose.runOnUiThread { model.select(4) }; waitFor { !model.state.value.busy && model.state.value.core?.active == true }
            compose.runOnUiThread { model.update(listOf(200)); model.action(1) }; waitFor { model.state.value.core?.revisionCount == 3 }
        } finally { compose.runOnUiThread { model.closeEditor() }; input.delete(); invalid.delete() }
    }
}
