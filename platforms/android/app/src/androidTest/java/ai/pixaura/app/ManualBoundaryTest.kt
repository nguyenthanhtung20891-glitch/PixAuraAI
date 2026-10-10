package ai.pixaura.app

import ai.pixaura.bridge.CoreProbe
import androidx.test.platform.app.InstrumentationRegistry
import java.util.UUID
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ManualBoundaryTest {
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
        val ids = listOf("brightness", "contrast", "crop", "exposure", "highlights", "rotate", "saturation", "shadows", "temperature")
        assertEquals(ids.size, tools.length())
        ids.forEachIndexed { index, id -> assertEquals("pixaura.$id", tools.getJSONObject(index).getString("tool_id")) }
        val exposure = tools.getJSONObject(3).getJSONArray("parameters").getJSONObject(0)
        assertEquals(-5000, exposure.getInt("minimum"))
        assertEquals(5000, exposure.getInt("maximum"))
        assertEquals(0, exposure.getInt("default"))
    }
}
