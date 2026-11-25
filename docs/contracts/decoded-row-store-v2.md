# Managed original reader v2: bounded decoded-row fallback

Implementation proposal within approved original-reader-v2/stage-plan-v2. This is
private versioned engineering, not a modification of the v1 decoder/working ABI,
Schema 3, numerical kernels, source ownership, or resource ceilings. No support or
performance PASS is claimed before the equivalence and application tests pass.

The current managed reader retains full RGBA8. A 12 MP high-entropy PNG can retain
about 48 MB encoded plus 48 MB decoded while opening; LOW's existing 64 MiB native
policy cannot admit that overlap. Normal 128 MiB admission already has evidence.
The fallback must improve this bounded path without increasing either ceiling.

Keep the existing in-memory path where admitted. Only managed immutable sources
may fall back after resource denial. Preserve the verified encoded bytes, frozen
metadata parser, limits, profile rejection, orientation and pinned codec options.
Use an additive private row decoder: identical JPEG ISLOW/RGBA/non-fancy settings;
identical PNG transforms, CRC and warning handling. Adam7 must read the previously
decoded row before combining each pass. Row callbacks never throw across C/longjmp.
All scratch and callback row bytes remain covered by the same native Budget.

Decode to a private temporary RGBA8 backing file in the validated project root.
It carries no revision, approval or executable authority. POSIX unlinks it immediately
after exclusive/no-follow creation; Windows uses exclusive delete-on-close semantics.
Crash/close therefore removes it without a persistent cache or backup inclusion.
Its exact length is checked against frozen admitted decoded metadata. Storage failure
aborts import/reopen without a native success receipt or commercial charge.

A bounded cache of at most sixteen 128×128 RGBA8 tiles (1 MiB) supports immutable
random access, including geometry and halos. Reserve before allocation; synchronize
cache access between actual native workers. No floating-point intermediate is stored
in the backing file. Conversion uses the unchanged sRGB/alpha tables and arithmetic.
Normal source byte identity and full-resolution export dimensions remain unchanged.
No downsampling, model, networking, analytics, device fingerprint or GPU admission.

Proof obligations: row decoder bytes against the frozen decoder for JPEG/PNG,
alpha/gray/palette/Adam7/orientation and malformed/truncated input; allocation and
storage/cancellation failures; exact tile-reader float parity at adversarial edges;
real high-entropy4000×3000 import/edit/history/full JPEG+PNG at existing LOW64 MiB;
no leaked file/FD/lease on failures or close; sanitizer/platform consumer coverage.
Mac/Simulator/emulator observations cannot certify physical cohorts or thermal gates.
