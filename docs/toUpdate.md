# toUpdate — deferred fixes

Things known to be wrong or inconsistent, deliberately not fixed yet. Delete an entry when it lands.

Guide 3 is `docs/guide-3-completing-the-table.html`, modules 0, 1, 1.5, 2, 3, 4, 5, 6.

**As of 2026-08-14 the guide is deliberately trailing the code.** See "Guide 3 is being
left behind on purpose" under Guides before fixing anything in a shown code block.

---

## Next modules — planned, not written

### Module 7 — the cursor chip and the bet readout

The half of the chip work module 6 did not cover: a translucent chip following the aim,
plus a readout naming the bet and its payout, so the player sees which of the 157
positions they are about to back and what it costs before any money moves. Decided
2026-08-13:

- **The preview shows the bet name and its payout**, not just a ghost chip.
  `FHasardBetPosition::DisplayName` was built for exactly this, and naming the cost
  before the stake is taken is the responsible-gaming version of a hover readout.
- It needs the hit location every frame, so `IHasardInteractable` gains
  `OnPlayerHover(APawn*, const FVector&)`, the pawn starts ticking, and the pawn tracks
  the last actor it hovered so it can tell one to stop previewing.
- Reuses `BP_Chip` with a translucent material instance rather than a second mesh.
- **Fix the trace-origin entry below first.** Until the ray starts at the camera, the
  ghost chip will sit where the ray lands rather than under the crosshair, and the
  whole point of the preview is that those agree.

### Module 8 — the table the player sees — WRITTEN 2026-08-14, guide still owed

The code landed. What follows is the record of what it does and what still needs doing,
because the guide module has not been written and this is the only account of it.

**`FHasardBetPosition` gained `BoxSize`.** The rectangle a position is printed as,
centered on `ChipLocation`. Zero for the 83 line bets, and that zero *is* the "has no
box" test rather than a separate flag. `AddPosition` takes it last with a zero default, so
the line-bet call sites did not change. 49 positions have a box: zero, 36 straight ups,
3 columns, 3 dozens, 6 even-money.

**The felt prints itself from four components plus one.** `Cloth` is a single quad over
the felt bounds plus `ClothMargin`. `Lines` is an instanced component holding one white
quad per box at full size. `RedBoxes`, `BlackBoxes` and `GreenBoxes` hold the colored
faces, each inset by `BorderWidth` on every side — **there is no line geometry, the plate
showing round each box is the line.** That is why a line cannot be drawn where a box is
not. `ZeroBox` is its own single component so that zero's green can never be the same
value as a dozen's by construction.

Verified 2026-08-14 against `CellSizeX/Y = 24`, `ZeroBoxWidth = 24`, `OutsideBandDepth =
24`: 49 boxes, no two overlapping, all inside the felt bounds. Box area 38 016 cm² against
a felt of 40 320 cm² — the 2 304 cm² difference is the two corners below the zero box and
below the column band, which carry no bet and are bare cloth on a real table too.

**`HasardAuditLayout` now checks the boxes.** No two overlap, none escapes the bounds.
Both are properties of the layout asset, so they are checked before anything is drawn.
The overlap test matters more than it looks: `BoxSize` and `ResolvePosition` read the same
measurements, so two boxes drawn over each other is also two cells whose clicks land
somewhere unintended, and that half of it is silent.

**Four properties were deleted, not renamed.** `RedNumberColor`, `BlackNumberColor`,
`ZeroLabelColor` and `OutsideLabelColor` are gone; `NumeralColor` replaces all four with a
single printed white. No core redirects were added *deliberately* — a redirect would carry
a numeral color into a box color, and those mean different things. If `BP_Felt` overrode
any of the four, that override is correctly dropped. `LabelColorFor` is deleted and its
red/black/zero rule now lives in `BoxesFor` and the `ZeroBox` routing.

**`bDrawDebugLayout` now defaults to false**, since the felt draws its own lines. A
`BP_Felt` that already overrides it to true keeps that override — **uncheck it by hand.**

Still owed:

- The guide module, once the table stops moving.
- Numerals are still 49 `UTextRenderComponent`s. That is now the felt's largest draw-call
  cost by a wide margin, and it is the thing to attack first if VR needs headroom — an
  atlas material or baked text would collapse it to one.
