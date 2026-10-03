package ai.pixaura.app

import ai.pixaura.bridge.CoreProbe
import androidx.test.platform.app.InstrumentationRegistry
import java.util.UUID
import org.junit.Assert.assertArrayEquals
import org.junit.Assert.assertNull
import org.junit.Test

class DocumentBoundaryTest {
    private fun identity(): ByteArray = UUID.randomUUID().toString()
        .replace("-", "").toByteArray(Charsets.US_ASCII)

    @Test fun canonicalNativeDocumentFixture() {
        val fixture = InstrumentationRegistry.getInstrumentation().context.assets
            .open("image-document-v1.json").use { it.readBytes() }
        val bridge = CoreProbe()
        assertArrayEquals(fixture, bridge.nativeDocumentRoundTrip(fixture, identity(), identity()))
        assertNull(bridge.nativeDocumentRoundTrip("{".toByteArray(), identity(), identity()))
        assertNull(bridge.nativeDocumentRoundTrip(fixture, byteArrayOf(0), identity()))
        val future = fixture.toString(Charsets.UTF_8).replace("\"schema_version\":1", "\"schema_version\":2")
        assertNull(bridge.nativeDocumentRoundTrip(future.toByteArray(), identity(), identity()))
    }
}
