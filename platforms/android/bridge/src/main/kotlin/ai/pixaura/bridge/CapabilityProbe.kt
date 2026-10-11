package ai.pixaura.bridge

// Step 1 transport seam. Platform probes/signals and policy arrive only after
// roadmap freeze and later authorization. No UI or document owns this data.
interface CapabilityProbe {
    fun snapshot(): CapabilitySnapshot?
}

class CapabilitySnapshot internal constructor(bytes: ByteArray) {
    private val diagnostic = bytes.copyOf()
    fun diagnosticBytes(): ByteArray = diagnostic.copyOf()
}

class ConservativeCapabilityProbe : CapabilityProbe {
    override fun snapshot(): CapabilitySnapshot? = CoreProbe().nativeCapabilityBaseline()?.let { CapabilitySnapshot(it) }
}
