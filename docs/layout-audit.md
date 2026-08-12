Guide 3, module 2 — `HasardAuditLayout`
2026-08-12 · DA_HasardTableLayout 12x3, DA_EuropeanPayouts 37 pockets, 15 rules

Every one of the 157 generated positions was checked against the payout table on
three counts: that a payout row exists for its bet type, that the number of pockets
it covers equals what that row declares, and that the resulting house edge is
1/37 to within 0.0001.

LogHasard: Warning: GameMode BeginPlay - phase EHasardRoundPhase::Betting, betting window: 30.0s
LogHasard: Warning: Payout audit: Straight Up pays 35:1, covers 1/37, true odds 36.00:1, edge 2.70%
LogHasard: Warning: Payout audit: Split pays 17:1, covers 2/37, true odds 17.50:1, edge 2.70%
LogHasard: Warning: Payout audit: Street pays 11:1, covers 3/37, true odds 11.33:1, edge 2.70%
LogHasard: Warning: Payout audit: Corner pays 8:1, covers 4/37, true odds 8.25:1, edge 2.70%
LogHasard: Warning: Payout audit: Six Line pays 5:1, covers 6/37, true odds 5.17:1, edge 2.70%
LogHasard: Warning: Payout audit: Column pays 2:1, covers 12/37, true odds 2.08:1, edge 2.70%
LogHasard: Warning: Payout audit: Dozen pays 2:1, covers 12/37, true odds 2.08:1, edge 2.70%
LogHasard: Warning: Payout audit: Red pays 1:1, covers 18/37, true odds 1.06:1, edge 2.70%
LogHasard: Warning: Payout audit: Black pays 1:1, covers 18/37, true odds 1.06:1, edge 2.70%
LogHasard: Warning: Payout audit: Even pays 1:1, covers 18/37, true odds 1.06:1, edge 2.70%
LogHasard: Warning: Payout audit: Odd pays 1:1, covers 18/37, true odds 1.06:1, edge 2.70%
LogHasard: Warning: Payout audit: Low 1-18 pays 1:1, covers 18/37, true odds 1.06:1, edge 2.70%
LogHasard: Warning: Payout audit: High 19-36 pays 1:1, covers 18/37, true odds 1.06:1, edge 2.70%
LogHasard: Warning: Payout audit: Trio pays 11:1, covers 3/37, true odds 11.33:1, edge 2.70%
LogHasard: Warning: Payout audit: Basket pays 8:1, covers 4/37, true odds 8.25:1, edge 2.70%
LogHasard: Display: Straight Up: 37 positions
LogHasard: Display: Split: 60 positions
LogHasard: Display: Street: 12 positions
LogHasard: Display: Trio: 2 positions
LogHasard: Display: Corner: 22 positions
LogHasard: Display: Basket: 1 positions
LogHasard: Display: Six Line: 11 positions
LogHasard: Display: Column: 3 positions
LogHasard: Display: Dozen: 3 positions
LogHasard: Display: Low 1-18: 1 positions
LogHasard: Display: Even: 1 positions
LogHasard: Display: Red: 1 positions
LogHasard: Display: Black: 1 positions
LogHasard: Display: Odd: 1 positions
LogHasard: Display: High 19-36: 1 positions
LogHasard: Display: 157 positions generated, 0 failing the edge check

---

Position counts, and where each number comes from. NC = 12 number columns,
NR = 3 number rows.

| Bet | Count | Derivation |
|---|---|---|
| Straight up | 37 | 1 zero + NC x NR |
| Split | 60 | NC x (NR-1) = 24 within a column, (NC-1) x NR = 33 across, + 3 zero pairings |
| Street | 12 | NC |
| Trio | 2 | 0/1/2 and 0/2/3 — the only two ways three numbers include zero |
| Corner | 22 | (NC-1) x (NR-1) |
| Basket | 1 | 0/1/2/3 |
| Six line | 11 | NC-1 |
| Column | 3 | NR |
| Dozen | 3 | NC x NR / 12 |
| Even-money | 6 | Red, Black, Even, Odd, 1-18, 19-36 |
| **Total** | **157** | |

The type ordering in the summary comes from a `TMap` and is not guaranteed. Only
the counts and the total are load-bearing.

Two things this audit does **not** prove, and neither is hidden:

The 157 positions are data, not yet clickable. Nothing has resolved a trace hit to
a `PositionId` at this point, so a position could be generated at the wrong
`ChipLocation` and this audit would still pass — it checks covered numbers and
payouts, not geometry. Geometry gets checked when the felt is drawn from the same
numbers and the two are seen to line up.

`Trio` and `Basket` still have no settlement path. `PlaceBet` refuses both, because
`FHasardBet::PrimaryNumber` cannot express a bet that includes zero. They appear
here with correct covered numbers and a correct 2.70% edge, and they become
playable when a bet stores a `PositionId` instead of a number.
