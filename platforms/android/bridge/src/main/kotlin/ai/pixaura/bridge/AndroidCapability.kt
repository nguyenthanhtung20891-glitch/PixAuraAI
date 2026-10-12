package ai.pixaura.bridge

/** Internal trusted adapter records, never a document or diagnostic deserializer. */
internal data class AndroidStaticFacts(val cpus: Int, val api: Int, val physicalBytes: Long,
    val processBudgetBytes: Long, val vulkan: Int) {
    fun nativeWords() = longArrayOf(1, cpus.toLong(), api.toLong(), physicalBytes, processBudgetBytes, vulkan.toLong())
}
internal data class AndroidObservation(val thermal: Int, val memory: Int, val constrained: Boolean,
    val vulkan: Int, val processBudgetBytes: Long, val sequence: Long) {
    fun nativeWords() = longArrayOf(1, thermal.toLong(), memory.toLong(), if (constrained) 1 else 0,
        vulkan.toLong(), processBudgetBytes, sequence)
}
internal data class AndroidCapabilitySnapshot(val facts: AndroidStaticFacts, val observation: AndroidObservation)
internal data class ExecutionStrategy(val tier: Int, val cap: Long, val edge: Int, val concurrency: Int,
    val backend: Int, val forcedCpu: Boolean, val reason: Int, val recoveryPending: Boolean,
    val stable: Int, val epoch: Long) {
    companion object {
        fun fromNative(words: LongArray): ExecutionStrategy {
            require(words.size == 11)
            if (words[0] != 0L) throw EditorFailure(words[0].toInt(), "Capability")
            return ExecutionStrategy(words[1].toInt(), words[2], words[3].toInt(), words[4].toInt(),
                words[5].toInt(), words[6] != 0L, words[7].toInt(), words[8] != 0L, words[9].toInt(), words[10])
        }
    }
}

/** API translation only: no tier, budget, preview or backend trust decisions. */
internal object AndroidSignalMapping {
    fun thermal(status: Int, supported: Boolean = true): Int = if (!supported) 0 else when (status) {
        0 -> 1
        1, 2 -> 2
        3 -> 3
        4, 5, 6 -> 4
        else -> 0
    }
    // UI_HIDDEN is lifecycle, not evidence of critical memory merely because 20>15.
    fun trim(level: Int): Int = when (level) {
        5 -> 2 // RUNNING_MODERATE
        10 -> 3 // RUNNING_LOW
        15, 40, 60, 80 -> 4 // RUNNING_CRITICAL and background pressure levels
        20 -> 0 // UI_HIDDEN: keep memory observation; caller sets constrained
        else -> 0
    }
    fun memory(low: Boolean?, available: Long, threshold: Long): Int = when {
        low == null || available < 0 || threshold <= 0 -> 0
        low || available < threshold -> 3
        else -> 1
    }
    fun processBudget(memoryClassMiB: Int, managedMax: Long, physical: Long): Long {
        if (memoryClassMiB <= 0 || managedMax <= 0) return 0
        val observed = minOf(memoryClassMiB.toLong() * 1048576L, managedMax)
        return if (physical > 0) minOf(observed, physical) else observed
    }
}
