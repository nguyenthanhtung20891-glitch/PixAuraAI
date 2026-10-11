package ai.pixaura.app

import ai.pixaura.bridge.CoreProbe
import androidx.test.platform.app.InstrumentationRegistry
import java.util.UUID
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ManualBoundaryTest {
    @Test fun integratedGestureOwnsPreviewAndTerminalLifecycle() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val root = java.io.File(instrumentation.targetContext.noBackupFilesDir, "gesture-${UUID.randomUUID()}")
        assertTrue(root.mkdir())
        val executor = java.util.concurrent.Executors.newSingleThreadExecutor()
        try {
            val fixture = JSONObject(instrumentation.context.assets.open("decode.json").bufferedReader().use { it.readText() }).getJSONObject("png")
            val array = fixture.getJSONArray("bytes")
            val bytes = ByteArray(array.length()) { array.getInt(it).toByte() }
            val directory = java.io.File(root, "assets/sha256")
            assertTrue(directory.mkdirs())
            val digest = fixture.getString("sha256")
            java.io.File(directory, digest).writeBytes(bytes)
            val identity = UUID.randomUUID().toString().replace("-", "").toByteArray()
            assertEquals(0, executor.submit<Int> { CoreProbe().nativeGestureLifecycle(root.canonicalPath.toByteArray(), digest.toByteArray(), bytes.size.toLong(), identity) }.get())
        } finally { executor.shutdown(); assertTrue(root.deleteRecursively()) }
    }
    @Test fun toneGestureBoundary() {
        val fixture = InstrumentationRegistry.getInstrumentation().context.assets
            .open("image-document-v1.json").use { it.readBytes() }
        fun identity() = UUID.randomUUID().toString().replace("-", "").toByteArray(Charsets.US_ASCII)
        assertEquals(0, CoreProbe().nativeToneBoundary(fixture, identity()))
        assertTrue(CoreProbe().nativeToneBoundary("{".toByteArray(), identity()) != 0)
    }
    @Test fun geometryGestureBoundary() {
        val fixture = InstrumentationRegistry.getInstrumentation().context.assets
            .open("image-document-v1.json").use { it.readBytes() }
        fun identity() = UUID.randomUUID().toString().replace("-", "").toByteArray(Charsets.US_ASCII)
        assertEquals(0, CoreProbe().nativeGeometryBoundary(fixture, identity()))
        assertTrue(CoreProbe().nativeGeometryBoundary("{".toByteArray(), identity()) != 0)
    }
    @Test fun sharedOfflineRegistry() {
        val bytes = requireNotNull(CoreProbe().nativeManualRegistry())
        assertTrue(bytes.size <= 32768 && bytes.last() == 10.toByte())
        val registry = JSONObject(bytes.toString(Charsets.UTF_8))
        assertEquals(1, registry.getInt("descriptor_version"))
        val tools = registry.getJSONArray("tools")
        val ids = listOf("blur", "brightness", "contrast", "crop", "exposure", "highlights", "rotate", "saturation", "shadows", "sharpen", "temperature")
        assertEquals(ids.size, tools.length())
        ids.forEachIndexed { index, id -> assertEquals("pixaura.$id", tools.getJSONObject(index).getString("tool_id")) }
        val exposure = tools.getJSONObject(4).getJSONArray("parameters").getJSONObject(0)
        assertEquals(-5000, exposure.getInt("minimum"))
        assertEquals(5000, exposure.getInt("maximum"))
        assertEquals(0, exposure.getInt("default"))
    }
    @Test fun presetsUseSharedCanonicalAndBatchProposalBoundary() {
        val fixture = InstrumentationRegistry.getInstrumentation().context.assets.open("image-document-v1.json").use { it.readBytes() }
        val identity = java.util.UUID.randomUUID().toString().replace("-", "").toByteArray(Charsets.US_ASCII)
        assertEquals(0, CoreProbe().nativePresetBoundary(fixture, identity))
    }

}
