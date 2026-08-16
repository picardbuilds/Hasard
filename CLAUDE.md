# CLAUDE.md - Hasard

Engineering brief for this repo. Read before touching code.

## What this is

Hasard is a roulette prototype whose point is to demonstrate responsible gaming
design. That is the deliverable. The wheel, the ball, and the pocket are the
vehicle for it.

It is **not** a gambling simulator that happens to have limits bolted on. The
responsible-gaming mechanics are the reason this project exists - they are the
subject, not a feature on the list. Read the Governing principle below before
anything else in this file.

Built flat-screen first, with VR layered on afterwards. Built in C++ on Unreal
Engine 5, to be shown and read by other people. It is a personal portfolio piece
aimed at Loto-Québec - a regulated operator with a social mandate, which is
precisely why the stance below is the point of the piece rather than a
nice-to-have attached to it.

This is **not** a picardbuilds product. It shares no code, no infrastructure, and
no conventions with any picardbuilds repo beyond this document's shape. Do not
wire it into picardbuilds tooling, accounts, or hosting.

## Governing principle

Every design and engineering decision in this repo is checked against one
question:

> **Does this respect the player, or exploit them?**

When the two conflict, respecting the player wins - even at the cost of the thing
feeling less exciting. That is not a reluctant compromise, it is the exercise. A
roulette table that felt thrilling would demonstrate nothing; the industry has
solved that problem thoroughly and exploitatively. Giving up the cheap thrill in
order to keep the player informed and free to leave is the hard version, and
**that tradeoff is the portfolio piece.**

So "it feels flat without it" is not an argument for adding something the rules
below prohibit. If a mechanic is only compelling because it pressures the player,
its absence is the demonstration working.

**Any session working on this repo - human or agent - must read this section,
along with the responsible-gaming rules under Architecture principles and
Anti-patterns, before adding gameplay features.** If a feature request conflicts
with them, raise the conflict and get an explicit decision. Do not silently
implement it, and do not quietly soften it until it fits.

## Who writes the code

**Joe types every line of C++ in `Source/`. Agents never edit it. There is no
exception to this, including "just this once", "it is only a typo", and "you
asked me to add the feature".**

This is not a preference about workflow. It is what the project is for. Hasard is
a portfolio piece for an internship, and its value is entirely in being able to
sit across from an interviewer and explain why `SessionStartRealTime` is a
`double`, why chips are actors and printed boxes are instances, why the felt has
no line geometry. **Code that was not typed by hand is code that cannot be
defended in that room**, and a repo full of it is worth less than half the size
written slowly.

Guide 2 and guide 3 exist because of this. They are build-along courses whose
Build panels show **complete files** precisely so they can be typed rather than
merged by eye. An agent that edits the source has skipped the deliverable and
produced the byproduct.

### What an agent produces instead

| Asked for | The agent's output |
|---|---|
| A feature | A guide module, or a build panel, that Joe types. Nothing in `Source/` changes. |
| A fix to existing code | The defect, a `diff` fence with the exact replacement, and the line numbers. Joe applies it. |
| A review of pasted code | Defects only, per the `hasard-guide-authoring` skill. |
| Docs, notes, decisions | Direct edits are fine - `docs/`, `CLAUDE.md`, `toUpdate.md` are not what is being learned. |

If an agent needs working code to reason about - to check that an audit passes, to
count boxes, to verify arithmetic - it writes a throwaway script **outside the
repo**, in its own scratch directory. It does not edit `Source/` and revert.

**Verifying a change is not a licence to make it.** The temptation is to edit the
file, confirm it compiles or that the numbers work, and then hand over a diff. The
edit is the violation regardless of what happens afterwards; if the working tree
was touched, the line was crossed.

If something genuinely cannot be expressed as a panel to type - it is too large, or
it is a mechanical rename across forty files - **say so and ask.** Getting explicit
permission takes one message. Assuming it is the failure this section exists to
prevent.

## Repo layout

The repo root **is** the Unreal project root. `Hasard.uproject` sits beside this
file. There is no wrapper directory - cloning this repo gives you a project the
launcher can open directly.

