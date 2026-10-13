package ai.pixaura.app

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test
import org.junit.runner.RunWith
import java.io.File

@RunWith(AndroidJUnit4::class)
class CrossTierReplayTest {
    private external fun nativeCertify(root: String): Int

    @Test fun sharedCanonicalReplayAndReferenceCertification() {
        System.loadLibrary("pixaura_bridge")
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val root = File(context.noBackupFilesDir, "cross-tier-certification")
        assertTrue(root.mkdirs() || root.isDirectory)
        assertEquals(0, nativeCertify(root.canonicalPath))
        val matrix = File(root, "cross-tier-matrix.tsv").readText()
        assertTrue(matrix.toByteArray().size <= 32768)
        assertTrue(matrix.contains("SYNTHETIC POLICY EVIDENCE"))
        assertTrue(matrix.contains("PEER CANONICAL LOW"))
        assertTrue(matrix.contains("T0_MANUAL_FLOW_PASS"))
        assertTrue(matrix.contains("LOW_HIGH_PASS"))
        println("ANDROID EMULATOR INTEGRATION, NOT PHYSICAL CERTIFICATION\n$matrix")
    }
}
