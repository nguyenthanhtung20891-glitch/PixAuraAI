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

    // Synchronous SQLite capability check; application services call on a worker.
    external fun nativeStorageVersion(privateRoot: ByteArray): Int

    // Synchronous background-only check. Core owns admission and allocation policy.
    external fun nativeDecodeCheck(
        privateRoot: ByteArray,
        digest: ByteArray,
        assetBytes: Long,
        contextIdentity: ByteArray,
        encodedLimit: Long,
    ): Int

    // Bounded metadata adapter; call from a worker. No image bytes or algorithms.
    external fun nativeDocumentRoundTrip(
        manifest: ByteArray,
        contextIdentity: ByteArray,
        sessionIdentity: ByteArray,
    ): ByteArray?
}
