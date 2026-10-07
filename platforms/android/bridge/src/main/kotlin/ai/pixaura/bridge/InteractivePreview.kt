package ai.pixaura.bridge

import android.graphics.Bitmap
import java.nio.ByteBuffer
import java.util.concurrent.locks.ReentrantLock
import kotlin.concurrent.withLock

/** Bounded synchronous adapter. Run initialization/render/close off the UI thread.
 * All lifecycle changes and display installation pass through this owner.
 * The direct buffer is private caller-owned native context storage, never copied.
 */
class InteractivePreview(
    privateRoot: ByteArray, digest: ByteArray, assetBytes: Long, identity: ByteArray,
) : AutoCloseable {
    companion object { init { System.loadLibrary("pixaura_core"); System.loadLibrary("pixaura_bridge") } }
    class Ticket internal constructor(internal val contextId: String, val generation: Long)
    class Candidate internal constructor(val ticket: Ticket, val bitmap: Bitmap)
    private val contextId = identity.toString(Charsets.US_ASCII)
    private val storage = ByteBuffer.allocateDirect(nativeStorageSize())
    private val gate = ReentrantLock()
    private val idle = gate.newCondition()
    private var active = 0
    private var closed = false
    private var requestedGenerationValue = 0L
    private var displayed: Candidate? = null
    init { check(nativeInit(storage, privateRoot, digest, assetBytes, contextId.toByteArray(Charsets.US_ASCII)) == 0) }
    fun begin(): Ticket = gate.withLock {
        check(!closed)
        val generation = nativeBegin(storage)
        check(generation != 0L)
        requestedGenerationValue = generation
        Ticket(contextId, generation)
    }
    fun cancel(ticket: Ticket): Boolean = gate.withLock {
        !closed && ticket.contextId == contextId && nativeCancel(storage, ticket.generation) == 0
    }
    fun render(ticket: Ticket, maxWidth: Int, maxHeight: Int): Candidate? {
        require(maxWidth in 1..1024 && maxHeight in 1..1024)
        gate.withLock { if (closed || ticket.contextId != contextId || active >= 2) return null; active++ }
        try {
            val pixels = nativeRender(storage, ticket.generation, maxWidth, maxHeight) ?: return null
            val width = pixels[0]; val height = pixels[1]
            check(width in 1..maxWidth && height in 1..maxHeight && pixels.size == 2 + width * height)
            return Candidate(ticket, Bitmap.createBitmap(pixels, 2, width, width, height, Bitmap.Config.ARGB_8888))
        } finally { gate.withLock { active--; idle.signalAll() } }
    }
    /** Gate covers both native eligibility and replacement; supersession cannot enter between them. */
    fun install(candidate: Candidate): Boolean = gate.withLock {
        if (closed || candidate.ticket.contextId != contextId || nativeCurrent(storage, candidate.ticket.generation) != 0) false
        else { displayed = candidate; true }
    }
    fun requestedGeneration(): Long? = gate.withLock { requestedGenerationValue.takeIf { it != 0L } }
    fun displayedGeneration(): Long? = gate.withLock { displayed?.ticket?.generation }
    fun displayedBitmap(): Bitmap? = gate.withLock { displayed?.bitmap }
    override fun close() = gate.withLock {
        if (!closed) {
            closed = true
            check(nativeStop(storage) == 0)
            while (active != 0) idle.awaitUninterruptibly()
            check(nativeDestroy(storage) == 0)
        }
        // Detached copies remain usable by callers; no eager Bitmap.recycle().
    }
    private external fun nativeStorageSize(): Int
    private external fun nativeInit(storage: ByteBuffer, root: ByteArray, digest: ByteArray, bytes: Long, identity: ByteArray): Int
    private external fun nativeBegin(storage: ByteBuffer): Long
    private external fun nativeCancel(storage: ByteBuffer, generation: Long): Int
    private external fun nativeCurrent(storage: ByteBuffer, generation: Long): Int
    private external fun nativeRender(storage: ByteBuffer, generation: Long, maxWidth: Int, maxHeight: Int): IntArray?
    private external fun nativeStop(storage: ByteBuffer): Int
    private external fun nativeDestroy(storage: ByteBuffer): Int
}
