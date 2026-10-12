package ai.pixaura.app

import ai.pixaura.bridge.AndroidCapabilitySnapshot
import ai.pixaura.bridge.AndroidSignalMapping
import ai.pixaura.bridge.AndroidStaticFacts
import org.junit.Assert.*
import org.junit.Test
import java.util.concurrent.CountDownLatch
import java.util.concurrent.atomic.AtomicInteger

class AndroidCapabilityOwnerTest {
    private class Signals : CapabilitySignals {
        var heat = 1; var pressure = 1; var backend = 2
        var registrations = 0; var removals = 0
        var heatCallback: (Int) -> Unit = {}
        var trimCallback: (Int) -> Unit = {}
        override fun facts() = AndroidStaticFacts(8, 36, 8L * 1073741824, 512L * 1048576, backend)
        override fun thermal() = heat
        override fun memory() = pressure
        override fun budget() = 512L * 1048576
        override fun availability() = backend
        override fun listen(thermal: (Int) -> Unit, trim: (Int) -> Unit): AutoCloseable {
            registrations++; heatCallback = thermal; trimCallback = trim
            return AutoCloseable { removals++ }
        }
    }
    @Test fun exactNormalizedMappingsAndOverflowSafeBudget() {
        assertEquals(listOf(1, 2, 2, 3, 4, 4, 4), (0..6).map { AndroidSignalMapping.thermal(it) })
        for (value in listOf(-1, 7, Int.MAX_VALUE)) assertEquals(0, AndroidSignalMapping.thermal(value))
        assertEquals(0, AndroidSignalMapping.thermal(0, false))
        assertEquals(listOf(2, 3, 4, 0, 4, 4, 4), listOf(5, 10, 15, 20, 40, 60, 80).map { AndroidSignalMapping.trim(it) })
        assertEquals(0, AndroidSignalMapping.trim(-1)); assertEquals(0, AndroidSignalMapping.trim(81))
        assertEquals(0, AndroidSignalMapping.memory(null, 100, 50))
        assertEquals(0, AndroidSignalMapping.memory(false, -1, 50))
        assertEquals(0, AndroidSignalMapping.memory(false, 100, 0))
        assertEquals(3, AndroidSignalMapping.memory(true, 100, 50))
        assertEquals(3, AndroidSignalMapping.memory(false, 49, 50))
        assertEquals(1, AndroidSignalMapping.memory(false, 50, 50))
        assertEquals(0L, AndroidSignalMapping.processBudget(-1, Long.MAX_VALUE, Long.MAX_VALUE))
        assertEquals(0L, AndroidSignalMapping.processBudget(256, -1, 0))
        assertEquals(512L, AndroidSignalMapping.processBudget(Int.MAX_VALUE, Long.MAX_VALUE, 512))
        assertEquals(Int.MAX_VALUE.toLong() * 1048576, AndroidSignalMapping.processBudget(Int.MAX_VALUE, Long.MAX_VALUE, 0))
    }
    @Test fun singleListenerRecreationStaleCallbackAndFreshProcessState() {
        val signals = Signals(); val owner = AndroidCapabilityOwner(signals)
        owner.foreground(true)
        val received = mutableListOf<AndroidCapabilitySnapshot>()
        val a = owner.subscribe { received.add(it) }; val b = owner.subscribe { received.add(it) }
        assertEquals(1, signals.registrations)
        val facts = received.last().facts
        signals.heatCallback(3); assertEquals(3, received.last().observation.thermal)
        signals.trimCallback(15); assertEquals(4, received.last().observation.memory)
        owner.foreground(false); assertTrue(received.last().observation.constrained)
        owner.foreground(true); assertFalse(received.last().observation.constrained)
        signals.backend = 1; assertEquals(1, owner.sample().observation.vulkan)
        assertEquals(facts, owner.sample().facts)
        a.close(); assertEquals(0, signals.removals)
        val stale = signals.heatCallback; b.close(); assertEquals(1, signals.removals)
        val size = received.size; stale(4); assertEquals(size, received.size)
        val recreated = owner.subscribe { received.add(it) }; assertEquals(2, signals.registrations)
        val after = received.size; stale(4); assertEquals(after, received.size)
        recreated.close(); owner.close(); assertEquals(2, signals.removals)
        val fresh = AndroidCapabilityOwner(Signals()); assertEquals(1L, fresh.sample().observation.sequence)
        assertTrue(fresh.sample().observation.constrained); fresh.close()
    }
    @Test fun concurrentCallbacksAndBoundedChurnNoDuplicateSubscription() {
        val signals = Signals(); val owner = AndroidCapabilityOwner(signals); val count = AtomicInteger()
        var sequence = 0L
        val subscription = owner.subscribe { snapshot ->
            assertTrue(snapshot.observation.sequence >= sequence); sequence = snapshot.observation.sequence; count.incrementAndGet()
        }
        val start = CountDownLatch(1)
        val threads = (0..3).map { lane -> Thread {
            start.await(); repeat(2500) { if (lane % 2 == 0) signals.heatCallback(it % 5) else signals.trimCallback(15) }
        }.also { it.start() } }
        start.countDown(); threads.forEach { it.join() }
        assertTrue(count.get() >= 10000); assertEquals(1, signals.registrations)
        subscription.close(); val before = count.get(); signals.heatCallback(1)
        assertEquals(before, count.get()); assertEquals(1, signals.removals); owner.close()
    }
}
