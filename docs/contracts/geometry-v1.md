# Bounded geometry planning, version 1

## Coordinates and historical tuples

Canonical normalized orientation is 1. Origin is the top-left image edge; +X right, +Y down. Extent W,H covers [0,W) x [0,H). Pixel (x,y) occupies [x,x+1) x [y,y+1), with center (x+1/2,y+1/2). Integer rectangles are half-open pixel-edge bounds. Centers and edges are never interchangeable. Zero dimensions, empty/reversed rectangles and invalid parameters reject; no clamping.

Crop/1/1 remains exactly x_ppm,y_ppm in 0..999999, width_ppm,height_ppm in 1..1000000, sums <=1000000. Units are millionths of the current input extent after preceding operations. For M=1000000, the Product Owner approved:

    x0 = floor(W*x_ppm/M)
    x1 = ceil(W*(x_ppm+width_ppm)/M)
    y0 = floor(H*y_ppm/M)
    y1 = ceil(H*(y_ppm+height_ppm)/M)
    output = (x1-x0, y1-y0)

Ceil is checked (product+M-1)/M. All operands use checked unsigned integer sums/products; no floating conversion. Legal positive crops always produce nonempty bounds wholly inside input, including 1x1 images. Outward rounding applies only to integer rasterization of exact rational edges. No interpolation or fractional resampling. Future crop pixel mapping is source=(destination.x+x0,destination.y+y0); this step executes no crop pixels.

Rotate/1/1 retains quarter_turns 0..3 clockwise. For source index (x,y):

| Turns | Output W,H | Destination index |
| --- | --- | --- |
| 0 | W,H | x,y |
| 1 | H,W | H-1-y,x |
| 2 | W,H | W-1-x,H-1-y |
| 3 | H,W | y,W-1-x |

Inverse is the (4-turns)%4 forward mapping on destination extent; indices validate before subtraction. Edge-coordinate mappings replace W-1/H-1 with W/H: turn 1 maps (u,v) to (H-v,u), turn 2 to (W-u,H-v), turn 3 to (v,W-u). This rotates canonical pixels, never reapplies EXIF orientation. Crop/rotation ordering follows recorded stack order exactly.

## Planning, tiles and admission

Shared document parsing accepts the unchanged exact evaluation envelope and strict integer schema. Native preflight retains one Stage per operation: input/output extent, source rectangle and turn count. Exposure stages retain extent; crop selects its source rectangle; rotation retains its full source rectangle. No reordering, merging or folding. Internal stages preserve correspondence with the caller stack. Public preflight returns final dimensions, tile count, executable flag, output raster bytes, actual stage vector capacity bytes and conservative stage-input pixel visits. Planning does not allocate a raster or mutate source/history/persistence. Geometry stages set executable=0, including zero turns; executable evaluation still returns unsupported 5.

Fixed implementation tile edge 128, row-major (top-to-bottom, left-to-right), lazy checked index lookup. Tile i has column i%ceil(W/128), row i/ceil(W/128). Origin is column*128,row*128; ends are min(origin+128,extent). Tiles cover the full logical destination exactly once, with bounded edge tiles, no overlap/gaps. Internal tile rectangle is destination-only for exposure; no halo/source tile cache is needed. Source rectangles for future geometry live in ordered stages, not owning tile pointers. No tile metadata array allocation exists; temporary tile state is constant stack space.

Limits: operations 256; serialized request 65536 bytes; each operation 1024 consumed bytes under shared parser; stage metadata <=256*sizeof(Stage) payload (currently 9216 bytes); total parser+plan conservative reservation 1 MiB plus bounded STL/control overhead. Tile count <=4096; dimensions <=16384; pixels <=8388608; row <=262144 bytes; working raster <=128 MiB. Positive caller limits only reduce defaults. Planned stage-input pixel visits <=2147483648 (256*8388608); executed validation/copy adds <=8388608 visits. Context payload remains <=256 MiB, with <=64 combined image/source/cancellation handles. Tiling does not admit larger logical images, reduce destination ownership, or change full-raster allocation limits. Maximum request working rasters: immutable source plus private destination, no per-operation raster and no heap kernel scratch.

Geometry family version 1 adds geometry_preflight and geometry_tile_at. All outputs remain unchanged on failure. Existing status meanings: 1 argument, 2 API version, 3 stale/wrong kind, 5 unsupported tuple/execution, 6 malformed envelope, 7 invalid parameters/geometry, 8 resource/allocation limit, 14 internal, 19 invalid working layout. ABI 1 remains unchanged. Plans/tiles never persist as authoritative project data; platform layers call native APIs and own UI publication.

## Golden geometry

2x3 labels a b / c d / e f rotate clockwise to e c a / f d b (90), f e / d c / b a (180), b d f / a c e (270). A 3x2 crop x=250000,width=500000 covers x=[0,3); x=999999,width=1 covers x=[2,3). Full crop retains dimensions; any positive legal crop of 1x1 yields 1x1. 129x129 has four tiles: [0,128)x[0,128), [128,129)x[0,128), [0,128)x[128,129), [128,129)x[128,129).