| Path | What it is |
|---|---|
| `Hasard.uproject` | Project descriptor. Module `Hasard` (Runtime), plugin `ModelingToolsEditorMode` |
| `Source/Hasard/` | C++ game module |
| `Source/*.Target.cs` | Build targets (game, editor) |
| `Config/` | `DefaultEngine.ini`, `DefaultGame.ini`, `DefaultInput.ini`, `DefaultEditor.ini` |
| `Content/` | Assets. Empty as of 2026-07-31 - see decision log |
| `docs/` | Handoff note, `CONVENTIONS.md`, `epic-standard-audit.md`, and the two learning guides - see below |

## Skills - load these at the start of every session

Two skills exist for this repo specifically. **Any new session working here loads both
up front, without being asked and without asking first.** They are not on-demand tools
to reach for once a problem appears.

| Skill | What it carries |
|---|---|
| `ue5-code-review` | Epic's official C++ coding standard and engine-usage conventions, targeted at UE 5.8. Naming, formatting, const, encapsulation, `UPROPERTY`/`UFUNCTION`, GC and lifetimes, actor lifecycle, gameplay-framework ownership, Enhanced Input, and what dates a UE4-era tutorial. |
| `ue5-guide-authoring` | The `guide-N-*.html` course format and its rules: a complete file at every Build step, every editor action spelled out, each module ending in something observable, quiz answers distributed across all four positions. |

**Why up front rather than on request.** Both skills are preventive. `ue5-code-review`
is meant to be read *before* C++ is written here, so it complies on the first pass -
loading it when a review is finally requested means the violations already exist and the
session is correcting itself. `ue5-guide-authoring` is the same for guide work: the
full-file rule and the editor-step checklist shape a module as it is written; applied
afterwards they are a rewrite. A session that waits to be asked has already spent the
value.

So: read both at session start. Do not ask whether to load them, do not wait for a
prompt containing the word "review", and do not treat "the user did not mention the
guides" as a reason to skip the second one - a code change frequently means a guide
module needs correcting in the same commit.

The audit in `docs/epic-standard-audit.md` (2026-08-08) is the current gap list between
this repo and Epic's standard, including the places where `docs/CONVENTIONS.md` itself
contradicts Epic. Read it alongside `CONVENTIONS.md`; where the two disagree, the audit
is the newer judgement and Epic is the authority behind it.

## Learning guides

Two self-contained HTML courses live in `docs/`. No build step, no dependencies -
open them in a browser.

| File | What it is |
|---|---|
| `docs/guide-1-unreal-cpp-foundations.html` | Unreal C++ concepts, adapted from Tom Looman's Complete Guide. Pointers, reflection, interfaces, delegates, containers, GC. **Reference - complete.** |
| `docs/guide-2-building-hasard.html` | The build-along course that produces **this project**. 15 modules, each ending in a Build step with real code for this repo. **Active.** |

**Guide 2 is the project plan.** Its Build steps are the intended build order, and
each one ends with a commit message. If you are picking up work on this repo,
read the module after the last one in the commit history and continue from there.

Progress as of 2026-08-08: **Modules 0-7 complete** - wheel actor with component
hierarchy, exposed tuning properties, GameMode/PlayerController/PlayerState with
round phases, actor lifecycle logging, a wall-clock session timer with reality
check, the table camera pawn, Enhanced Input (look, place bet, spin), and the
betting component with `HasardTypes.h` holding the shared enum and struct.
Module 8 (collision, tracing, the clickable felt) is next.

`CameraDistance` is applied in `AHasardPlayerPawn::OnConstruction`, not the
constructor: Blueprint writes its property overrides after the C++ constructor
runs, so a constructor-set arm length ignores the Blueprint value entirely.

Both guides carry corrections found while building. Guide 1's input module is
explicitly flagged as legacy - use Enhanced Input, per Guide 2 Module 6.

**There is exactly one copy of each guide, and it lives here.** Edit these files
in place. Do not create `-v2`, `-final`, `-updated` or dated duplicates, and do
not work on a copy elsewhere and forget to bring it back. When a module surfaces
something the guide got wrong or left out, fix the guide in this folder and
commit it alongside the code change that revealed it.

## Stack

- Unreal Engine 5, C++ (not Blueprint-only)
- Visual Studio for compilation; the `.sln` is generated, never committed
- Git + Git LFS for version control

`EngineAssociation` in `Hasard.uproject` is a machine-local GUID, not a version
string. On any other machine the project will prompt you to pick an engine
version. That is expected for a launcher-registered project - pick the installed
UE5 and let it re-associate.

