# Hasard — Conventions

Project: `C:\UE5\Hasard` · Module prefix `HASARD_API` · Repo `joehockey04/Hasard` (private, Git LFS)

Rules derived from the code already written (Modules 1–6). Follow these instead of re-deciding each time.

**Spawn the Blueprint, never the C++ class.** Any class picker — Default Pawn Class, Default GameMode, Player Controller Class — shows both `HasardPlayerPawn` and `BP_PlayerPawn`. The C++ class is the declaration; the Blueprint holds the assigned assets. Pick the C++ one and every `EditDefaultsOnly` property is `None` at runtime, so the feature does nothing and reports nothing.

---

## 1. Naming

| Thing | Rule | Example |
|---|---|---|
| Actor class | `A` + `Hasard` + noun | `AHasardWheel` |
| Framework class | `A` + `Hasard` + role | `AHasardGameMode`, `AHasardPlayerState` |
| Enum | `E` + `Hasard` + noun, `enum class : uint8` | `EHasardRoundPhase` |
| Struct | `F` + `Hasard` + noun | `FHasardBet` |
| Blueprint | `BP_` + C++ class without `A` | `BP_HasardGameMode` |
| Level | `L_` + purpose | `L_RouletteTest` |
| Shared header | `Hasard` + noun + `.h` | `HasardTypes.h` |
| Bool | `b` prefix | `bIsSpinning` |
| Component member | noun, no prefix | `WheelRoot`, `WheelMesh` |

Every gameplay type carries the `Hasard` prefix. Rename assets **in the editor** (F2), never in Explorer, then right-click the folder → **Fix Up Redirectors in Folder**.

**Spelling is American, always.** The engine API uses `Initialize`, `Serialize`, `Color`, `Behavior`, `Center`, `Normalize`. `PostInitializeComponents`, not `PostInitialise`. `FLinearColor`, not `FLinearColour`. When an `override` fails to resolve (`E1455`), check spelling before anything else — the error says the base member doesn't exist, and a `z`/`s` swap is the usual reason.

---

## 2. Header vs .cpp

| What you're adding | Header | .cpp |
|---|---|---|
| Data with a simple default | ✓ `= value` inline | — |
| Function | declare | define |
| Component | declare pointer | `CreateDefaultSubobject` in constructor |
| Data needing computed init | declare | assign in constructor |

- Default values go **inline in the header** (`int32 PocketCount = 37;`), not assigned in the constructor.
- Includes go in the `.cpp`. The header gets a forward declaration, or nothing.
- `.generated.h` is always the **last** include in a header.
- `UENUM` / `USTRUCT` declarations sit **above** the `UCLASS`, **below** the `.generated.h` include.
- The GameModeBase wizard does **not** generate a constructor. Declare it yourself under `public:` before defining it, or you get `E0333`.

---

## 3. UPROPERTY specifiers

Decide with one question: **is this a designer knob, or runtime state?**

```cpp
// Tuning — a rule of the game, set once on the class default
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Round")
float BettingWindowSeconds = 30.0f;

// Runtime state — the GameMode owns it, nobody edits it
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hasard|Round")
EHasardRoundPhase CurrentPhase = EHasardRoundPhase::Betting;
```

| Specifier | Use when |
|---|---|
| `EditDefaultsOnly` | Game rule. Per-instance override would be a bug (a second wheel with a different house edge). |
| `EditAnywhere` | Genuinely per-placement. Rare in this project. |
| `VisibleAnywhere` | Runtime state, shown for debugging only. |
| `BlueprintReadOnly` | **Default.** Blueprint may read. |
| `BlueprintReadWrite` | Only when writing has no side effects. Phase changes have side effects — route them through a function. |

**Time is a `double`.** `FPlatformTime::Seconds()` returns one, and Windows adds a
`+16777216.0` offset specifically so that storing it in a `float` breaks visibly —
at 2^24 a float's precision is exactly one second. Cast to `float` only for small
elapsed values on the way out, never for the absolute stamp.

- Members are `protected` by default. `private` + Blueprint exposure requires `meta=(AllowPrivateAccess="true")`; `protected` doesn't, which is why it's the default here.
- Category is always `Hasard|<Subsystem>` — `Hasard|Wheel`, `Hasard|Round`, `Hasard|Bankroll`.

## 4. UFUNCTION

- Never declare a `UFUNCTION` without a body — UHT generates registration code that references it, so it fails to **link**. (A plain C++ declaration can dangle; a `UFUNCTION` cannot.)
- `BlueprintNativeEvent` → define `Foo_Implementation()`, not `Foo()`.
- Any function bound to a delegate needs a bare `UFUNCTION()`, even with no Blueprint exposure.

---

## 5. Comments

Three triggers. Everything else is noise.

1. **A reader would reasonably write it differently.** The comment defends the choice.
2. **The line is a responsible-gaming decision**, not a mechanical one. It marks what must not be quietly "simplified" later.
3. **It sits above a reflected member.** `/** */` on a `UPROPERTY` becomes the **tooltip in the editor** — that one isn't written for programmers at all.

