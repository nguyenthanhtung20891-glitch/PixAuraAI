package ai.pixaura.app

import android.app.ActivityManager
import android.app.Application
import android.app.Activity
import android.content.ComponentCallbacks2
import android.content.res.Configuration
import android.os.Build
import android.os.PowerManager
import android.os.Bundle
import ai.pixaura.bridge.AndroidCapabilitySnapshot
import ai.pixaura.bridge.AndroidObservation
import ai.pixaura.bridge.AndroidSignalMapping
import ai.pixaura.bridge.AndroidStaticFacts
import ai.pixaura.bridge.CoreProbe

/** One application owner; only application context retained. No tier/policy table. */
internal interface CapabilitySignals {
    fun facts(): AndroidStaticFacts
    fun thermal(): Int
    fun memory(): Int
    fun budget(): Long
    fun availability(): Int
    fun listen(thermal: (Int) -> Unit, trim: (Int) -> Unit): AutoCloseable
}
private class SystemCapabilitySignals(private val app: Application) : CapabilitySignals {
    private val manager = app.getSystemService(ActivityManager::class.java)
    private val power = app.getSystemService(PowerManager::class.java)
    private fun memoryInfo(): ActivityManager.MemoryInfo? = try {
        manager?.let { service -> ActivityManager.MemoryInfo().also { service.getMemoryInfo(it) } }
    } catch (_: RuntimeException) { null }
    override fun budget(): Long = AndroidSignalMapping.processBudget(
        try { manager?.memoryClass ?: 0 } catch (_: RuntimeException) { 0 }, Runtime.getRuntime().maxMemory(), memoryInfo()?.totalMem ?: 0)
    override fun facts(): AndroidStaticFacts = AndroidStaticFacts(
        Runtime.getRuntime().availableProcessors().takeIf { it in 1..1024 } ?: 0, Build.VERSION.SDK_INT,
        memoryInfo()?.totalMem?.takeIf { it > 0 } ?: 0, budget(),
        availability())
    override fun availability(): Int = try { CoreProbe().nativeVulkanAvailability() } catch (_: LinkageError) { 0 }
    override fun thermal(): Int = if (Build.VERSION.SDK_INT >= 29) {
        try { power?.currentThermalStatus?.let { AndroidSignalMapping.thermal(it) } ?: 0 }
        catch (_: RuntimeException) { 0 }
    } else 0
    override fun memory(): Int = memoryInfo()?.let { AndroidSignalMapping.memory(it.lowMemory, it.availMem, it.threshold) } ?: 0
    @Suppress("DEPRECATION", "OVERRIDE_DEPRECATION") // Older supported OS callbacks; modern OS can omit them.
    override fun listen(thermal: (Int) -> Unit, trim: (Int) -> Unit): AutoCloseable {
        val callbacks = object : ComponentCallbacks2 {
            override fun onConfigurationChanged(newConfig: Configuration) = Unit
            override fun onLowMemory() = trim(15)
            override fun onTrimMemory(level: Int) = trim(level)
        }
        app.registerComponentCallbacks(callbacks)
        var registered = false
        val listener = if (Build.VERSION.SDK_INT >= 29) PowerManager.OnThermalStatusChangedListener { thermal(AndroidSignalMapping.thermal(it)) } else null
        if (Build.VERSION.SDK_INT >= 29 && listener != null && power != null) {
            try { power.addThermalStatusListener(listener); registered = true }
            catch (_: RuntimeException) { thermal(0) }
        }
        return AutoCloseable {
            app.unregisterComponentCallbacks(callbacks)
            if (Build.VERSION.SDK_INT >= 29 && registered && listener != null) power?.removeThermalStatusListener(listener)
        }
    }
}