## Architecture principles

### Responsible gaming - build these in

Load-bearing requirements, not polish. They do not get deferred until after the
mechanic works, because they are what the mechanic is for.

- **True odds and the house edge are always visible and honest.** A player can see
  what the game takes, stated plainly, without hunting for it.
- **Running spend is visible in the play space**, not buried in a menu. If the
  player has to go looking for what they have spent, it is hidden.
- **A session timer with an in-world reality check** - present in the scene the
  player is actually in, not a notification that can be dismissed unread. It is
  measured on **wall clock**, never world time: a clock that pauses when the game
  pauses understates the session and is a hidden total by another name.
- **Leaving is always one clear action away.** No confirmation gauntlet, no
  parting offer, no flow that makes stopping more effort than continuing.

### Engineering

- Gameplay logic lives in C++ under `Source/Hasard/`. Blueprints are for wiring
  and designer-facing tweaks, not for logic that belongs in a class.
- Anything generated is disposable. `Binaries/`, `Intermediate/`,
  `DerivedDataCache/`, `Saved/`, `.vs/` can all be deleted and regenerated by
  right-clicking the `.uproject` and regenerating project files.
- A fresh clone plus a rebuild must produce a working editor session. If it does
  not, something generated got committed or something needed got ignored.

## Assets and Git LFS

Binary assets go through LFS. Tracked extensions are declared in
`.gitattributes`: `uasset umap fbx blend png jpg tga psd exr wav mp3 ogg mp4
ttf otf`.

**LFS only applies to files staged after the tracking rules were committed.** The
`.gitattributes` and `.gitignore` pair is the root commit of this repo
(2026-07-31) precisely so that every asset ever added lands in LFS. If you add a
new binary extension, commit the `.gitattributes` change **before** staging files
of that type.

Verify with `git lfs ls-files` - if a binary asset is missing from that list, it
went in as a normal blob and needs to be rewritten out.

## What does not belong in this repo

- **Packaged builds.** Shipping/Development packages go to Google Drive. Never
  commit a build.
- **The demo video.** Google Drive. Never commit it.
- Generated build output: `Binaries/`, `Intermediate/`, `DerivedDataCache/`,
  `Saved/`, `Build/`.
- IDE state: `.vs/`, `.idea/`, `*.sln`, `*.slnx`, `*.suo`, `UpgradeLog.htm`.
- Compiled binaries: `*.dll`, `*.exe`, `*.lib`, `*.o`, `*.obj`, `*.pdb`.

The working tree ran about 4.9 GB on 2026-07-31 while the tracked content was
under 40 KB. Effectively all of that mass is regenerable. If a commit is
suddenly large, the ignore rules were bypassed - do not push it, fix it.

## Commit messages

**Subject line: 50 characters, hard limit.** Imperative mood, capitalised, no
trailing period. `Add betting component and shared types`, not
`Added the betting component and the shared Hasard types.`

Anything that needs explaining goes in the body, never the subject. Use a second
`-m`, wrapped at 72 columns, and write *why* rather than *what* - the diff
already says what changed.

```
git commit -m "Move camera distance to OnConstruction" -m "Set in the
constructor, the Blueprint override never reached it: Blueprint writes its
property defaults after the C++ constructor has run."
```

Guide 2's Build steps each end with a suggested commit. Those subjects are
written to fit this limit; if one does not, shorten it rather than copying it
verbatim, and fix the guide in the same commit.

## Anti-patterns - do not do this

### Responsible gaming - never implement

Not tunable, not negotiable. Do not implement these, and do not implement a
softened version "just to see how it feels":

- **Near-miss animations**, or any feedback that makes a loss feel close. The ball
  landing one pocket over is a loss and must read as a loss.
- **Losses disguised as wins.** No celebratory audio or visuals on a spin that
  nets a loss.
- **Hidden or hard-to-find spend totals.** If it takes a menu dive, it is hidden.
- **Obscured, omitted, or misstated odds.**
- **Dark patterns that extend a session** - fake urgency, "one more spin" nudges,
  interrupted or delayed exit flows.
- **Anything that misrepresents the house edge**, including by omission, or by
  framing that makes it look smaller than it is.

Every one of these is routine in commercial gambling products. That is exactly why
refusing them is the thing worth showing.

### Repo hygiene

