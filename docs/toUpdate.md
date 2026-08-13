# toUpdate — deferred fixes

Things known to be wrong or inconsistent, deliberately not fixed yet. Delete an entry when it lands.

Guide 3 is `docs/guide-3-completing-the-table.html`, modules 0, 1, 1.5, 2, 3, 4, 5.

---

## Next modules — planned, not written

### Module 6 — the chip

An `AHasardChip` actor spawned per click, stacked in Z per position, plus a translucent
copy following the aim as a cursor so the player sees which of the 157 positions they
are about to back before any money moves. Decided 2026-08-13:

- **Actor per chip**, not an instanced mesh. Simplest to read, matches the patterns
  already in the repo, and a stack is just actors at increasing Z. The scale a player
  can reach by hand does not need instancing.
- **The preview shows the bet name and its payout**, not just a ghost chip.
  `FHasardBetPosition::DisplayName` was built for exactly this, and naming the cost
  before the stake is taken is the responsible-gaming version of a hover readout.
- **Clearing goes through a delegate.** `UHasardBettingComponent` gains
  `FOnBetsCleared`, broadcast from `ClearAllBets` and at the end of `SettleRound`;
  the felt binds to it. The alternative — the GameMode calling both the component and
  the felt — puts chip lifetime somewhere that does not own it.
- The preview needs the hit location per frame, so `IHasardInteractable` gains
  `OnPlayerHover(APawn*, const FVector&)` and the pawn starts ticking. Fix the
  trace-origin entry below **first**, or the preview will sit where the ray lands
  rather than under the crosshair.

### Module 7 — the table the player sees

Green felt, red/black/green boxes behind the numbers, white borders, white numerals.
Requested 2026-08-13 and blocked on a data change rather than a material:

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
