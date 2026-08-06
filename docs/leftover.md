# Hasard — handoff note

As of 2026-08-06. Setup is complete, and the project is defined: Hasard is a
roulette table prototype — wheel, ball, pocket — built flat-screen first with VR
layered on afterwards, carrying responsible-gaming mechanics as its differentiator.
It is a portfolio piece aimed at Loto-Québec. See Settled decisions below.

## What exists

| Thing | State |
|---|---|
| Engine | Unreal Engine 5.8.1, installed via Epic Games Launcher |
| Toolchain | Visual Studio Community 2026 (v18), Game development with C++ workload installed |
| Project | `C:\UE5\Hasard` — Blank C++ template, Desktop, Scalable quality preset |
| Repo | `https://github.com/picardbuilds/Hasard` — private, `main` pushed |
| Git LFS | Configured and verified. `.gitattributes` is the root commit |
| Content/ | Empty. Zero assets exist yet |
| Vault | Path Index and Product-Dev - Brain updated (commit `368572b`) |

`C:\UE5` is a projects root, not a repo. `C:\UE5\Hasard` is the repo and the Unreal
project root. Not inside any cloud-synced folder — deliberate.

## Conventions

- `.gitattributes` must be committed before staging any assets. LFS only applies to
  files staged after the rules exist.
- Packaged builds and demo video go to Google Drive, never into this repo.
- Package output goes outside `C:\UE5\Hasard` — each build is several GB and Unreal
  does not clean up old ones.
- `.gitignore` covers `.slnx` as well as `.sln` (Visual Studio 18 uses the new format).
- The vault is canonical for status and business decisions. `CLAUDE.md` in the repo is
  canonical for code-level decisions.

## Machine constraints

Disk is the binding constraint, not CPU or GPU.

- C: drive was at 6 GB free on 2026-07-31; cleared to 18.5 GB.
- Riot Games (~38 GB) is the largest remaining reclaim. Not yet uninstalled.
- Project caches `.vs` + `Intermediate` + `Binaries` + `DerivedDataCache` are ~5 GB
  and regenerate. Safe to delete when the editor and Visual Studio are closed; costs
  one full rebuild including shader compile (~15-30 min).
- `DerivedDataCache` grows with every asset imported. Watch it.
- Free space check: `Get-PSDrive C | Select-Object @{n='FreeGB';e={[math]::Round($_.Free/1GB,1)}}`

Viewport performance on the XPS was tested on a blank scene and was acceptable.
Decision: develop and package on Windows, single machine. The Mac is not in the loop.
This decision should be revisited once a scene has real geometry and lighting.

## Corrections carried forward

- GitHub LFS free tier is 10 GiB storage and 10 GiB bandwidth, not 1 GiB. An earlier
  note in this project said 1 GiB — that figure is stale.
- GitHub Pro (including via Student Developer Pack) does not raise the LFS allowance.
  Only Team and Enterprise do. So the account choice was not an LFS decision.
- Folder sizes reported by summing file `Length` are unreliable for cloud-backed
  folders — placeholder stubs report remote size, not disk usage. Free space is the
  only trustworthy number.

## Settled decisions

### 2026-08-06 — What the project is

A roulette table prototype. The core mechanic is the wheel and the ball settling
into a pocket.

Built flat-screen first; VR is layered on afterwards. Roughly 80% of the work —
rules, interaction, data, UI — is VR-agnostic, so VR is a layer over a working
game rather than a foundation under an unwritten one. Building flat-screen first
keeps that 80% testable without a headset on.

It is a portfolio piece aimed at Loto-Québec, for a co-op / internship application.

The intended differentiator is responsible-gaming mechanics built into the design
rather than bolted on afterwards:

- Session timer with an in-world reality check.
- Running spend visible rather than hidden.
- Honest display of the true odds and the house edge.
- Deliberate avoidance of known dark patterns — no near-miss animations, no
  losses-disguised-as-wins.

Scope discipline: one interesting mechanic, packaged early.

### 2026-08-06 — Renamed `First` → `Hasard`

`First` was a placeholder. `hasard` is the French word for chance, and *jeux de
hasard* is the regulatory term for games of chance; the name states what the
project is in the language of the intended audience. Done while `Content/` was
still empty, which is the cheap moment — the `.uproject`, `Source/` folder,
`.Build.cs`, module header/cpp, and the `HASARD_API` macro all carry the name.

## Open decisions

1. **Commit author email.** Both commits are authored `joseph.picard@picanne.com`.
   Kept deliberately. Revisit only if the repo goes public.
2. **Unreal MCP.** UE 5.8 ships an experimental first-party MCP plugin that lets an
   agent drive the editor. Not enabled. Two reasons to hold off: it is experimental,
   and agent-driven asset edits re-save `.uasset` files, each re-push storing the whole
   file again against the LFS quota.
3. **Spin result: predetermined, or emergent from ball physics.** Undecided, and it
   decides whether the physics drives the outcome or merely presents it — so decide
   before the wheel is built.
   - *Predetermined:* pick the winning pocket first, then animate the wheel and ball
     to land there. Guarantees a fair, uniform distribution across all 37 pockets,
     and is frame-rate independent.
   - *Real physics:* let the result emerge from the simulated ball. Genuinely
     emergent, but fairness across 37 pockets is hard to guarantee, and the same
     spin can land differently at different frame rates.

   Note that the responsible-gaming stance above raises the stakes on this one: an
   honest-odds display is a claim about the distribution, and only the predetermined
   approach lets you state that distribution with confidence.

## Learning approach

Epic's own docs, read with the editor open, applying as you go:

- Programming with C++ in Unreal Engine —
  https://dev.epicgames.com/documentation/unreal-engine/programming-with-cplusplus-in-unreal-engine
  Read in order: Reflection System, then Gameplay Framework, then Containers.
- Epic C++ Coding Standard —
  https://dev.epicgames.com/documentation/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine
- Tom Looman's C++ guide — https://tomlooman.com/unreal-engine-cpp-guide/
  Not Epic, but current. Covers conventions Epic's docs assume, e.g. `TObjectPtr<T>`
  in header UPROPERTYs.

Much YouTube Unreal content is UE 4.27-era and wrong about current APIs.

The test that matters: can you explain why the code in the repo works? If not, slow
down. An interviewer will ask.

## Next session

1. Read the reflection system docs with the editor open. Make a class, add a
   `UPROPERTY`, watch it appear in the details panel.
2. Decide what the project is.
3. Package a test build in week 2, not week 8. Finding out what breaks early is the
   whole point.
