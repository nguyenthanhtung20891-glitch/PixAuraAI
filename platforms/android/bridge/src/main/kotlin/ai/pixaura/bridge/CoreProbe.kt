package ai.pixaura.bridge

// Harmless, versioned boundary probe. No photo processing in Kotlin.
class CoreProbe {
    companion object {
        init {
            System.loadLibrary("pixaura_core")
            System.loadLibrary("pixaura_bridge")
        }
    }

    external fun nativeAbiVersion(): Int
}
