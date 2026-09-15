# Hasard

A single-zero roulette table written in C++ on Unreal Engine 5.8, whose subject is
responsible-gaming design rather than roulette.

**One-minute demo:** https://youtu.be/nbdHmSVrK30

---

## What this is

The wheel, the ball and the pocket are the vehicle. The deliverable is the design
stance: every decision in this repository is checked against one question —

> Does this respect the player, or exploit them?

When the two conflict, respecting the player wins, even where that makes the game
feel less exciting. A thrilling roulette table demonstrates nothing; the industry
solved that problem thoroughly and exploitatively. Giving up the cheap thrill in
order to keep the player informed and free to leave is the hard version, and that
tradeoff is the point of the piece.

Built flat-screen first, with VR layered on afterwards — roughly 80% of the work
(rules, data, interaction, UI) is VR-agnostic.

## What is built

| | |
|---|---|
| Engine | Unreal Engine 5.8, C++ (Blueprints for wiring only) |
| Source | 33 C++ files under `Source/Hasard/` — wheel, felt, chips, bets, payouts, bankroll, save, HUD, pawn, controller, game mode |
| Content | One map, 7 Blueprints, 3 data assets, 9 input assets, 3 materials, one HUD widget |
| Table | 157 bet positions generated at runtime from 15 bet-type rules over a data asset |
| Wheel | 37 pockets in real physical rim order, a coloured plate and a numeral printed under each |
| Round | Betting window → spin → settlement, with the result drawn at the start of the spin |
| Persistence | One save slot; a record written on every money movement |

## The responsible-gaming requirements

These are load-bearing, not polish. They were built alongside the mechanic, not after it.

- **True odds and the house edge are always visible and honest.** 2.70% on every bet
  type, stated plainly, with nothing to hunt for.
- **Running spend lives in the play space**, not in a menu. If you have to go looking
  for what you have spent, it is hidden.
- **The session timer runs on wall clock**, never world time. A clock that pauses when
  the game pauses understates the session and is a hidden total by another name, so
  `AHasardPlayerState` measures with `FPlatformTime::Seconds()` rather than
  `UWorld::GetTimeSeconds()`. Session length is a claim about the player, so no in-game
  state may alter it.
- **An in-world reality check**, present in the scene the player is actually in, rather
  than a notification that can be dismissed unread.
- **One save slot, no profile picker.** An unverified profile picker is a spend-reset
  button with a friendly name. In a regulated product, profiles are identity-verified;
  here they would not be.

## What the project refuses to implement

Not tunable, not negotiable, and every one of them routine in commercial gambling
products — which is exactly why refusing them is the thing worth showing.

- Near-miss animation, or any feedback that makes a loss feel close. The ball landing
  one pocket over is a loss and reads as a loss.
- Losses disguised as wins — no celebratory audio or visuals on a spin that nets a loss.
- Hidden or hard-to-find spend totals.
- Obscured, omitted or misstated odds.
- Dark patterns that extend a session: fake urgency, "one more spin" nudges, interrupted
  or delayed exit flows.

## Evidence

The table audits itself rather than asserting that it is correct.

**Startup audit** — every one of the 157 generated positions is checked against the
payout table on three counts: that a payout row exists for its bet type, that the number
of pockets it covers matches what that row declares, and that the resulting house edge is
1/37 to within 0.0001.

```
LogHasard: Payout audit: Straight Up pays 35:1, covers 1/37, true odds 36.00:1, edge 2.70%
LogHasard: Payout audit: Split      pays 17:1, covers 2/37, true odds 17.50:1, edge 2.70%
...
LogHasard: 157 positions generated, 0 failing the edge check
```

Position counts are derived, not hand-listed: 37 straight up, 60 splits, 12 streets,
2 trios, 22 corners, 1 basket, 11 six-lines, 3 columns, 3 dozens, 6 even-money.
Full derivation in [`docs/layout-audit.md`](docs/layout-audit.md).

**Distribution** — 370,000 spins measured against the draw, widest per-pocket deviation
2.3% across 37 pockets. [`docs/distribution-test.md`](docs/distribution-test.md).

**Geometry** — the felt, its 49 labels and the wheel's numerals are generated from the
same measurements the hit test reads, so the picture and the click cannot drift apart.
Nothing is modelled and hand-aligned, and nothing goes through LFS.

## A note on the spin

`DetermineWinningPocket()` draws a pocket with `FMath::RandRange` over a uniform range at
the *start* of the spin; the animation moves toward a known pocket and does not produce
the result.

This is a responsible-gaming decision as much as a technical one. The HUD claims every
bet costs 2.70% of what you stake — a claim about the distribution over 37 pockets. With
a simulated ball that distribution depends on the physics substep rate and the collision
solver: it could not be stated, let alone evidenced, and steering it by nudging initial
conditions would be far harder to audit than an animation.

## Who wrote this

Every line of C++ in `Source/` is typed by hand. No agent writes source in this
repository — it is a rule in the project's engineering brief, with one recorded
exception logged in the decision log rather than quietly absorbed.

The reason is the same one the repository exists for: the value of the piece is being
able to sit across from an interviewer and explain why `SessionStartRealTime` is a
`double`, why chips are actors and printed boxes are instances, and why the felt has no
line geometry. Code that was not typed by hand is code that cannot be defended in that
room.

## Building

```
git clone https://github.com/picardbuilds/Hasard.git
```

The repository root **is** the Unreal project root — `Hasard.uproject` sits beside this
file, with no wrapper directory. Right-click the `.uproject`, generate project files, and
build from Visual Studio. `EngineAssociation` is a machine-local GUID, so the project will
ask you to pick an engine version on any other machine; pick the installed UE 5.8 and let
it re-associate.

Assets go through Git LFS, configured from the root commit. Packaged builds and the demo
video are deliberately not in this repository — they are multi-gigabyte and regenerable.

## Status

Playable, packaged and tagged. `v0.1-playable`, `v0.2-full-table`.

Still open, and named rather than hidden: a pause menu that is one action from leaving is
designed but not yet built, so the fourth responsible-gaming pillar is a stated intention
rather than a shipped feature; in-progress bets are not persisted, because a bet stores an
index into the generated position array and writing one to disk would make the order of
the twelve generation rules part of the save format; and the pockets are rectangles where
a real rim is wedges.

## Documentation

Two self-contained HTML courses live in `docs/` — no build step, open them in a browser.
They are the build-along record of how the project was made, one complete file per step.

- `docs/guide-2-building-hasard.html` — the build-along that produces this project
- `docs/guide-3-completing-the-table.html` — the full table
- `docs/guide-4-spinning-the-wheel.html` — the spin

## Licence

All rights reserved. Portfolio piece — readable, not licensed for reuse.
