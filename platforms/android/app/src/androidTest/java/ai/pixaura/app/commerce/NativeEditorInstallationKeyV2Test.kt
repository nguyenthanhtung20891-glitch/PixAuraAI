package ai.pixaura.app.commerce

import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import java.io.File
import java.security.KeyStore
import java.util.UUID
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith

@RunWith(AndroidJUnit4::class)
class NativeEditorInstallationKeyV2Test {
    @Test fun atomicBackupRestoresTheSameInstallationSecret() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val namespace = "native-key-test-${UUID.randomUUID()}"
        val alias = "${context.packageName}.$namespace"
        val file = File(context.noBackupFilesDir, "$namespace.key")
        val backup = File(context.noBackupFilesDir, "$namespace.key.bak")
        val owner = NativeEditorInstallationKeyV2(context, namespace)
        try {
            val first = owner.readOrCreate()
            try {
                val encrypted = file.readBytes()
                assertTrue(file.renameTo(backup))
                val recovered = owner.readOrCreate()
                try { assertArrayEquals(first, recovered) } finally { recovered.fill(0) }
                assertArrayEquals(encrypted, file.readBytes()); assertFalse(backup.exists())
            } finally { first.fill(0) }
        } finally {
            for (suffix in listOf(".key", ".key.new", ".key.bak", ".lock")) File(context.noBackupFilesDir, namespace + suffix).delete()
            KeyStore.getInstance("AndroidKeyStore").apply { load(null) }.deleteEntry(alias)
        }
    }
    @Test fun uncommittedFirstWriteCannotReplaceTheInstallationSecret() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val namespace = "native-key-test-${UUID.randomUUID()}"
        val alias = "${context.packageName}.$namespace"
        val pending = File(context.noBackupFilesDir, "$namespace.key.new")
        val bytes = ByteArray(60) { (it + 1).toByte() }
        try {
            pending.writeBytes(bytes)
            try { NativeEditorInstallationKeyV2(context, namespace).readOrCreate(); fail("uncommitted key silently replaced") } catch (_: Exception) { }
            assertArrayEquals(bytes, pending.readBytes())
            assertFalse(File(context.noBackupFilesDir, "$namespace.key").exists())
            assertFalse(KeyStore.getInstance("AndroidKeyStore").apply { load(null) }.containsAlias(alias))
        } finally {
            for (suffix in listOf(".key", ".key.new", ".key.bak", ".lock")) File(context.noBackupFilesDir, namespace + suffix).delete()
            KeyStore.getInstance("AndroidKeyStore").apply { load(null) }.deleteEntry(alias)
        }
    }
    @Test fun missingCiphertextCannotRotateAnExistingInstallationSecret() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val namespace = "native-key-test-${UUID.randomUUID()}"
        val alias = "${context.packageName}.$namespace"
        val file = File(context.noBackupFilesDir, "$namespace.key")
        val owner = NativeEditorInstallationKeyV2(context, namespace)
        try {
            owner.readOrCreate().fill(0); assertTrue(file.delete())
            try { owner.readOrCreate(); fail("lost ciphertext rotated native receipt secret") } catch (_: Exception) { }
            assertFalse(file.exists())
            assertTrue(KeyStore.getInstance("AndroidKeyStore").apply { load(null) }.containsAlias(alias))
        } finally {
            for (suffix in listOf(".key", ".key.new", ".key.bak", ".lock")) File(context.noBackupFilesDir, namespace + suffix).delete()
            KeyStore.getInstance("AndroidKeyStore").apply { load(null) }.deleteEntry(alias)
        }
    }
    @Test fun stablePrivateKeyFailsClosedOnDamageAndKeyLoss() {
        val context = InstrumentationRegistry.getInstrumentation().targetContext
        val namespace = "native-key-test-${UUID.randomUUID()}"
        val alias = "${context.packageName}.$namespace"
        val file = File(context.noBackupFilesDir, "$namespace.key")
        val owner = NativeEditorInstallationKeyV2(context, namespace)
        try {
            val first = owner.readOrCreate()
            try {
                assertEquals(32, first.size)
                val second = NativeEditorInstallationKeyV2(context, namespace).readOrCreate()
                try { assertArrayEquals(first, second) } finally { second.fill(0) }
                val encrypted = file.readBytes(); assertEquals(60, encrypted.size)
                assertFalse(encrypted.asList().windowed(32).any { it == first.asList() })
                val damaged = encrypted.copyOf().also { it[20] = (it[20].toInt() xor 1).toByte() }
                file.writeBytes(damaged)
                try { owner.readOrCreate(); fail("damaged secret was replaced") } catch (_: Exception) {}
                assertArrayEquals(damaged, file.readBytes())
                file.writeBytes(encrypted)
                val store = KeyStore.getInstance("AndroidKeyStore").apply { load(null) }; store.deleteEntry(alias)
                try { owner.readOrCreate(); fail("lost Keystore key was replaced") } catch (_: Exception) {}
                assertArrayEquals(encrypted, file.readBytes())
            } finally { first.fill(0) }
        } finally {
            for (suffix in listOf(".key", ".key.new", ".key.bak", ".lock")) File(context.noBackupFilesDir, namespace + suffix).delete()
            KeyStore.getInstance("AndroidKeyStore").apply { load(null) }.deleteEntry(alias)
        }
    }
}
