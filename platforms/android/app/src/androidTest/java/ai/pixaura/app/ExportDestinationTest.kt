package ai.pixaura.app

import android.content.Intent
import android.net.Uri
import androidx.compose.ui.test.junit4.createAndroidComposeRule
import androidx.compose.ui.test.onNodeWithTag
import androidx.compose.ui.test.onNodeWithText
import androidx.compose.ui.test.performClick
import androidx.test.platform.app.InstrumentationRegistry
import ai.pixaura.bridge.EditorFailure
import ai.pixaura.bridge.ManualEditor
import java.io.File
import java.io.IOException
import java.util.UUID
import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Rule
import org.junit.Test

class ExportDestinationTest {
    @get:Rule val compose = createAndroidComposeRule<MainActivity>()
    private val app get() = InstrumentationRegistry.getInstrumentation().targetContext
    private fun fixture(): File {
        val tests = InstrumentationRegistry.getInstrumentation().context
        val json = tests.assets.open("decode/fixtures.json").bufferedReader().use { JSONObject(it.readText()) }
        val values = json.getJSONObject("png").getJSONArray("bytes")
        return File(app.cacheDir, "export-fixture-${UUID.randomUUID()}.png").apply { writeBytes(ByteArray(values.length()) { values.getInt(it).toByte() }) }
    }
    @Test fun exportKeepsHistoryAndRejectsAStaleRevision() {
        val file = fixture()
        val root = File(app.noBackupFilesDir, "export-${UUID.randomUUID()}").apply { mkdir() }
        val editor = file.inputStream().use { ManualEditor(root.canonicalPath, it, file.length()) }
        try {
            val before = editor.state()
            val png = editor.exportEncoded(2, 0)
            assertEquals(6, png.state)
            assertTrue(png.bytes.copyOfRange(0, 8).contentEquals(byteArrayOf(0x89.toByte(), 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A)))
            val again = editor.exportEncoded(2, 0)
            assertTrue(png.bytes.contentEquals(again.bytes))
            val jpeg = editor.exportEncoded(1, 90)
            assertEquals(6, jpeg.state)
            assertTrue(jpeg.bytes.copyOfRange(0, 2).contentEquals(byteArrayOf(0xFF.toByte(), 0xD8.toByte())))
            val after = editor.state()
            assertEquals(before.revisionCount, after.revisionCount)
            assertEquals(before.revisionId, after.revisionId)
            val stale = runCatching { editor.exportEncoded(2, 0, revisionId = "0123456789abcdef0123456789abcdef") }
            val failure = stale.exceptionOrNull() as EditorFailure
            assertEquals(9, failure.status)
            assertEquals(before.revisionId, editor.state().revisionId)
        } finally { editor.close() }
    }
    @Test fun partialDestinationIsRemovedAndShareIsNotDelivery() {
        val file = File(app.cacheDir, "partial-${UUID.randomUUID()}.bin")
        val uri = Uri.fromFile(file)
        var published = false
        val failure = runCatching {
            ExportDestination.savePending(
                insert = { file.writeBytes(ByteArray(0)); uri },
                open = { throw IOException("disk denied") },
                publish = { published = true },
                remove = { file.delete() },
                bytes = byteArrayOf(1, 2, 3))
        }
        assertTrue(failure.exceptionOrNull() is IOException)
        assertFalse(published)
        assertFalse(file.exists())
        val share = ExportDestination.shareIntent(uri, "image/png")
        assertEquals(Intent.ACTION_SEND, share.action)
        assertEquals("image/png", share.type)
        assertTrue(share.flags and Intent.FLAG_GRANT_READ_URI_PERMISSION != 0)
        compose.onNodeWithText("Open Manual editor").performClick()
        compose.runOnUiThread { compose.activity.manualModel.importFile(fixture()) }
        compose.waitUntil(20_000) { compose.activity.manualModel.state.value.image != null && !compose.activity.manualModel.state.value.busy }
        compose.onNodeWithTag("export-share").assertExists()
        compose.onNodeWithTag("export-save-file").assertExists()
        compose.onNodeWithText("JPEG").assertExists()
        compose.onNodeWithText("PNG").assertExists()
    }
}
