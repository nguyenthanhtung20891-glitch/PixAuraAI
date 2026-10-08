package ai.pixaura.bridge

// Dedicated background diagnostic, never a display or document mutation path.
class GpuValidation {
    companion object {
        init {
            System.loadLibrary("pixaura_core")
            System.loadLibrary("pixaura_bridge")
        }
    }
    external fun nativeEvidence(): String
}
