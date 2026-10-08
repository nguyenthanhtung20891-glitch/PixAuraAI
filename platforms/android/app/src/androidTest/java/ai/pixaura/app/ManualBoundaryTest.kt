package ai.pixaura.app

import ai.pixaura.bridge.CoreProbe
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ManualBoundaryTest {
    @Test fun sharedOfflineRegistry() {
        val bytes = requireNotNull(CoreProbe().nativeManualRegistry())
        assertTrue(bytes.size <= 32768 && bytes.last() == 10.toByte())
        val registry = JSONObject(bytes.toString(Charsets.UTF_8))
        assertEquals(1, registry.getInt("descriptor_version"))
        val tools = registry.getJSONArray("tools")
        assertEquals(3, tools.length())
        assertEquals("pixaura.crop", tools.getJSONObject(0).getString("tool_id"))
        assertEquals("pixaura.exposure", tools.getJSONObject(1).getString("tool_id"))
        assertEquals("pixaura.rotate", tools.getJSONObject(2).getString("tool_id"))
        val exposure = tools.getJSONObject(1).getJSONArray("parameters").getJSONObject(0)
        assertEquals(-5000, exposure.getInt("minimum"))
        assertEquals(5000, exposure.getInt("maximum"))
        assertEquals(0, exposure.getInt("default"))
    }
}
