package ai.pixaura.app
import ai.pixaura.bridge.CoreProbe
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.util.UUID
import org.junit.Assert.assertTrue
import org.junit.Test
class AssetPublicationTest {
    @Test fun actualAndroidRenameNoReplaceAndImmutableFailureCorpus() {
        val root = File(InstrumentationRegistry.getInstrumentation().targetContext.noBackupFilesDir, "publication-${UUID.randomUUID()}")
        assertTrue(root.mkdir())
        try {
            val checks = CoreProbe().nativeAndroidAssetPublication(root.canonicalPath.toByteArray())
            assertTrue("ADR0021 actual syscall/failure corpus failed: $checks", checks >= 50)
            println("ADR0021 Android RENAME_NOREPLACE actual branch: $checks checks PASS")
        } finally { root.walkBottomUp().forEach { it.setWritable(true); it.delete() } }
    }
}
