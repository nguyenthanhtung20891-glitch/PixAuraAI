package ai.pixaura.app

import ai.pixaura.bridge.GpuValidation
import android.os.Build
import android.os.ParcelFileDescriptor
import android.util.Base64
import android.util.Log
import androidx.test.platform.app.InstrumentationRegistry
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

private fun shellOutput(command: String): String {
    val descriptor = InstrumentationRegistry.getInstrumentation().uiAutomation.executeShellCommand(command)
    return ParcelFileDescriptor.AutoCloseInputStream(descriptor).use {
        it.readBytes().toString(Charsets.UTF_8)
    }
}

private fun publishEvidence(validationRun: String, json: String) {
    require(validationRun.matches(Regex("[0-9a-f]{32}")))
    val bytes = json.toByteArray(Charsets.UTF_8)
    require(bytes.size <= 16384)
    val encoded = Base64.encodeToString(bytes, Base64.NO_WRAP)
    val path = "/data/local/tmp/pixaura-gpu-$validationRun/publish.sh"
    // executeShellCommand tokenizes arguments: no shell quotes or operators here.
    val command = "sh $path $encoded"
    assertEquals("Evidence transport failed", "PIXAURA_WRITTEN", shellOutput(command))
}

class GpuEvidenceTransportTest {
    @Test fun shellArtifactSurvivesPrivateEvidenceRemovalAndRejectsOverwrite() {
        // Synthetic transport regression only: never a hardware certification.
        val run = requireNotNull(InstrumentationRegistry.getArguments().getString("pixauraTransportRun")) {
            "Run through check-android-emulator.sh or supply a fresh host-prepared transport directory"
        }
        require(run.matches(Regex("[0-9a-f]{32}")))
        val directory = "/data/local/tmp/pixaura-gpu-$run"
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        try {
            val json = "{\"validation_run\":\"$run\",\"status\":\"TRANSPORT_TEST_ONLY\"}"
            context.openFileOutput("gpu-transport-test.json", android.content.Context.MODE_PRIVATE).use {
                it.write(json.toByteArray(Charsets.UTF_8))
            }
            publishEvidence(run, json)
            assertTrue(context.deleteFile("gpu-transport-test.json"))
            assertEquals(json, shellOutput("cat $directory/evidence.json"))
            var rejected = false
            try { publishEvidence(run, "stale replacement") } catch (_: AssertionError) { rejected = true }
            assertTrue("Existing evidence must never be overwritten", rejected)
            assertEquals(json, shellOutput("cat $directory/evidence.json"))
        } finally {
            context.deleteFile("gpu-transport-test.json")
            // Host reads this artifact after UTP uninstall, then removes the directory.
        }
    }
}

class GpuHardwareTest {
    @Test fun physicalVulkanCertification() {
        val args = InstrumentationRegistry.getArguments()
        val controlled = args.getString("pixauraHardware") == "true"
        val validationRun = args.getString("pixauraRun") ?: ""
        val sourceSha = args.getString("pixauraSha") ?: ""
        if (controlled) {
            require(validationRun.matches(Regex("[0-9a-f]{32}")))
            require(sourceSha.matches(Regex("[0-9a-f]{40}")))
        }
        val evidence = JSONObject(GpuValidation().nativeEvidence())
        evidence.put("device_model", "${Build.MANUFACTURER} ${Build.MODEL}")
        evidence.put("os_version", Build.VERSION.RELEASE)
        evidence.put("controlled_hardware_gate", controlled)
        if (controlled) {
            evidence.put("validation_run", validationRun)
            evidence.put("source_sha", sourceSha)
        }
        // Physical provenance is required in addition to the native device probe.
        val emulator = Build.FINGERPRINT.startsWith("generic") ||
            Build.MODEL.contains("Emulator", ignoreCase = true) ||
            Build.HARDWARE in setOf("ranchu", "goldfish")
        if (emulator && evidence.optString("status") == "PASS") {
            evidence.put("status", "UNSUPPORTED")
            evidence.put("reason", "Physical Android device required")
        }
        InstrumentationRegistry.getInstrumentation().targetContext.openFileOutput(
            "gpu-hardware.json", android.content.Context.MODE_PRIVATE,
        ).use { it.write(evidence.toString().toByteArray(Charsets.UTF_8)) }
        if (controlled) {
            // Only synthetic diagnostics leave app storage. The shell-owned artifact
            // survives UTP uninstall; its directory is created by this invocation.
            publishEvidence(validationRun, evidence.toString())
        }
        Log.i("PixAuraGPU", evidence.toString())
        println("PIXAURA_GPU_EVIDENCE ${evidence}")
        println("Vulkan hardware validation: ${evidence.getString("status")} " +
            "${evidence.optInt("parity_passed")}/${evidence.optInt("expected_test_count")} expected parity cases; " +
            "GPU=${evidence.optString("gpu")}")
        if (controlled) {
            assertTrue("Physical device required: $evidence", !emulator)
            assertEquals(evidence.toString(), "PASS", evidence.getString("status"))
            assertTrue(evidence.getBoolean("hardware"))
            assertTrue(evidence.getBoolean("pipeline") && evidence.getBoolean("dispatch"))
            assertEquals(evidence.getInt("test_count"), evidence.getInt("parity_passed"))
            assertEquals(evidence.getInt("expected_test_count"), evidence.getInt("test_count"))
        } else {
            // Hosted SwiftShader is a rejection regression, never certification.
            if (emulator) assertEquals(evidence.toString(), "UNSUPPORTED", evidence.getString("status"))
            assertTrue(evidence.toString(), evidence.getString("status") != "FAIL")
        }
    }
}
