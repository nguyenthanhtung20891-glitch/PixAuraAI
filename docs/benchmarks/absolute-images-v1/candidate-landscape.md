# Production assessment candidate landscape — 2026-10-26

Public-source engineering screening only. No production candidate is selected or
admitted. UNKNOWN means not established; architecture estimates are not measured
artifact counts. No upstream benchmark is PixAuraAI quality evidence. The exact
source revisions, source hashes, artifact identity and observed metadata are retained
in landscape.json. Licensing below records declarations, not final legal clearance.

## Landscape and shortlist

| Family / task | Architecture, parameters, input | Output / fit | Source / weights / provenance | Decision |
|---|---|---|---|---|
| NIMA technical / aesthetic | MobileNet-v1 + 10-bin head, approximately 3.24M from standard architecture (not measured); 224 RGB; training loader resizes 256 then randomly crops 224; evaluation generator defaults to direct 224 resize | Rating distribution; technical variant may veto severe defects, aesthetic variant mixes style. Neither identifies an edit/direction | idealo implementation Apache-2.0; repo-hosted technical HDF5 pinned and SHA-256 measured; trained on TID2013, training custody/data-rights diligence incomplete. Google paper is research origin, not proof Google built these weights | SHORTLIST 1: compact technical-quality veto hypothesis; not integration-ready |
| FFCC / illuminant uncertainty | Learned log-chroma histogram filters/prior; project-dependent count, not a fixed CNN; linear black-level-corrected camera thumbnail | Illuminant posterior, not edited RGB or Kelvin parameter. Raw-camera domain differs from already white-balanced sRGB imports | Google code Apache-2.0, archived; train-per-imaging-pipeline recipe. No appropriate pinned rendered-sRGB production artifact established | SHORTLIST 2: bounded illuminant uncertainty hypothesis; input-domain/artifact gate fails |
| MobileIQA / mobile NR-IQA | Distilled MobileNet student / MobileViT teacher with attention; exact deployed count UNKNOWN; paper studies 224 through 1907x1231 | Global MOS, not per-defect direction or uncertainty calibrated to correction | Official repository lacks a declared LICENSE and published checkpoint in inspected tree; README testing/checkpoint TBU. No artifact identity/hash. Paper's TFLite experiment is not a distributable audited artifact | SHORTLIST 3: high-resolution technical-quality context hypothesis; rights/checkpoint gate fails |
| PaQ-2-PiQ / local-global quality | ResNet18-family body + ROI quality pooling; exact trained count/bytes UNKNOWN | Local/global quality, no safe correction direction | Official README declares CC BY-NC-SA 4.0; repository identity pinned | EXCLUDE commercial production shortlist: noncommercial restriction; no research inference performed |
| Deep White-Balance Editing | Multi-task encoder/decoder; exact count/input profile UNKNOWN; reduced-resolution estimation/full-resolution mapping; PyTorch net_awb.pth tree size 17,474,846 bytes | Corrected RGB / mapping, not just uncertainty; cannot be used as a new pixel executor | Official LICENSE.md CC BY-NC-SA 4.0, not MIT; repository includes model files but none downloaded | EXCLUDE: noncommercial terms and direct-image output incompatible with current eight-tool evidence-only role |
| Learning Multi-Scale Photo Exposure Correction | Multi-scale correction network; exact count/input policy not verified; MATLAB model.mat tree size 28,167,349 bytes | Corrected image, not exact existing tool parameters | Official LICENSE.md CC BY-NC-SA 4.0; no artifact downloaded or hash verified | EXCLUDE: noncommercial terms, direct enhancement semantics and unverified mobile conversion |
| HyperIQA | ResNet50 hypernetwork + target regressor; total exact count/bytes UNKNOWN; 224 patches | Score 0..100; content adaptation may help IQA but no action direction | Official code MIT; KonIQ checkpoint linked via Drive/Baidu without verified artifact/training custody. Training data rights require separate review | DEPRIORITIZE: heavier backbone/crop aggregation and incomplete artifact evidence; no demonstrated task advantage over compact shortlist |
| MUSIQ | Multi-scale transformer; exact checkpoint count/bytes not measured; native + 224 + 384 scales | Global aesthetic/technical quality; attention may contextualize quality, no tool direction | Official Google Research code Apache-2.0; named cloud checkpoints available; per-checkpoint license/training-data lineage not fully reviewed | DEPRIORITIZE: multi-scale execution/package complexity without task-specific correction evidence |

Existing EmbeddingGemma 2 is DISQUALIFIED FOR THE CURRENT PIXAURAAI MVP by accepted
Round 3 decision. No further probe, conversion, provenance repair or tuning occurs.

## Shortlist feasibility and explicit non-results

1. NIMA technical exists ONLY to test whether a small quality-distribution component
   improves a deterministic system's safe veto/abstention. It cannot assign brightness,
   exposure, warmth or sharpening values. The original Keras artifact is 13,174,216
   bytes, SHA-256 78c8bb95098fd27695de4147465f4fd504d0ca5e11699a487534854b5aeb3036,
   retrieved from the pinned idealo repository into ignored build evidence only.
   No conversion occurred. Architecture/config/preprocessing sources are identified;
   full training/redistribution custody and calibrated defect meaning are not proven.
