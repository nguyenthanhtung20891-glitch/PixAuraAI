package ai.pixaura.app

import ai.pixaura.bridge.CoreProbe
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.security.MessageDigest
import java.util.UUID
import java.util.concurrent.Executors
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class DecodeBoundaryTest {
    @Test fun boundedDecodeUsesSharedNativeOwnership() {
        val instrumentation = InstrumentationRegistry.getInstrumentation()
        val root = File(instrumentation.targetContext.noBackupFilesDir, "decode-${UUID.randomUUID()}")
        assertTrue(root.mkdir())
        val executor = Executors.newSingleThreadExecutor()
        try {
            val fixtures = JSONObject(instrumentation.context.assets.open("decode.json").bufferedReader().use { it.readText() })
            executor.submit {
                val bridge = CoreProbe()
                val directory = File(root, "assets/sha256")
                assertTrue(directory.mkdirs())
                for (name in listOf("png", "jpeg")) {
                    val fixture = fixtures.getJSONObject(name)
                    val array = fixture.getJSONArray("bytes")
                    val bytes = ByteArray(array.length()) { array.getInt(it).toByte() }
                    val digest = fixture.getString("sha256")
                    File(directory, digest).writeBytes(bytes)
                    fun call(hash: String, limit: Long, size: Long = bytes.size.toLong()): Int = bridge.nativeDecodeCheck(
                        root.canonicalPath.toByteArray(), hash.toByteArray(), size,
                        UUID.randomUUID().toString().replace("-", "").toByteArray(), limit,
                    )
                    assertEquals(2, call(digest, 64L * 1024 * 1024))
                    val bitmap = bridge.referenceBitmap(root.canonicalPath.toByteArray(), digest.toByteArray(), bytes.size.toLong(), UUID.randomUUID().toString().replace("-", "").toByteArray(), 1, 2)
                    checkNotNull(bitmap)
                    assertEquals(1, bitmap.width)
                    assertEquals(1, bitmap.height)
                    if (name == "png") assertEquals(0xffbcbcff.toInt(), bitmap.getPixel(0, 0))
                    bitmap.recycle()
                    if (name == "png") {
                        val exact = bridge.referenceBitmap(root.canonicalPath.toByteArray(), digest.toByteArray(), bytes.size.toLong(), UUID.randomUUID().toString().replace("-", "").toByteArray(), 2, 3)
                        checkNotNull(exact)
                        assertEquals(0xffff0000.toInt(), exact.getPixel(0, 0))
                        assertEquals(0xff00ff00.toInt(), exact.getPixel(1, 0))
                        assertEquals(0xff0000ff.toInt(), exact.getPixel(0, 1))
                        assertEquals(0xffffffff.toInt(), exact.getPixel(1, 1))
                        assertEquals(128, android.graphics.Color.alpha(exact.getPixel(1, 2)))
                        exact.recycle()
                    }
                    assertEquals(-8, call(digest, bytes.size.toLong() - 1))
                    val bad = byteArrayOf(1, 2, 3)
                    val badHash = MessageDigest.getInstance("SHA-256").digest(bad).joinToString("") { "%02x".format(it) }
                    File(directory, badHash).writeBytes(bad)
                    assertEquals(-17, call(badHash, 64L * 1024 * 1024, bad.size.toLong()))
                    assertEquals(2, call(digest, 64L * 1024 * 1024))
                }
            }.get()
        } finally {
            executor.shutdown()
            root.deleteRecursively()
        }
    }
}
