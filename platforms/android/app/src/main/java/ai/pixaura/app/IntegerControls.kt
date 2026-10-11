package ai.pixaura.app

import kotlin.math.floor

/** Presentation lattice only. Core validates; no clamping or persisted float.
 * Nearest descriptor lattice point, halfway toward the greater integer.
 */
fun integerPosition(position: Float, minimum: Int, step: Int): Int =
    minimum + floor((position.toDouble() - minimum) / step + 0.5).toInt() * step

fun displayValue(value: Int, unit: String): String = when {
    unit == "clockwise_quarter_turns" -> "${value * 90}° clockwise"
    unit == "daylight_white_kelvin" -> "$value K"
    unit.contains("exposure_stop") -> "${value / 1000.0} EV"
    unit.contains("log2_contrast") -> "${value / 1000.0} stops"
    unit.startsWith("thousandths") -> "${value / 10.0}%"
    else -> value.toString()
}