```cpp
/** How often the reality check interrupts. 15s for observation; 300s once the UI exists. */
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Session")
float RealityCheckIntervalSeconds = 15.0f;

/**
 * Wall clock at session start, from FPlatformTime::Seconds().
 * Deliberately not GetTimeSeconds(): world time stops on pause and scales with
 * dilation, either of which understates how long the player has been here.
 * double, not float - the engine offsets this value past 2^24.
 */
double SessionStartRealTime = 0.0;
```

**Delete every wizard comment on sight** — `// Called every frame`, `// Sets default values for this actor's properties`, `// Called to bind functionality to input`. A comment that restates the line beneath it trains you to stop reading comments, which is expensive on the day one of them is load-bearing.

The `.cpp` needs far less than the header, because the header carries the reasoning. Worth a line there: a non-obvious include, or an ordering that looks wrong (`ClearTimer` before `Super::EndPlay`).

**Deleting a responsible-gaming comment removes the only warning that the line below it is not free to change.** `SessionStartRealTime` looks exactly like something a tidy-minded reader would "fix" to `float` and `GetTimeSeconds()` in five minutes.

---

## 6. Logging

One shared category in `HasardTypes.h` (`DECLARE_LOG_CATEGORY_EXTERN`), not `DEFINE_LOG_CATEGORY_STATIC` per file — per-file statics create separate categories that merely share a name.

```cpp
UE_LOG(LogHasard, Warning, TEXT("GameMode BeginPlay - phase: %s"),
       *UEnum::GetValueAsString(CurrentPhase));
```

- `Warning` while developing (yellow, findable in the noise). Drop to `Log` once you filter by category.
- Enums: `*UEnum::GetValueAsString(X)`, never `(int32)X` with `%d`.
- The `*` is the `FString` → `TCHAR*` operator, not a dereference.

---

## 7. Build workflow

Do all of a module in one pass. Each editor↔VS transition costs a close-build-reopen cycle.

1. **Editor** — create every C++ class the module needs via the wizard, then close it
2. **Visual Studio** — write *all* headers and sources
3. **Build once** — `Ctrl+Shift+B` (not the Play button, not F5)
4. **Editor** — Blueprints, Project Settings, press Play, verify
5. **Commit**

Rules:

- Touched a header? Full build with the **editor closed**. Live Coding won't do it.
- A class compiled while the editor is open is invisible to it — no error is reported anywhere.
- `E####` errors come from IntelliSense; `C####` errors in the Output window come from MSVC. Only the second kind stops a build.
- IntelliSense squiggles naming `GENERATED_BODY()`, `Super::`, `_Implementation` or `.generated.h` are noise — it cannot run UHT, so it reads a stale generated header. `E0135: AActor has no member SetupPlayerInputComponent` is this: `Super` has not resolved to `APawn` yet.
- Plain C++ errors are real even from IntelliSense — `E0333` (defining an undeclared member), type mismatches, missing declarations. The prefix tells you who is speaking, not whether they are right; the tiebreaker is whether the symbol is macro-generated.
- Squiggles persisting after a clean build: right-click `Hasard.uproject` → Generate Visual Studio project files, then reopen the solution.
- A GameMode never set in Project Settings → Maps & Modes is a silent no-op that looks exactly like broken code.

---

## 8. Commits

**One commit per module**, made after the module works end to end — build, Blueprint, Project Settings, Play.

### Subject line

- **Imperative mood**, no trailing period: "Add", "Expose", "Wire", "Fix", "Rename"
- **50 characters max**, hard limit 72
- Name the type, not the file: `AHasardWheel`, not `HasardWheel.cpp`

```
Add AHasardWheel with scene root and mesh
Expose wheel tuning properties on AHasardWheel
Add GameMode, PlayerController and PlayerState
Rename project First -> Hasard
```

### Body — optional, and short

Only when the diff hides a **decision**. Two sentences, hard cap. If it takes more than that, it belongs in `docs/`, not in the log.

```powershell
git commit -m "Expose wheel tuning properties on AHasardWheel" -m "PocketCount = 37 is the European single-zero wheel (2.70% house edge). EditDefaultsOnly so a second wheel cannot silently run a different edge."
```

Skip the body entirely when the subject already says it:

```powershell
git commit -m "Add L_RouletteTest level"
```

### What earns a body

| Earns one | Doesn't |
|---|---|
| A magic number with a reason (`37`, `2.70%`) | Restating the diff |
| A class that looks dead but isn't | Listing files changed |
| A specifier choice that prevents a bug | "as per Module 3" |

### Before committing

`git status` — expect C++ pairs, `.uasset` files, and `Config/DefaultEngine.ini` when project settings changed. All three belong in the repo.

---

## 9. Repo layout

```
C:\UE5\Hasard\          repo root == project root (outside cloud-synced folders)
  Config/               DefaultEngine.ini is tracked
  Content/              binary assets via Git LFS
  Source/Hasard/        HasardTypes.h + one .h/.cpp pair per class
  docs/                 CONVENTIONS.md, leftover.md, unreal-cpp-notes.md
```

`.gitattributes` was committed before any assets — keep it that way for every new binary type.