internal class AndroidCapabilityOwner(private val signals: CapabilitySignals) : AutoCloseable {
    private var facts: AndroidStaticFacts? = null
    private var thermal = 0
    private var memory = 0
    private var foreground = false
    private var backend = 0
    private var sequence = 0L
    private var last: AndroidCapabilitySnapshot? = null
    private var registration: AutoCloseable? = null
    private var generation: Any? = null
    private var closed = false
    private val observers = LinkedHashMap<Any, (AndroidCapabilitySnapshot) -> Unit>()
    @Synchronized internal fun subscriptionCount(): Int = observers.size
    /** Trusted normalized callback entry seams; no exported component/deserializer. */
    @Synchronized internal fun observeThermal(value: Int) {
        if (closed || observers.isEmpty()) return
        thermal = value.takeIf { it in 0..4 } ?: 0; publish()
    }
    @Synchronized internal fun observeTrim(level: Int) {
        if (closed || observers.isEmpty()) return
        if (level == 20) foreground = false else memory = AndroidSignalMapping.trim(level)
        publish()
    }
    @Synchronized fun foreground(value: Boolean) {
        if (closed) return
        foreground = value
        if (facts != null) publish()
    }
    private fun initialize() {
        if (facts == null) {
            facts = signals.facts(); backend = requireNotNull(facts).vulkan
            thermal = signals.thermal(); memory = signals.memory()
        }
    }
    private fun publish(): AndroidCapabilitySnapshot {
        check(!closed && sequence < Long.MAX_VALUE)
        initialize()
        val observation = AndroidObservation(thermal, memory, !foreground, backend, signals.budget(), ++sequence)
        val snapshot = AndroidCapabilitySnapshot(requireNotNull(facts), observation)
        last = snapshot
        observers.values.toList().forEach { it(snapshot) }
        return snapshot
    }
    /** Fresh OS samples are recovery evidence; no timer or fabricated stability. */
    @Synchronized fun sample(): AndroidCapabilitySnapshot {
        check(!closed); initialize(); thermal = signals.thermal(); memory = signals.memory(); backend = signals.availability()
        return publish()
    }
    @Synchronized fun subscribe(observer: (AndroidCapabilitySnapshot) -> Unit): AutoCloseable {
        check(!closed && observers.size < 8)
        initialize()
        val key = Any(); observers[key] = observer
        if (registration == null) {
            val token = Any(); generation = token
            registration = signals.listen(
                { value -> synchronized(this) { if (!closed && generation === token) observeThermal(value) } },
                { level -> synchronized(this) { if (!closed && generation === token) {
                    observeTrim(level)
                } } })
        }
        observer(last ?: publish())
        return AutoCloseable { synchronized(this) {
            observers.remove(key)
            if (observers.isEmpty()) { generation = null; registration?.close(); registration = null }
        } }
    }
    @Synchronized override fun close() {
        if (!closed) { closed = true; generation = null; observers.clear(); registration?.close(); registration = null }
    }
}

class PixAuraApplication : Application() {
    internal lateinit var capabilityOwner: AndroidCapabilityOwner
    private var startedActivities = 0
    private val lifecycle = object : ActivityLifecycleCallbacks {
        override fun onActivityStarted(activity: Activity) {
            if (startedActivities == Int.MAX_VALUE) { capabilityOwner.foreground(false); return }
            startedActivities++; capabilityOwner.foreground(true)
        }
        override fun onActivityStopped(activity: Activity) {
            if (startedActivities > 0) startedActivities--
            if (startedActivities == 0 && !activity.isChangingConfigurations) capabilityOwner.foreground(false)
        }
        override fun onActivityCreated(activity: Activity, savedInstanceState: Bundle?) = Unit
        override fun onActivityResumed(activity: Activity) = Unit
        override fun onActivityPaused(activity: Activity) = Unit
        override fun onActivitySaveInstanceState(activity: Activity, outState: Bundle) = Unit
        override fun onActivityDestroyed(activity: Activity) = Unit
    }
    override fun onCreate() {
        super.onCreate(); capabilityOwner = AndroidCapabilityOwner(SystemCapabilitySignals(this))
        registerActivityLifecycleCallbacks(lifecycle)
    }
    override fun onTerminate() {
        unregisterActivityLifecycleCallbacks(lifecycle); capabilityOwner.close(); super.onTerminate()
    }
}
