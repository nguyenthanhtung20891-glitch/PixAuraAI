package ai.pixaura.app

import android.content.Context
import android.system.Os
import ai.pixaura.app.commerce.NativeEditorInstallationKeyV2
import ai.pixaura.bridge.AndroidCapabilitySnapshot
import ai.pixaura.bridge.EditorFailure
import ai.pixaura.bridge.EditorRuntimeV2
import ai.pixaura.bridge.StoreVerifierV3
import ai.pixaura.bridge.configureStoreV3
import java.io.File
import java.nio.file.Files
import java.util.concurrent.atomic.AtomicBoolean

/** Application-owned resource pool, not an editing or paid authority. One strong
 * runtime survives detached UI frames; idle cleanup closes it only after every
 * native context/frame retires. The shared native registry enforces ownership. */
internal class NativeEditorRuntimePoolV2(context: Context,
    private val root: File = File(context.noBackupFilesDir, "native-commercial-v2"),
    keyNamespace: String = "native-receipt-v2", private val storeVerifier: StoreVerifierV3? = null) : AutoCloseable {
    init { require(root.canonicalPath.startsWith(context.noBackupFilesDir.canonicalPath + File.separator)) }
    private val gate = Any()
    private val keys = NativeEditorInstallationKeyV2(context, keyNamespace)
    private var runtime: EditorRuntimeV2? = null
    private var leases = 0
    private var closed = false
    class Lease internal constructor(private val pool: NativeEditorRuntimePoolV2, val runtime: EditorRuntimeV2) : AutoCloseable {
        private val retired = AtomicBoolean(false)
        override fun close() { if (retired.compareAndSet(false, true)) pool.release(runtime) }
    }
    private fun create(snapshot: AndroidCapabilitySnapshot): EditorRuntimeV2 {
        require(!Files.isSymbolicLink(root.toPath()))
        check(root.isDirectory || root.mkdirs()); Os.chmod(root.path, 448)
        val marker = File(root, "installation-v2")
        return try {
            require(!Files.isSymbolicLink(marker.toPath()))
            if (marker.exists()) require(marker.isFile && marker.length() == 0L)
            val key = keys.readOrCreate(allowCreate = !marker.exists() && !File(root, "commercial-v2.sqlite").exists())
            try {
                if (!marker.exists() && !marker.createNewFile()) require(marker.isFile && marker.length() == 0L && !Files.isSymbolicLink(marker.toPath()))
                Os.chmod(marker.path, 384)
                EditorRuntimeV2(root, key, snapshot).also { owner ->
                    // Optional store verification failure must not become key-loss
                    // recovery or disable an otherwise healthy Free/trial editor.
                    if(storeVerifier!=null)try{owner.configureStoreV3(storeVerifier)}catch(_:Exception){/* Store path stays fail-closed. */}
                }
            } finally { key.fill(0) }
        } catch (_: Exception) {
            // Never replace a lost/unreadable committed key or manufacture an
            // entitlement/trial. Explicit native recovery grants no NEW editing.
            EditorRuntimeV2.recovery(root, snapshot)
        }
    }
    fun acquire(snapshot: AndroidCapabilitySnapshot): Lease = synchronized(gate) {
        check(!closed); if (leases >= 8) throw EditorFailure(8, "Editor owners")
        var owner = runtime
        if (owner == null) {
            owner = create(snapshot)
            val retained = owner
            val weak = java.lang.ref.WeakReference(this)
            owner.observeFrameRetirement { weak.get()?.idle(retained) }
            runtime = owner
        } else { owner.updateCapability(snapshot.observation) }
        val lease = Lease(this, owner); leases++; lease
    }
    private fun release(owner: EditorRuntimeV2) = synchronized(gate) {
        check(runtime === owner && leases > 0); leases--; idle(owner)
    }
    private fun idle(owner: EditorRuntimeV2) = synchronized(gate) {
        if (runtime !== owner || leases != 0) return@synchronized
        try { owner.close(); runtime = null }
        catch (failure: EditorFailure) { if (failure.status != 15) throw failure }
    }
    override fun close() = synchronized(gate) {
        if (leases != 0) throw EditorFailure(15, "Editor owners")
        runtime?.let { it.close() }; runtime = null; closed = true
    }
}
