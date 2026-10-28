# Review preview numerical fixture v1

Project-owned deterministic synthetic RGB image, 640×480; no external image or model.
R=floor(x*255/639), G=floor(y*255/479), B=64+128*((floor(x/32) XOR floor(y/32)) AND 1).
Top-left rows, opaque RGB8 PNG, filter 0; Node 24.14.0 zlib level 9.
Used for host preview diagnostics and exact native approved-output comparisons,
not artistic/photo intelligence quality evidence. No runtime asset packaging.
SHA-256: `eb90137f56c0597b74f5e42ced84e05832e21724b0429e2ffc704621091a437b`.
Artifact bytes: 118440.