- Do not commit `.sln`/`.slnx`. Visual Studio regenerates them, and `*.sln` does
  not match `*.slnx` - both patterns are in `.gitignore` for that reason.
- Do not add an asset extension to `.gitattributes` in the same commit as files
  using it. Rules first, then assets.
- Do not place this repo inside a cloud-synced folder (Google Drive, OneDrive,
  Dropbox). The sync client races Git on `.git/` internals and corrupts the repo.
  It lives at `C:\UE5\Hasard`, outside every sync root, deliberately.
- Do not commit with the editor open mid-save. Close Unreal before staging
  assets so `.uasset` files are not captured half-written.
- Do not let an init tool overwrite this file. `npx ruflo init` and similar
  scaffolders write their own `CLAUDE.md`. Commit before running any of them and
  keep this hand-written brief authoritative.

### Sessions

- **Do not edit anything under `Source/`.** See Who writes the code. An agent that
  has opened a `.h` or `.cpp` with a write tool has already got it wrong, whatever
  the diff looks like afterwards.
- Do not "helpfully" apply a fix you have just correctly identified. Report it with
  a diff fence and stop. The report is the whole job.
- Do not edit source to test a theory, intending to revert. Use a scratch copy
  outside the repo.
- Do not start writing C++ here before loading `ue5-code-review`, and do not start
  editing a guide before loading `ue5-guide-authoring`. See Skills above.
- Do not ask permission to load them. Asking each time is the failure this rule
  exists to remove.

## Decision log

### 2026-07-31 - Repo initialized with LFS from the first commit
`git init -b main` at the Unreal project root. `.gitattributes` and `.gitignore`
committed **alone** as the root commit, before the `.uproject`, `Source/`, or
`Config/` were staged, so LFS tracking applies to every asset from the start.

### 2026-07-31 - Location chosen outside any cloud sync root
`C:\UE5\Hasard`. Verified not to sit under OneDrive (`C:\Users\Joepi\OneDrive`),
Google Drive, or Dropbox, and not to be a junction or symlink into one.

### 2026-07-31 - Content/ intentionally empty at first commit
Blank C++ template, no starter content. `git lfs ls-files` correctly returns
nothing until the first asset is added. That is not a misconfiguration - the
tracking rules are already live and waiting.

### 2026-07-31 - Builds and demo video live on Google Drive
Kept out of Git entirely rather than managed through LFS. Packaged Unreal builds
are multi-gigabyte and regenerable; paying LFS bandwidth for them is waste.

### 2026-08-06 - Renamed First -> Hasard
`First` was a placeholder. `hasard` is the French word for chance, and *jeux de
hasard* is the regulatory term for games of chance - the name states what the
project is in the language of the intended audience. Done while `Content/` was
still empty, which is the cheap moment: the name is carried by `Hasard.uproject`,
the `Source/Hasard/` module folder, `Hasard.Build.cs`, the module header/cpp, and
the `HASARD_API` macro. Renaming once assets reference `/Script/Hasard` is
materially harder.

### 2026-08-06 - Project scope defined
Roulette prototype, flat-screen first with VR layered on afterwards,
responsible-gaming mechanics as the differentiator, aimed at Loto-Québec. The full
entry - core mechanic, the VR-agnostic split, the responsible-gaming list, scope
discipline - is in `docs/leftover.md` under Settled decisions. Not duplicated here.

### 2026-08-06 - Responsible gaming is the governing principle
Responsible gaming is the project's organizing principle, not a feature. The
roulette table is the vehicle for demonstrating it. The target is Loto-Québec, a
regulated operator with a social mandate: a merely well-built roulette prototype
demonstrates nothing they cannot already buy, so refusing the exploitative
patterns that are standard in commercial gambling products is the differentiator.
The accepted cost is that the result will be less exciting than an exploitative
equivalent - deliberate, and the point rather than a shortfall to fix later.

Consequence for an already-open decision: the spin result - predetermined, or
emergent from real ball physics (open decision 3 in `docs/leftover.md`) - is now
partly a responsible-gaming question rather than a purely technical one, because
an honest-odds display is a claim about the distribution.

### 2026-08-06 - Learning guides committed to `docs/`
Two HTML courses now live in the repo rather than outside it: `guide-1` for
Unreal C++ concepts, `guide-2` as the build-along that produces this project.
They are committed here so any session - human or agent - can find the intended
build order without being told, and so corrections found while building are
versioned alongside the code that revealed them.

