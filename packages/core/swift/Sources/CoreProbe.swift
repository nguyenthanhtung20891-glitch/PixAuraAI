import CPixAuraCore

public enum CoreProbe {
    public static func abiVersion() -> UInt32? {
        var info = pixaura_core_info()
        let status = pixaura_get_core_info(
            UInt32(PIXAURA_ABI_VERSION), &info,
            UInt32(MemoryLayout<pixaura_core_info>.size))
        return status == PIXAURA_OK ? info.abi_version : nil
    }
}
