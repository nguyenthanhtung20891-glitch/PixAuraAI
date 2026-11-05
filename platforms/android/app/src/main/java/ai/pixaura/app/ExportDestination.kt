package ai.pixaura.app

import android.content.ClipData
import android.content.ContentResolver
import android.content.ContentValues
import android.content.Context
import android.content.Intent
import android.net.Uri
import android.os.Build
import androidx.annotation.RequiresApi
import android.os.Environment
import android.provider.MediaStore
import androidx.core.content.FileProvider
import java.io.File
import java.io.IOException
import java.io.OutputStream

/** Platform destination only. Encoded bytes are already a committed-revision projection. */
internal object ExportDestination {
    fun savePending(insert: () -> Uri, open: (Uri) -> OutputStream?, publish: (Uri) -> Unit, remove: (Uri) -> Unit, bytes: ByteArray): Uri {
        val uri = insert()
        try {
            open(uri)?.use { stream -> stream.write(bytes) } ?: throw IOException("missing destination")
            publish(uri)
            return uri
        } catch (failure: Throwable) {
            runCatching { remove(uri) }
            throw failure
        }
    }

    @RequiresApi(29)
    fun saveToPhotos(resolver: ContentResolver, name: String, mime: String, bytes: ByteArray): Uri {
        if (Build.VERSION.SDK_INT < 29) throw IOException("Photos save requires the document picker on this Android version")
        return savePending(
            insert = {
                val values = ContentValues().apply {
                    put(MediaStore.MediaColumns.DISPLAY_NAME, name)
                    put(MediaStore.MediaColumns.MIME_TYPE, mime)
                    put(MediaStore.MediaColumns.RELATIVE_PATH, Environment.DIRECTORY_PICTURES + "/PixAuraAI")
                    put(MediaStore.MediaColumns.IS_PENDING, 1)
                }
                resolver.insert(MediaStore.Images.Media.getContentUri(MediaStore.VOLUME_EXTERNAL_PRIMARY), values)
                    ?: throw IOException("missing destination")
            },
            open = { resolver.openOutputStream(it) },
            publish = { uri ->
                val done = ContentValues().apply { put(MediaStore.MediaColumns.IS_PENDING, 0) }
                if (resolver.update(uri, done, null, null) != 1) throw IOException("save failure")
            },
            remove = { resolver.delete(it, null, null) },
            bytes = bytes)
    }

    fun writeDocument(resolver: ContentResolver, uri: Uri, bytes: ByteArray) {
        try {
            resolver.openOutputStream(uri)?.use { it.write(bytes) } ?: throw IOException("missing destination")
        } catch (failure: Throwable) {
            runCatching { resolver.delete(uri, null, null) }
            throw failure
        }
    }

    fun cacheUri(context: Context, name: String, bytes: ByteArray): Uri {
        val dir = File(context.cacheDir, "export").apply { mkdirs() }
        dir.listFiles()?.forEach { child -> child.delete() }
        val file = File(dir, name)
        file.writeBytes(bytes)
        return FileProvider.getUriForFile(context, context.packageName + ".export", file)
    }

    fun shareIntent(uri: Uri, mime: String): Intent = Intent(Intent.ACTION_SEND).apply {
        type = mime
        putExtra(Intent.EXTRA_STREAM, uri)
        clipData = ClipData.newRawUri("PixAuraAI export", uri)
        addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
    }
}
