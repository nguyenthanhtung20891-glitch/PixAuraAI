package ai.pixaura.app

import ai.pixaura.bridge.CapabilityProbe
import ai.pixaura.bridge.CapabilitySnapshot
import org.junit.Assert.assertArrayEquals
import org.junit.Test

class CapabilityProbeTest {
    @Test fun injectedBoundaryOwnsDetachedDiagnostics() {
        val input = byteArrayOf(1, 2, 3)
        val seam = object : CapabilityProbe {
            override fun snapshot() = CapabilitySnapshot(input)
        }
        val snapshot = seam.snapshot()
        input.fill(0)
        val exported = snapshot.diagnosticBytes()
        exported.fill(0)
        assertArrayEquals(byteArrayOf(1, 2, 3), snapshot.diagnosticBytes())
    }
}
