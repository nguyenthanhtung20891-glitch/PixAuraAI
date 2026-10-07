import Foundation
import CoreGraphics
import CPixAuraCore

/// One-shot, off-main-thread consumer. The image owns a derived Data copy;
/// it does not retain the native handle or authorize document changes.
public enum ReferencePreview {
    public static func copyImage(context: inout pixaura_decode_context,
                                 preview: inout pixaura_decode_handle) -> CGImage? {
        var metadata = pixaura_preview_metadata()
        guard pixaura_preview_query(&context, &preview, &metadata) == 0,
              metadata.pixel_format == 1, metadata.width > 0, metadata.height > 0,
              metadata.width <= 1024, metadata.height <= 1024,
              metadata.row_stride == UInt64(metadata.width) * 4,
              metadata.image_bytes == metadata.row_stride * UInt64(metadata.height) else { return nil }
        var bytes = Data(count: Int(metadata.image_bytes))
        let status = bytes.withUnsafeMutableBytes {
            pixaura_preview_copy(&context, &preview, 0,
                                 $0.bindMemory(to: UInt8.self).baseAddress, metadata.image_bytes)
        }
        guard status == 0, let provider = CGDataProvider(data: bytes as CFData),
              let space = CGColorSpace(name: CGColorSpace.sRGB) else { return nil }
        return CGImage(width: Int(metadata.width), height: Int(metadata.height),
                       bitsPerComponent: 8, bitsPerPixel: 32,
                       bytesPerRow: Int(metadata.row_stride), space: space,
                       bitmapInfo: CGBitmapInfo.byteOrder32Big.union(CGBitmapInfo(rawValue: CGImageAlphaInfo.last.rawValue)),
                       provider: provider, decode: nil, shouldInterpolate: false, intent: .defaultIntent)
    }
}
