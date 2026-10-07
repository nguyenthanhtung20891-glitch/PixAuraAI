package ai.pixaura.bridge

import android.graphics.Bitmap

// Harmless, versioned boundary probe. No photo processing in Kotlin.
class CoreProbe {
    companion object {
        init {
            System.loadLibrary("pixaura_core")
            System.loadLibrary("pixaura_bridge")
        }
    }

    external fun nativeAbiVersion(): Int

    private external fun nativePreviewPixels(
        privateRoot: ByteArray, digest: ByteArray, assetBytes: Long,
        contextIdentity: ByteArray, maxWidth: Int, maxHeight: Int,
    ): IntArray?

    // One-shot background consumer. Native RGBA is explicitly packed to ARGB
    // words; Bitmap's documented IntArray API avoids endian-dependent buffers.
    fun referenceBitmap(
        privateRoot: ByteArray, digest: ByteArray, assetBytes: Long,
        contextIdentity: ByteArray, maxWidth: Int, maxHeight: Int,
    ): Bitmap? {
        require(maxWidth in 1..1024 && maxHeight in 1..1024)
        val pixels = nativePreviewPixels(privateRoot, digest, assetBytes, contextIdentity, maxWidth, maxHeight)
            ?: return null
        val width = pixels[0]
        val height = pixels[1]
        check(width in 1..maxWidth && height in 1..maxHeight && pixels.size == 2 + width * height)
        return Bitmap.createBitmap(pixels, 2, width, width, height, Bitmap.Config.ARGB_8888)
    }

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
