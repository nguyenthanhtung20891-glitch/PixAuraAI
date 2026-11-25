package ai.pixaura.bridge

import android.graphics.Bitmap
import java.lang.ref.PhantomReference
import java.lang.ref.ReferenceQueue
import java.util.concurrent.atomic.AtomicBoolean

/** Resource retirement only: no editing callbacks or publication authority.
 * UI Bitmaps may survive StateFlow coalescing, composition and render-thread
 * references. Never recycle them while the platform still owns references.
 * The native consumer reservation remains live until actual Java retirement.
 * At most the existing 64 native frame handles can enter this queue. */
internal object BitmapLifetimeV2 {
    private val gate = Any()
    private val queue = ReferenceQueue<Bitmap>()
    private val pending = mutableSetOf<Record>()
    private var worker: Thread? = null
    private var failure: Throwable? = null
    private class Record(bitmap: Bitmap, private val runtime: EditorRuntimeV2,
        private val editor: Long, private val frame: Long, private val retired: () -> Unit) : PhantomReference<Bitmap>(bitmap, queue) {
        private val released = AtomicBoolean(false)
        fun release() {
            if (!released.compareAndSet(false, true)) return
            // Keep the Java runtime owner alive across release of its last frame.
            synchronized(runtime) {
                val status = NativeEditorV2.frame(editor, frame, 2, null)
                if (status != 0) throw EditorFailure(status, "UI frame retirement")
            }
            clear(); retired(); runtime.frameRetired()
        }
    }
    fun register(bitmap: Bitmap, runtime: EditorRuntimeV2, editor: Long, frame: Long, retired: () -> Unit): Unit = synchronized(gate) {
        failure?.let { throw EditorFailure(12, "UI frame retirement") }
        if (pending.size >= 64) throw EditorFailure(8, "UI frame retirement")
        val record = Record(bitmap, runtime, editor, frame, retired)
        pending.add(record)
        try {
            if (worker == null) {
                val next = Thread({ collect() }, "pixaura-ui-frame-retirement").apply { isDaemon = true }
                worker = next
                try { next.start() } catch (error: Throwable) { worker = null; throw error }
            }
        } catch (error: Throwable) { pending.remove(record); record.clear(); throw error }
    }
    private fun collect() {
        try {
            while (true) {
                val record = queue.remove() as Record
                record.release()
                synchronized(gate) {
                    pending.remove(record)
                    if (pending.isEmpty()) { worker = null; return }
                }
            }
        } catch (error: Throwable) {
            // Do not silently certify cleanup or admit further UI reservations
            // after an unexpected release failure. No retry/approval authority.
            synchronized(gate) { failure = error; worker = null }
        }
    }
    internal fun outstanding(): Int = synchronized(gate) { pending.size }
}
