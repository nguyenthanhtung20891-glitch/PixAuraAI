# Cross-platform manual editor integration v1

Authority: Phase 3 Step 7 only, baseline
`116c4efc65f55025f717241a01bc6056ed27d042`. Steps 1–6 remain accepted.
Step 8 has NOT started. DH-APPLE-METAL-01 is unchanged.

## Owners and acceptance criteria

Compose and SwiftUI are presentation over the same shared application service.
The additive checked `editor.h` family composes existing AssetStore, schema-3
Repository, document C handles, Step 6 manual gestures and PRV1. It is neither a
second gesture engine nor a scheduler/publication owner. Kotlin and Swift retain
the established platform owner pattern: stable private native context storage,
short lifecycle/check+display lock, render/copy outside the lock, detached image
copies, bounded admission, stop -> join -> destroy. No numerical/kernel change.

Measurable acceptance: eleven shared descriptors appear on both platforms with
identical IDs/ranges/defaults/steps/versions; one changed Apply creates one durable
revision, default/neutral Apply and cancel create none; 10,000 updates retain one
pending edit; end-to-end import/preview/Apply/undo/redo/reopen reproduces pixels;
cancel/switch/background/disposal revoke delayed preview installation. Native,
Android controller/Compose and Swift owner/SwiftUI tests must actually execute.

## Information architecture and integers

Categories: Geometry, Tone/Color, Detail, Filters/Presets, in that presentation
order. Within a category, both UIs retain the shared ASCII descriptor order.
All tool identities, names, integer fields/min/max/default/step and versions come
from `pixaura_manual_registry_json`; platforms do not construct operations.
The shared service builds an exact integer request from descriptor-indexed values
and submits it to the existing parser/controller. No platform float is durable.

Each tool selection starts a new append adjustment on the current committed
revision, initially neutral. Existing ordered operations are never fused/reordered.
Sliders round their platform position once to the descriptor's integer lattice;
native UPDATE is authority. EV/thousandths/K formatting is presentation only.
Default is a visible button. Rotation renders buttons from shared quarter-turn
metadata (0/1/2/3), displaying degrees clockwise. Crop uses four exact ppm text
fields and explicit Preview crop; parsing whole-number text is presentation,
rectangle validation/rasterization stays native. Invalid rectangles are rejected,
never clamped. This is the smallest usable crop interaction; no overlay math.

Filters/Presets is visible with an empty-state panel. No reference recipe is
shipping content; no fake catalog, preset semantics change or new tool.

## Lifecycle, bounded work and display

Selection cancels/revokes/releases the old gesture before BEGIN for the new tool.
Category navigation cancels an active adjustment. UPDATE coalesces pending values;
slider release does not commit. Apply is explicit COMMIT, followed by ordinary
application approval/persistence. Native no-op detection remains authoritative.
Undo/redo is disabled/rejected while active, with no platform shadow history.
Cancel and interruption restore the retained committed display immediately when
the owner processes the event, then regenerate the committed preview as needed.

Each platform uses two fixed application work lanes (commands and rendering),
one latest pending integer payload and one latest preview request, one render at
a time, no task per slider tick. Render admission is below the existing two-render
cap, not a new global ceiling. Detached fit images are <=512x512 (<=1MiB), below
the accepted 1024/4MiB platform bounds. Late completion must pass native gesture
and PRV1 eligibility inside the platform lock covering actual display replacement.
Failure preserves the previous valid image; preview loading explicitly says it
is not a durable commit. UI widgets never decide publication eligibility.
Pending UI input carries native edit identity to reject delayed old-tool events.
Apply consumes the latest pending accepted integer payload before closing.

Background, navigation, screen disposal and close cancel uncommitted work, never
auto-commit. Close signals stop, joins native render/copy calls, then destroys
document before raster. Re-entry opens a fresh core session from SQLite; gesture
state is never persisted/restored. Configuration changes likewise reopen approved
state, not in-progress controls. Local preference stores only last project name.

## Import, approval, failures and storage

System pickers accept JPEG/PNG only. Streaming import copies source bytes into
immutable managed storage under app-private no-backup roots; source never changes.
Android uses bounded private staging plus a <=64KiB reader; Swift uses a scoped
file reader. Shared decoder admission limits and verified source metadata remain
authoritative. Apple creates roots with data protection and backup exclusion.
Root namespace stability/canonical paths retain ADR 0010 requirements.

A genuinely new project explicitly provisions existing 1->2->3 schema transforms
before root-document creation. Reopen requires schema 3; it never migrates an
existing project automatically. No Schema 4 or durable UI state is introduced.

Changed COMMIT creates one Step 6 detached proposal. The shared application service
prepares/validates the ordinary transition and all next-state allocation before
Repository.apply(expected epoch). Only successful durable approval replaces live.
A storage failure retains the exact proposal for Retry Apply, never a second
gesture commit. Cancel discards it. Stale storage failure invalidates/cancels;
reopen is required. Checkpoint failure after durable success remains success,
according to the existing Repository contract. Preview failure is separate from
approval failure; a new generation retries rendering without new history.

The UI DTO exposes the actual native `gesture_state` separately from `active`
(an open adjustment, including a completed proposal awaiting approval retry) and
`approval_pending`. History/input controls remain disabled while approval is
pending. Retry Apply is persistence retry, never another Step 6 COMMIT.

UI errors distinguish invalid integer/rectangle, stale edit, resource/admission,
unavailable capability, preview and commit failure. Errors are visible, not silent
success. Accessibility exposes named controls, current integer values, selected
tools, loading/error messages and disabled actions. Scrollable content and tool
rows, bounded-fit preview and existing safe-area handling support common phones
and narrow/wide layouts. No specialized tablet design or certification claim.

A: required manual editor/service/parity/tests. B: Step 8 certification and closure.
C: crop overlay, artistic catalogs and visual polish. No AI/GPU/network/export
redesign, dependency or numerical semantics expansion.