- The even-money `RED` and `BLACK` boxes are rectangles. A real layout prints them as
  diamonds, which needs a second mesh rather than a second color.
- `SurfaceMaterial` is one asset for cloth, lines and boxes. If the cloth ever wants a
  nap or a weave normal, it needs its own.

### Module 8 (original entry) — the table the player sees

Green felt, red/black/green boxes behind the numbers, white borders, white numerals.
Requested 2026-08-13, restated 2026-08-14, and blocked on a data change rather than a
material:

**`FHasardBetPosition` needs a box extent.** Drawing a box behind 17 needs 17's
rectangle, and number cells, dozens (four cells wide), columns and even-money boxes
(a sixth of the grid) are all different sizes. Deriving them in the felt from
`CellSizeX` and the bet type would be a second copy of the layout arithmetic, which is
the thing modules 3 and 4 both refused. So `BuildPositions` generates a `FVector2D
BoxSize` alongside `ChipLocation`, and the audit gains a check that no two boxes of the
same bet type overlap.

Rendering approach not yet decided. One plane mesh per box with a material instance per
color is the natural continuation of module 4's 49 components, and a white border
derived from UV gives the lines for free. An instanced mesh with per-instance color, or
baking the whole felt to a render target, are both cheaper at runtime and bigger jumps.

**It retires three properties.** Once the box behind a number carries the color, the
numeral goes white and `RedNumberColor`, `BlackNumberColor` and `ZeroLabelColor` stop
being label colors — they become box colors. `LabelColorFor` either returns a single
white or is deleted outright, and the three properties move to whatever draws the boxes.
Worth deciding deliberately rather than leaving a felt that colors a numeral *and* the
box behind it, which is two sources for one fact.

Zero keeps its own color through the move. It is green on a real table because it is the
house edge, and CLAUDE.md's anti-patterns forbid printing it as though it were one of the
blacks — that applies to the box as much as it did to the numeral.

---

## Guides

### Guide 3, module 4 — `LabelRotation` default renders nothing

`FRotator(90.0f, 180.0f, 0.0f)` puts the text face-down. `UTextRenderComponent` renders
single-sided, so the labels exist, log correctly as 49, and are invisible from above.
Confirmed in the editor 2026-08-13.

Pitch decides up or down, yaw decides mirrored or not. The panel shows X/Y/Z as
Roll/Pitch/Yaw, which is its own trap. `Y = -90` renders; whether `Z` should be 0 or
180 was still being eyeballed when the session ended — the last screenshot read
upside-down from the player's side, so 180 is suspect.

**Set the working values as the default and fix step 6's prose**, which says only "if
the numbers come out mirrored, add 180 to Yaw". The actual first failure is that
nothing appears at all, and that is a pitch problem.

### Guide 3 is being left behind on purpose until the table settles

Decided 2026-08-14: **the code leads, the guide gets regenerated in one pass at the end.**
Patching module-by-module was costing a diff per session on files that are still moving,
and every patch risks the marker classification without buying a reader anything until
the table is finished.

So the shown copies of `HasardFelt.h/.cpp` in modules 3, 4 and 6 are known stale. What
has diverged so far:

- `LabelTextSize` is `8.0f`, not `14.0f`, and its comment says "about 8". A 14 cm cap on
  a 24 cm cell crowded the lines.
- `RedNumberColor` `(200, 30, 30)`, `BlackNumberColor` `(20, 20, 20)`, `ZeroLabelColor`
  `(20, 130, 60)`, `OutsideLabelColor` `(40, 40, 40)`. The guide still shows the
  saturated originals.
- `Surface`'s tooltip is "Sized from the layout, never authored by hand." The guide still
  says "Offset against Root, so resizing it never moves the actor" — that reason survives
  in `SyncBoxToLayout`'s own comment, so nothing was lost.
- Module 6 step 18's prose said the chip-clear line prints *after* the settlement
  summary. It prints before, because `DropAllBets` broadcasts on the line above that
  `UE_LOG`. **Already fixed in the guide** 2026-08-14.
