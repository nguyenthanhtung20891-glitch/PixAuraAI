package ai.pixaura.bridge

// Boundary probe only. Phase 1 packages the libraries in a real Android app.
class CoreProbe {
    companion object {
        init {
            System.loadLibrary("pixaura_core")
            System.loadLibrary("pixaura_bridge")
        }
    }

    external fun nativeAbiVersion(): Int
}
