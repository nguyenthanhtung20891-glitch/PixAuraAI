# PixAuraAI product specification

Status: authoritative initial baseline, 2026-10-01. Implemented status is tracked separately in ROADMAP.md and milestone reports.

## Thesis and users
The user defines the outcome; AI operates editing tools; the user reviews, refines and approves. The user may be Director, Approver or Operator. AI may be Advisor, Planner or Operator. AI-first never means AI-only. Android and iOS are production targets, including lower-performance devices.

## Interaction modes
AI Edit: import -> intent -> analyze -> inspect plan -> execute into candidate -> preview -> compare -> approve or refine -> export. A plan can be edited or cancelled. Approval is required before a candidate becomes the accepted project revision.

Manual Edit: import -> select tool -> manipulate preview -> commit operation -> compare/undo/redo -> export. No AI runtime, account, model download or connectivity is necessary.

AI-Assisted Manual: analysis and suggestions explain likely improvements and tradeoffs. Suggestions do not apply themselves. The user operates tools; accepting a suggested setting goes through the ordinary manual command path.

## MVP requirements
| ID | Requirement | Acceptance |
| --- | --- | --- |
| P01 | Import JPEG/PNG from system picker | Managed immutable copy; orientation handled; cancellation/failure leaves no broken project |
| P02 | Crop, rotate, resize | Bounded validated dimensions; preview and export share geometry semantics |
| P03 | Exposure, brightness, contrast, highlights, shadows, saturation, temperature | Documented units/ranges; CPU reference and GPU match within declared tolerance |
| P04 | Sharpen, blur, basic filters | Versioned recipes; deterministic replay; no hidden destructive changes |
| P05 | History, undo/redo, compare | Shared history for all modes; restart restores approved state; undo after export works |
| P06 | Save copy/export | New destination; source hash unchanged; failed write removes incomplete destination |
| P07 | AI commands, Auto Enhance, Smart Lighting | Local bounded plans using registered tools; approval required; offline and cancellation tested |
| P08 | Subject-aware adjustment, background removal, portrait blur | Optional local segmentation with editable mask; explicit unavailable state on unsupported device |
| P09 | Advisor foundation | Explain suggestions and limitations without changing project |
| P10 | Adaptive capability | Manual tools work at minimum tier; AI tier follows measured capability and memory pressure |
| P11 | Privacy and accessibility | No image-content network traffic; system pickers; accessible controls and compare alternatives |
| P12 | Licensing | 14-day trial, $0.99/month, $19.99 lifetime; local tools never consume AI credits |

Subject features belong to MVP but are capability-gated, not promised on every device. Provide clear reasons, reduced-resolution alternatives and manual masking where AI is unavailable. Supported first import/export formats are SDR JPEG and PNG; HEIF decoding via platform codec is later unless validated in Phase 2. RAW, HDR editing, generative fill, object synthesis, cloud collaboration and video are outside MVP.

## Product constraints
Originals never overwritten; imported copies are retained until explicit project deletion. Multiple projects and recovery after interruption are required. AI failures preserve the last approved revision. Export reflects an explicitly selected approved revision; candidate export requires explicit approval first. Offline licensing behavior is in MONETIZATION.md. Performance budgets are targets, not achieved claims.
