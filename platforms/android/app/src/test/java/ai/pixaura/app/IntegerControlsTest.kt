package ai.pixaura.app

import org.junit.Assert.assertEquals
import org.junit.Test

class IntegerControlsTest {
    @Test fun displayPositionsMapToExactIntegerLatticeIncludingNegativeHalfway() {
        assertEquals(0, integerPosition(-0.5f, -5000, 1))
        assertEquals(-1, integerPosition(-0.51f, -5000, 1))
        assertEquals(1000, integerPosition(1000.49f, -5000, 1))
        assertEquals(1001, integerPosition(1000.5f, -5000, 1))
        assertEquals(4000, integerPosition(4000f, 4000, 1))
        assertEquals(25000, integerPosition(25000f, 4000, 1))
        assertEquals(15, integerPosition(12.5f, 0, 5))
        assertEquals("1.0 EV", displayValue(1000, "thousandths_exposure_stop"))
        assertEquals("6504 K", displayValue(6504, "daylight_white_kelvin"))
        assertEquals("125.0%", displayValue(1250, "thousandths_chroma_multiplier"))
    }
}
