# toUpdate — deferred fixes

Things known to be wrong or inconsistent, deliberately not fixed yet. Delete an entry when it lands.

---

## Guides

### Guide 2, module 9 — remove the break-to-verify step

`guide-2-building-hasard.html`, the **Build the bankroll** panel, step 10:

> **Deliberately break it.** Remove the `UFUNCTION()` from `HandleBankrollChanged`, rebuild, place a bet. It compiles, it runs, and the callback never fires — no error anywhere. Put it back. You have now seen the failure mode instead of reading about it, and you will recognise it at 2am.

Deliberate-breakage exercises are out. Rewrite as a positive check, or cut the step and renumber 11 → 10. The lesson (a bound callback needs a bare `UFUNCTION()`) stays — it is already stated in CONVENTIONS.md §4 and in the panel's own prose.

Only one instance in guide 2. Guide 1 has none. Guide 3 had one in module 1.5 and it has been removed.

---

## Code

### `EHasardBetType` — Trio and Basket are refused, not implemented

`HasardBettingComponent.cpp`, `IsSettleableBetType`, plus the two `case` labels in `BetCoversPocket`.

Both bets include zero and cannot be expressed by `FHasardBet::PrimaryNumber`. Module 1.5 refuses them so no stake is taken for a bet that cannot win.

**Delete all three when module 2 lands.** Once `FHasardBetPosition::CoveredNumbers` replaces `PrimaryNumber`, settling becomes `Position.CoveredNumbers.Contains(WinningPocket)` and no bet type needs a special case. The `static_assert` on the enum count stays — it guards the payout rules, not the encoding.

### `FHasardBet::PrimaryNumber` — to be replaced by a position id

Cannot express a vertical split, a trio, or the basket. `BetCoversPocket`'s whole switch exists to work around it. Module 2 replaces it.

### `GetGridExtent` tooltip overstates what it returns

`HasardTableLayout.h`:

> `/** Far corner of the number grid, so callers can size the felt without repeating the sum. */`

It returns the **number grid** only. `ZeroBoxWidth` extends past negative X and `OutsideBandDepth` sits beyond the grid, so a caller who sizes the felt from this alone clips the zero box and the outside bands. Suggested:

> `/** Far corner of the number grid. Not the felt — the zero box and the outside bands extend past it. */`

### `CONVENTIONS.md` §3 is stale on access specifiers

§3 says members are `protected` by default, "which is why it's the default here". Every `UPROPERTY` in `HasardGameMode.h`, `HasardTableLayout.h` and `HasardBettingComponent.h` is `private` with `meta = (AllowPrivateAccess = "true")`.

The code is the standard. Update §3 to say so.

### `CoveredNumbers` promises ascending order and nothing enforces it

`FHasardBetPosition::CoveredNumbers` tooltip says "ascending". Module 2's generator has to keep that promise in twelve separate blocks and nothing checks it. Cheap insurance: sort inside `AddPosition` rather than trusting each caller.
