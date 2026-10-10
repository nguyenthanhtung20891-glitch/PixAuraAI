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
            assertEquals(0, bridge.nativeStorageMigrate(path, 1, 2))
            assertEquals(2, bridge.nativeStorageVersion(path))
            assertEquals(4, bridge.nativeStorageMigrate(path, 1, 2))
            assertEquals(2, bridge.nativeStorageVersion(path))
            assertEquals(0, bridge.nativeStorageMigrate(path, 2, 2))
            assertEquals(0, bridge.nativeStorageMigrate(path, 2, 3))
            assertEquals(3, bridge.nativeStorageVersion(path))
            assertEquals(4, bridge.nativeStorageMigrate(path, 2, 3))
            assertEquals(0, bridge.nativeStorageMigrate(path, 3, 3))
            assertEquals(4, bridge.nativeStorageMigrate(path, 3, 4))
        } finally {
            root.deleteRecursively()
        }
    }
}
