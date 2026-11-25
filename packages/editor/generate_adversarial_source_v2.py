"""Synthetic 12 MP codec fixtures, not production/device-memory instrumentation."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import zlib

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("output", type=Path)
parser.add_argument("--channels", type=int, choices=(3, 4), default=4)
args = parser.parse_args()
width, height = 4000, 3000
seed = b"PixAuraAI synthetic adversarial 12MP v2"
pixels = hashlib.shake_256(seed).digest(width * height * args.channels)
compressor = zlib.compressobj(1)
payload = bytearray()
stride = width * args.channels
for y in range(height):
    payload.extend(compressor.compress(b"\0" + pixels[y * stride:(y + 1) * stride]))
payload.extend(compressor.flush())


def chunk(kind, data):
    return (struct.pack(">I", len(data)) + kind + data
            + struct.pack(">I", zlib.crc32(kind + data) & 0xffffffff))


encoded = (b"\x89PNG\r\n\x1a\n"
           + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8,
                                       2 if args.channels == 3 else 6, 0, 0, 0))
           + chunk(b"IDAT", payload) + chunk(b"IEND", b""))
args.output.write_bytes(encoded)
args.output.with_suffix(".json").write_text(json.dumps({
    "synthetic": True, "width": width, "height": height,
    "channels": args.channels, "bytes": len(encoded),
    "sha256": hashlib.sha256(encoded).hexdigest(),
    "method": "SHAKE256 fixed seed, filter0, zlib level1",
    "seed": seed.decode("ascii"),
}, indent=2) + "\n")