- `BuildLabels` no longer reads `Position.DisplayName` and `LabelRotation` directly. It
  calls `LabelTextFor` and `LabelRotationFor`, and there is a second rotation property.
  See the entry below.

When the guide is rebuilt, diff every shown file against the repo rather than against the
previous module — the baselines in between are all suspect now.

### The felt prints "2 to 1" but the position is still called "Column 2"

Added 2026-08-14 for the real-table look. `AHasardFelt::LabelTextFor` substitutes
`"2 to 1"` for `EHasardBetType::Column`; `FHasardBetPosition::DisplayName` is untouched.

The split is deliberate and worth not collapsing later. `DisplayName` is what the click
log prints and what module 7's readout will name *before the stake is taken*. "2 to 1" is
a payout, not a bet — a player reading it would not know which of the three columns they
were about to back, which is the same failure as an unlabeled bet.

`LabelRotationFor` returns `LabelRotationTurned` for those three boxes and `LabelRotation`
for everything else, because the column boxes are the only positions past the end of the
grid and so the only ones read from that end. Two properties rather than one plus 90° of
yaw: the working values for the main rotation are still being found by eye (see the
module 4 entry above), and deriving one from the other would move both groups on every
correction.

**`LabelRotationTurned` needs setting on `BP_Felt`.** Its C++ default is
`FRotator(90, 90, 0)` and the Blueprint override is what actually renders.

### `layout-audit.md` still records the 12 cm table

`docs/layout-audit.md` is the 2026-08-12 run of `HasardAuditLayout` and its header says
`DA_HasardTableLayout 12x3`. The counts and the 2.70% edges are all still correct —
they depend on the 12×3 grid, not on cell size — but the run predates the move to 24 cm
cells. Re-run the audit and replace the log dump when convenient. Nothing in it is
wrong, it is just no longer the current table.

### Guide 3, module 1.5 — quiz answers do not use all four positions

Answers are 2, 1, 1, 2, 1, 1 — positions 0 and 3 never appear. Modules 0, 1, 2, 3, 4
and 5 are all fixed. Move two correct options and update their `c` values.

### Guide 2, module 9 — remove the break-to-verify step

`guide-2-building-hasard.html`, the **Build the bankroll** panel, step 10:

> **Deliberately break it.** Remove the `UFUNCTION()` from `HandleBankrollChanged`, rebuild, place a bet. It compiles, it runs, and the callback never fires — no error anywhere. Put it back. You have now seen the failure mode instead of reading about it, and you will recognise it at 2am.

Deliberate-breakage exercises are out. Rewrite as a positive check, or cut the step and renumber 11 → 10. The lesson (a bound callback needs a bare `UFUNCTION()`) stays — it is already stated in CONVENTIONS.md §4 and in the panel's own prose.

Only one instance in guide 2. Guide 1 has none. Guide 3 had one in module 1.5 and it has been removed.

### A copy of guide 3 is still in `C:\Users\Joepi\Downloads`

The authoritative copy is now `docs/guide-3-completing-the-table.html`. **Delete the
Downloads one.** Two diverging guides is the failure mode CLAUDE.md's one-copy rule
exists to prevent, and this is exactly how it starts.

### Guide 3's per-module commits have drifted from the history

Each Build panel ends with its own commit message, and the intent is one commit per
module. Modules 3 and 4 landed together because `HasardFelt.h/.cpp` carry changes from
both and cannot be split by file. Not worth rewriting history over; worth knowing the
"module after the last one named in the log" rule in CLAUDE.md is now approximate.

---

## Code

### The interaction trace starts at the pawn, not the camera

`HasardInteractionComponent.cpp`, `TraceForInteractable`:

> `OwnerPawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);`

`GetActorEyesViewPoint` returns the pawn's eye height and the control rotation. On
`AHasardPlayerPawn` the camera sits at the far end of a spring arm, so the ray's
direction matches the view but its origin is a boom length in front of it. A centred
crosshair therefore cannot agree with where the trace lands, and the aim error grows
with `CameraDistance`. Confirmed in the editor on 2026-08-12: the debug line renders
well off to the side of the crosshair.

