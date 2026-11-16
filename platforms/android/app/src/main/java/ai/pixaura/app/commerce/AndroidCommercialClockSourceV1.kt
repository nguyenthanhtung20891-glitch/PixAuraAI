package ai.pixaura.app.commerce

import android.content.Context
import android.os.SystemClock
import android.provider.Settings
import java.util.Locale

// OS observations only, local and never serialized to store/network requests.
// No trusted-time flag; native ledger owns all entitlement/quota/time decisions.
data class CommercialClockObservationV1(val utcSeconds: Long, val uptimeSeconds: Long, val bootToken: String)
class AndroidCommercialClockSourceV1(context: Context) {
    private val bootToken: String
    init {
        val count = Settings.Global.getInt(context.applicationContext.contentResolver, Settings.Global.BOOT_COUNT)
        check(count >= 0)
        bootToken = String.format(Locale.ROOT, "%032x", count)
    }
    fun sample(): CommercialClockObservationV1 {
        val utc = Math.floorDiv(System.currentTimeMillis(), 1000L)
        val uptime = SystemClock.elapsedRealtime() / 1000L
        check(utc >= 0 && uptime >= 0)
        return CommercialClockObservationV1(utc, uptime, bootToken)
    }
}
