package ai.pixaura.app

import ai.pixaura.bridge.CoreProbe
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.util.UUID
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class StorageBoundaryTest {
    @Test fun nativeStorageCreatesAndReopens() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val root = File(context.noBackupFilesDir, "storage-test-${UUID.randomUUID()}")
        assertTrue(root.mkdir())
        try {
            val bridge = CoreProbe()
            val path = root.canonicalPath.toByteArray(Charsets.UTF_8)
            assertEquals(1, bridge.nativeStorageVersion(path))
            assertTrue(File(root, "catalog.sqlite").isFile)
            assertEquals(1, bridge.nativeStorageVersion(path))
            assertEquals(-1, bridge.nativeStorageVersion("../escape".toByteArray()))
        } finally {
            root.deleteRecursively()
        }
    }
}