Fix, plus `#include "GameFramework/PlayerController.h"`:

```cpp
if (const APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
{
    // The camera, not the pawn. On a spring arm those are a boom length apart, and
    // the crosshair sits on the camera - a ray from the pawn cannot agree with it.
    PC->GetPlayerViewPoint(EyeLocation, EyeRotation);
}
else
{
    OwnerPawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);
}
```

`TraceDistance` then measures from the camera, so it loses a boom length of reach and
may need raising. Wrong in **guide 2 module 7** and carried into **guide 3 module 3
step 7** — both shown copies need the same edit. **Do this before module 6**, or the
chip cursor will sit where the ray lands rather than under the crosshair.

### `TraceRadius` default of 5.0 is wider than the thing it resolves

`HasardInteractionComponent.h`. A 5 cm sphere sweep reports `ImpactPoint` where the
sphere first touched, which on an angled view is displaced from the line of sight by up
to the radius. The line-bet band is `LineTolerance × CellSizeX` — 3.6 cm at
`LineTolerance = 0.15` and `CellSizeX = 24` — so the sweep shape is wider than the
feature it is trying to land on.

Set to **0**, which `UWorld::SweepSingleByChannel` turns into a plain line trace. Fixed
by hand on `BP_PlayerPawn` 2026-08-13; the class default is still 5.0 and guide 2's
shown copy still says 5.0.

### `DA_AmericanPayouts` has 13 rules and no Trio or Basket row

`DA_EuropeanPayouts` has all 15. Guide 3 module 3 deleted `IsSettleableBetType`, which
was the only thing making that gap unreachable. Point `BP_HasardGameMode` at the
American asset and a trio can now be placed: `TryStake` takes the money, the position
covers the pocket, `FindRule` returns `nullptr`, and settlement logs *"no rule for
Trio, bet not paid"* — after the stake is gone.

Either add the two rows (Basket is **five** numbers at 6:1 on a double-zero wheel, not
four at 8:1), or have `PlaceBet` reject a position whose bet type has no rule in the
assigned table. The second closes the whole class; see the entry below for why it is
not a one-liner.

### `PlaceBet` cannot validate that a position id exists

`HasardBettingComponent.cpp`. It rejects `INDEX_NONE` and nothing else, because it holds
neither the layout nor the payout table. `PlaceBet` is `BlueprintCallable`, so any
positive integer takes the stake and is reported by `SettleRound` afterwards.

The header comment now says so honestly rather than claiming a check that does not
exist. The durable fix is passing both assets in:

```cpp
bool PlaceBet(int32 PositionId, int32 Stake, const UHasardTableLayout* TableLayout,
    const UHasardPayoutTable* PayoutTable);
```

`SettleRound` already takes both, so it is consistent — but it changes the felt's call
site and the pawn's, so it wants its own module rather than being bolted onto one.

### `HasardAuditLayout` builds its own array instead of reading the cache

`HasardGameMode.cpp` calls `TableLayout->BuildPositions(Positions)` into a local. Since
module 3 introduced `GetPositions()` and a lazily built `CachedPositions`, the audit
validates an array the game never touches — so it cannot catch a stale cache, which is
the one new failure mode the cache introduced.

```cpp
const TArray<FHasardBetPosition>& Positions = TableLayout->GetPositions();
```

Then `BuildPositions` drops to `private:` beside `AddPosition`, since `GetPositions` is
its only remaining caller.

### `GetGridExtent` and `GetLineTolerance` have no callers

`HasardTableLayout.h`. `GetFeltBounds` took over the first; `ResolvePosition` reads the
`LineTolerance` member directly. Neither is called from any of the eleven files across
modules 1–5. Delete both, or state in the comment who they are for — `GetGridExtent`'s
tooltip was carefully corrected in module 3 for a function nothing uses.

### `CONVENTIONS.md` §3 is stale on access specifiers

§3 says members are `protected` by default, "which is why it's the default here". Every `UPROPERTY` in `HasardGameMode.h`, `HasardTableLayout.h` and `HasardBettingComponent.h` is `private` with `meta = (AllowPrivateAccess = "true")`.

The code is the standard. Update §3 to say so.