2. FFCC exists ONLY to explore explicit uncertainty in color assessment. It requires
   linear camera-pipeline data and training for that pipeline. Decoding a processed
   JPEG to linear sRGB does not undo unknown camera white balance/tone mapping.
   Its original mobile speed result is upstream evidence for a different input/task,
   not PixAuraAI performance or rendered-photo correctness. A fresh domain-specific
   artifact and RGB-to-existing-temperature calibration would need independent review.
3. MobileIQA exists ONLY as a mobile-resolution IQA reference. The original paper
   reports TFLite FP16 single-CPU tests, including 3636/2625 ms at 1271x820 on Xiaomi
   10S/HONOR Magic5 Pro. Those are upstream measurements, not host/device evidence here.
   A paper claiming mobile efficiency does not establish a licensed available weight
   or prove safe edit advice. Missing rights and checkpoint prevent a responsible probe.

None currently has demonstrated incremental PixAuraAI quality. No learned inference
was executed this round. The host's existing Python 3.14 environment has no TensorFlow,
PyTorch, NumPy, Pillow, h5py or ONNX Runtime; no new framework was installed. NIMA
artifact acquisition/hash is NOT a load, latency, memory or quality test. No unreviewed
community conversion was substituted to bypass framework/provenance work. Lack of
execution is not evidence that NIMA performs badly: its quality remains NOT VERIFIED.

| Shortlist | CPU / Android / Apple feasibility | Conversion / quantization | Runtime determinism | Complexity |
|---|---|---|---|---|
| NIMA technical | Conventional MobileNet ops make CPU plausible; Android LiteRT and Apple Core ML conversion are engineering possibilities, NOT executed compatibility proofs | No vetted NIMA int8/FP16/ONNX/TFLite/Core ML artifact established; requires pinned conversion and parity | Disable dropout; pin exact evaluation resize rather than random training crops; fixed kernels/threads and score tolerance; no network after assets present | MEDIUM: old archived TF/Keras stack, rights, conversion and semantic veto calibration |
| FFCC | CPU Fourier/histogram implementation plausible; no deployed Android/Apple adapter verified; original paper mobile test does not qualify this application | No universal quantized artifact; FFT/posterior parity and linear sensor preprocessing require review | Fixed binning, boundaries, FFT precision and posterior arithmetic; project-specific calibration | HIGH for this product: raw-versus-rendered domain and model training, despite small potential arithmetic/model |
| MobileIQA | Paper Android CPU/TFLite experiment; Apple unverified; no PixAuraAI CPU/mobile execution | Paper FP16 conversion described; reproducible released conversion/weights not established | Freeze resize, attention/ops, evaluation mode and numeric tolerance; no sampling | HIGH: absent rights/checkpoint plus multi-scale attention, high-resolution working buffers and conversion |

All three are conceptually offline; documentation/research downloads do not imply
network inference. No runtime package size, peak RAM, post-close RAM or PixAuraAI
inference latency is established for any shortlisted model. No mobile trust claim.

## Other supply-chain cautions

The actively maintained IQA-PyTorch toolbox at the inspected revision declares
PolyForm Noncommercial in its current LICENSE, with additional component licenses.
It is not a blanket commercially permissive shortcut for downloading arbitrary IQA
weights. No toolbox was installed. MobileNet classification weights are not technical
IQA weights. Generic scene classification is not proof of a photograph's desired edit.

## Primary sources

- NIMA research origin: https://research.google/pubs/nima-neural-image-assessment/
- Actual implementation/weights: https://github.com/idealo/image-quality-assessment/tree/dceaf7c2d218bc6e80b21d6e147e3b56a21b7f31
- FFCC source: https://github.com/google/ffcc/tree/2fa9e1316954dbd3913630b7d597927941b4dd32
- FFCC paper: https://research.google/pubs/fast-fourier-color-constancy/
- MobileIQA source: https://github.com/chencn2020/MobileIQA/tree/c74fec95a98811b867618bbcd30913a1d415ff56
- MobileIQA paper: https://arxiv.org/html/2409.01212v1
- PaQ-2-PiQ: https://github.com/baidut/PaQ-2-PiQ/tree/a3ba2373a0da4e498d6e75ea2097125b5fe0fc10
- Deep WB: https://github.com/mahmoudnafifi/Deep_White_Balance/tree/d70afa227c94152fd8421db75b3ee6ac06a417dc
- Exposure correction: https://github.com/mahmoudnafifi/Exposure_Correction/tree/9ce36271db133da208ee7e20f9e0f99fdef75b63
- HyperIQA: https://github.com/SSL92/hyperIQA/tree/c42e7279717e7dcb693a24b891fc14a4189a45ee
- MUSIQ: https://github.com/google-research/google-research/tree/0f585ccae259c32a877edfd48362c18a0010545f/musiq
- Toolbox license: https://github.com/chaofengc/IQA-PyTorch/blob/a3eecfbf33bd9313ac613df9bc80ccbba94f28e5/LICENSE

No author's update activity is inferred from unrelated repository push timestamps:
landscape.json records inspected commit date; a monorepo commit is explicitly not
the model's latest substantive release. No moving URL or unverified mirror is a
production identity. Any future serious integration still needs the complete frozen
Production Intelligence Provenance/Evaluation v1 gate, not these screening notes.
