package ai.pixaura.app

import ai.pixaura.bridge.ConservativeCapabilityProbe
import ai.pixaura.bridge.CoreProbe
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.Assert.assertEquals
import org.junit.Assert.assertArrayEquals
import org.junit.Test
import org.junit.runner.RunWith
import java.nio.ByteBuffer

@RunWith(AndroidJUnit4::class)
class CapabilityFoundationTest {
    @Test fun sharedVerifiedOnnxVulkanProviderCorpus() {
        assertEquals(0, CoreProbe().nativeAndroidInferenceBoundary())
    }
    @Test fun sharedVerifiedOnnxCpuReferenceCorpus() {
        assertEquals(0, CoreProbe().nativeInferenceBoundary())
    }
    @Test fun sharedSignedModelPackageAndIndependentEd25519Corpus() {
        assertEquals(0, CoreProbe().nativeModelPackageBoundary())
    }
    @Test fun sharedDynamicPressureRecoveryAndBackendCorpus() {
        assertEquals(0, CoreProbe().nativeDynamicPolicyBoundary())
    }
    @Test fun sharedBudgetThresholdOverflowAndDowngradeCorpus() {
        assertEquals(0, CoreProbe().nativeBudgetBoundary())
    }
    @Test fun sharedSyntheticAndMalformedCorpus() {
        assertEquals(0, CoreProbe().nativeCapabilityFoundationBoundary())
    }
    @Test fun conservativeProbeIsUnknownAndDetached() {
        val snapshot = checkNotNull(ConservativeCapabilityProbe().snapshot())
        val bytes = snapshot.diagnosticBytes()
        val words = ByteBuffer.wrap(bytes)
        assertEquals(76, bytes.size)
        assertEquals(1, words.getInt(0))
        assertEquals(3, words.getInt(4)) // observed native Android platform
        assertEquals(0, words.getInt(12)) // unprobed CPU count
        assertEquals(0, words.getInt(28)) // not GPU validated
        assertEquals(0L, words.getLong(36)) // unknown RAM
        assertEquals(1, words.getInt(52)) // conservative constrained state
        assertEquals(0L, words.getLong(60)) // no process allowance established
        val saved = bytes.copyOf()
        bytes.fill(127)
        assertArrayEquals(saved, snapshot.diagnosticBytes())
    }
}