Guide 2's Build steps are the project plan. Each ends with a commit message, so
the commit history and the guide stay in step: the module to work on next is the
one after the last module named in the log.

**One copy only.** Both files are edited in place. Versioned duplicates
(`-v2`, `-final`, dated copies) are the failure mode this decision exists to
prevent - two diverging guides is worse than no guide, because neither can be
trusted. They are plain HTML, so they diff as text and are deliberately not
LFS-tracked.

### 2026-08-06 - Coding conventions written down in `docs/CONVENTIONS.md`
Naming, `UPROPERTY` specifier choices, header/source split, logging, the
editor/Visual Studio build cycle, and commit-message limits. Separate from this
file because this brief is about what the project is and refuses to do;
CONVENTIONS is about how the code is written. Commit subjects are capped at 50
characters, bodies at two sentences - anything longer belongs in `docs/`.

### 2026-08-06 - Session clock uses wall time, not world time
`AHasardPlayerState` measures session length with `FPlatformTime::Seconds()`,
not `UWorld::GetTimeSeconds()`. World time stops while the game is paused and
scales with time dilation, so a player who paused for ten minutes would be told
they had been playing ten minutes less, and the reality check would be pushed
back by the same amount.

This is the responsible-gaming rule, not a correctness nicety. A clock that
quietly stops counting understates the cost without ever stating anything false -
the same shape as a hidden spend total, which the Anti-patterns section already
prohibits. **Session length is a claim about the player, so no in-game state may
alter it.**

The distinction is narrow and worth stating: gameplay durations *should* use
world time. The six-second spin is part of the simulation and should slow down
when the simulation does. Only the figures reported *to* the player about their
own behaviour are held to wall clock.

### 2026-08-08 - Two repo skills, loaded at session start rather than on request
`ue5-code-review` carries Epic's official coding standard and engine-usage conventions
for UE 5.8; `ue5-guide-authoring` carries the guide format and its completeness rules.
Both are loaded at the start of any session on this repo, without being asked.

The alternative - surfacing them when a request happens to mention "review" or "guide" -
was rejected because both skills are preventive. Code written before the standard is
loaded has to be corrected afterwards, and a guide module written before the full-file
rule is loaded has to be rewritten. The cost of loading them unnecessarily is a few
thousand tokens; the cost of loading them late is the work already being wrong.

Written down here rather than left to habit because a fresh session has no memory of the
previous one, and the previous one is where the reason lives.

### 2026-08-14 - Agents do not write the C++; the rule is now in the brief
Written down after an agent implemented guide 3 module 7 - `BoxSize`, the printed
felt, the box audit - directly in `Source/` instead of authoring the module for Joe
to type. Six files, and they landed in commit `d723d90` alongside hand-written
module 6 work.

Not reverted. The code is correct, the guide module was generated from it and
verified, and unpicking a mixed commit costs more than it returns. The cost paid is
that module 7 is the one module Joe did not type, and that is exactly the cost this
entry exists to stop recurring.

The failure was not malice or misunderstanding of the request - the request was
"make the table look real", which sounds like a request for code. It is that
nothing in this file said otherwise. The previous version told a session to read
Epic's standard *before writing C++ here*, which reads as permission. **Who writes
the code** now says the opposite, in the imperative, above the layout section, with
a table of what an agent produces instead.

Two smaller traps are named there because both are reasonable-sounding: editing a
file to verify a change before handing over the diff, and applying a fix that was
just correctly diagnosed. Both end with a modified working tree, which is the line.

### 2026-08-08 - Epic's standard is the authority over `docs/CONVENTIONS.md`
`docs/epic-standard-audit.md` records where this repo and its own conventions diverge
from Epic. Where `CONVENTIONS.md` is *stricter* than Epic it stands - the `Hasard` type
prefix, the three comment triggers, one shared log category, the batch build workflow.
Where it is *contrary* to Epic, Epic wins and the convention is the thing to fix.

The live case: `CONVENTIONS.md` §3 makes members `protected` by default, specifically to
avoid `meta=(AllowPrivateAccess="true")`. Epic requires private by default with protected
accessors, and treats that meta tag as the intended cost. Unresolved as of this entry -
it changes every header, so it is decided once at the convention level rather than
per file.
