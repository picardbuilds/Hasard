# Hasard — discrepancy report against Epic's coding standard

Audited 2026-08-08 against Epic's official C++ Coding Standard and Epic's engine-usage
documentation. Engine: UE 5.8.1. Scope: `C:\UE5\Hasard\docs\CONVENTIONS.md` and all
twelve files in `Source/Hasard/`.

Two sections. **Section A** is where the project's own written conventions contradict or
omit Epic — fix these first, because the code follows them. **Section B** is where the
code breaks Epic or breaks its own conventions.

Severity: **Bug** misbehaves at runtime · **Violation** breaks a stated Epic rule ·
**Drift** breaks the project's own convention · **Suggestion** optional.

---

## Section A — CONVENTIONS.md vs Epic

### A1 — "Members are protected by default" contradicts Epic *(Violation, systemic)*

`CONVENTIONS.md` §3: *"Members are `protected` by default. `private` + Blueprint exposure
requires `meta=(AllowPrivateAccess="true")`; `protected` doesn't, which is why it's the
default here."*

Epic's Encapsulation section, verbatim:

> *"Class members should almost always be declared private unless they are part of the
> public/protected interface to the class."*
> *"If particular fields are only intended to be usable by derived classes, make them
> private and provide protected accessors."*

The convention chose `protected` specifically to avoid the `AllowPrivateAccess` meta tag.
Epic treats that meta tag as the intended cost of correct encapsulation — Epic's own
`Character` template marks its private camera components exactly that way.

This is the largest discrepancy in the project, and it is a single decision rather than
many: every member of every class follows it. Resolve it once at the convention level.

**Fix**: change §3 to private-by-default, with `meta = (AllowPrivateAccess = "true")` on
editor-exposed members and protected accessors where a subclass genuinely needs access.
Then migrate the classes.

### A2 — No copyright rule *(Violation)*

`CONVENTIONS.md` says nothing about the file header. Epic requires the copyright notice as
the **first line of every source file**, and fails CI when it is missing or malformed. All
twelve source files still carry the wizard placeholder (see B1).

**Fix**: add a rule. Set the value once in **Project Settings → Description → Copyright
Notice** so the class wizard stops emitting the placeholder.

### A3 — No formatting section *(Violation)*

Nothing covers braces, indentation, or whitespace. Epic is specific and mandatory here:
Allman braces, braces on every block including single statements, **tabs at size 4 for
leading whitespace**, spaces only for mid-line alignment, explicit `break` or a
"falls through" comment on every switch case, always a `default` case.

The code has whitespace violations that a written rule would have caught (B6, B7).

Note: Epic states **no maximum line length**. If you want one, it is a house rule, not an
Epic rule.

### A4 — `virtual` + `override` is missing, and it is counter-intuitive *(Suggestion)*

Epic, verbatim: *"When declaring a virtual function in a derived class that overrides a
virtual function in the parent class, you must use both the `virtual` and the `override`
keywords."*

This is the **opposite** of the common convention elsewhere (Google style and most linters
say drop `virtual` once `override` is present). The Hasard code happens to get it right
everywhere, but nothing records why — so a future cleanup pass, human or agent, will
"tidy" the `virtual` away and be confidently wrong.

**Fix**: state it in §2 with the reason.

### A5 — No `TObjectPtr` rule *(Suggestion)*

The code uses `TObjectPtr<T>` correctly in every header UPROPERTY, but the convention
never says to. Epic's UE5 Migration Guide recommends `TObjectPtr<T>` for UObject pointer
properties in `UCLASS`/`USTRUCT` types, while locals, parameters and return types stay raw
pointers. Worth writing down before it drifts.

### A6 — No const-correctness section *(Suggestion)*

Epic: *"All code should strive to be const-correct."* Const on non-mutating member
functions, const ref for non-modified parameters, const iteration, const on by-value
params and locals — and **never `const` on a return type** ("this inhibits move semantics
… and will give compile warnings for built-in types").

### A7 — Default values inline is stated more absolutely than Epic does *(Suggestion)*

`CONVENTIONS.md` §2 makes header-inline defaults a rule. Epic treats default member
initializers as a **judgment call** — good for keeping type, flags and default together,
but they force rebuilds of dependents and cannot initialize base classes, subobjects or
forward-declared pointers. Epic: *"default member initializers make more sense with
in-game code than engine code."*

For this project the convention is fine. Worth noting it is a choice, not a requirement,
and that it does not extend to component pointers.

### Where the conventions are *stricter* than Epic — keep these

These add rules Epic does not state. They are compatible and worth keeping:

- **§1** the `Hasard` type prefix, and the American-spelling rule (correct: the engine API
  is `Initialize`, `Serialize`, `Color`, `Behavior`).
- **§5** the three comment triggers, and "delete every wizard comment on sight" — a
  sharper version of Epic's *"Do not over comment bad code."*
- **§6** one shared log category rather than per-file statics. Epic does not say this;
  it is correct, and the code currently breaks it (B3).
- **§7** the batch build workflow, and the `E####` vs `C####` distinction.
- **§8** commit format.
- The opening rule — **spawn the Blueprint, never the C++ class**. Not a coding-standard
  matter at all, and the highest-value line in the document.

---

## Section B — code findings

### B1 — Copyright placeholder in all 12 files *(Violation)*

Every file in `Source/Hasard/` authored by the wizard still reads:

```cpp
// Fill out your copyright notice in the Description page of Project Settings.
```

Epic requires a correctly formatted copyright line as the first line, and fails CI without
it. For a portfolio piece intended to be read by a prospective employer, the placeholder
reads as unfinished.

**Fix**: set Project Settings → Description → Copyright Notice, then replace the line in
all twelve files:

```cpp
// Copyright Joseph Picard. All Rights Reserved.
```

Only `Hasard.h` and `Hasard.Build.cs` are correct today, and only because the engine
template shipped them that way.

### B2 — Constructor computes from an `EditDefaultsOnly` property *(Bug)*

`HasardPlayerPawn.cpp`, constructor:

```cpp
CameraBoom->TargetArmLength = CameraDistance;   // CameraDistance is EditDefaultsOnly
```

Blueprint applies its stored property overrides **after** the C++ constructor runs. So the
constructor reads the C++ default of `250.0f`, sets the arm length from it, and a designer
who changes `CameraDistance` in `BP_PlayerPawn`'s Class Defaults sees the value change in
the panel and **nothing happen in the game**. Silent no-op, and it looks exactly like a
broken camera.

**Fix** — move it to `PostInitializeComponents`:

```cpp
void AHasardPlayerPawn::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	CameraBoom->TargetArmLength = CameraDistance;
}
```

Use `OnConstruction` instead if you want the editor viewport to preview it too.

This is a general trap worth adding to `CONVENTIONS.md` §2 — the constructor may not read
any `EditDefaultsOnly`/`EditAnywhere` property to compute anything.

### B3 — Four separate log categories named `LogHasard` *(Bug / Drift)*

`DEFINE_LOG_CATEGORY_STATIC(LogHasard, Log, All);` appears in `HasardWheel.cpp`,
`HasardGameMode.cpp`, `HasardPlayerPawn.cpp` and `HasardPlayerState.cpp`.

`DEFINE_LOG_CATEGORY_STATIC` creates a **file-local** category. Four files means four
distinct categories that merely share a display name — so `Log LogHasard Verbose` in the
console affects one of them, and verbosity is inconsistent across the project in a way
that looks like the console command not working.

This is precisely what `CONVENTIONS.md` §6 already forbids: *"One shared category in
`HasardTypes.h` (`DECLARE_LOG_CATEGORY_EXTERN`), not `DEFINE_LOG_CATEGORY_STATIC` per
file — per-file statics create separate categories that merely share a name."*

**`HasardTypes.h` does not exist.** The convention describes a file that was never created,
and `CONVENTIONS.md` §9 lists it in the repo layout.

**Fix** — create `Source/Hasard/HasardTypes.h`:

```cpp
// Copyright Joseph Picard. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

HASARD_API DECLARE_LOG_CATEGORY_EXTERN(LogHasard, Log, All);
```

`Source/Hasard/HasardTypes.cpp`:

```cpp
// Copyright Joseph Picard. All Rights Reserved.

#include "HasardTypes.h"

DEFINE_LOG_CATEGORY(LogHasard);
```

Then delete the four `DEFINE_LOG_CATEGORY_STATIC` lines and `#include "HasardTypes.h"` in
each `.cpp`. `HasardTypes.h` is also where the shared `UENUM`s belong — `EHasardRoundPhase`
currently sits in `HasardGameMode.h`, which forces anything needing the phase to include
the GameMode header.

### B4 — Duplicate `public:` blocks *(Violation)*

`HasardWheel.h`, `HasardPlayerPawn.h` and `HasardCharacter.h` each open `public:`, then
`protected:`, then `public:` again.

Epic: *"Classes should be organized with the reader in mind rather than the writer …
the public implementation should be declared first, followed by the class's private
implementation."* One block per access level, in order.

This is the class wizard's own default layout, which is why it survives review — it looks
like normal Unreal code. It is still a violation.

**Fix**: merge each pair. In `HasardWheel.h`, `Tick` moves up beside the constructor.

### B5 — Wizard comments throughout *(Drift)*

`// Sets default values for this actor's properties`, `// Called when the game starts or
when spawned`, `// Called every frame`, `// Called to bind functionality to input`,
`// Sets default values`, and the empty `/** */` stubs above `AHasardGameMode`,
`AHasardPlayerState` and `AHasardPlayerController`.

`CONVENTIONS.md` §5: *"Delete every wizard comment on sight … A comment that restates the
line beneath it trains you to stop reading comments, which is expensive on the day one of
them is load-bearing."* The project stated the rule and then did not apply it — and this
project has genuinely load-bearing comments (the `SessionStartRealTime` block in
`HasardPlayerState.h` is exactly the case §5 was written to protect).

### B6 — Leading space before a tab *(Violation)*

`HasardWheel.cpp`, first line of the constructor body, begins with a space and then a tab:

```
 	// Set this actor to call Tick() every frame...
```

Epic: *"Use tabs for whitespace at the beginning of a line, not spaces."* The line also
carries a wizard comment (B5), so it deletes entirely.

### B7 — Tabs used for mid-line alignment *(Violation)*

`HasardPlayerPawn.cpp`, in `SetupPlayerInputComponent`:

```cpp
EIC->BindAction(LookAction,		ETriggerEvent::Triggered, this, &AHasardPlayerPawn::Look);
EIC->BindAction(PlaceBetAction, ETriggerEvent::Started,   this, &AHasardPlayerPawn::PlaceBet);
EIC->BindAction(SpinAction,		ETriggerEvent::Started,   this, &AHasardPlayerPawn::RequestSpin);
```

Lines 1 and 3 align with tabs; line 2 aligns with spaces. Epic allows spaces *only* for
mid-line alignment — tabs are for leading whitespace. The result renders differently at
any tab width other than yours, which is why the rule exists.

**Fix**: spaces for all three, or drop the alignment.

### B8 — Detached `else` *(Violation)*

`HasardPlayerPawn.cpp`:

```cpp
	}

	else
	{
```

A blank line between the closing brace and `else`. Epic's if/else formatting puts `else`
on the line after the closing brace with nothing between.

### B9 — Mapping context added in `SetupPlayerInputComponent` *(Suggestion — fragile)*

`AddMappingContext` currently runs inside `SetupPlayerInputComponent`. That function fires
on input-component setup, which does not guarantee possession and the local player are
fully resolved — and it does not re-run on re-possession. The robust placements are
`BeginPlay` or `PawnClientRestart`.

Related: if `TableMappingContext` is unset on the Blueprint, the `if (TableMappingContext)`
guard swallows it and the player simply has no input, with nothing in the log. Given
`CONVENTIONS.md`'s own rule about the C++-vs-Blueprint class picker producing exactly this
class of silent failure, this deserves an `ensureMsgf` or a warning log on the `else`.

### B10 — Session clock does not survive level travel *(Bug — and a principle problem)*

`AHasardPlayerState` holds `SessionStartRealTime` and the reality-check timer.

`PlayerState` survives pawn death, but **not** a level change — travel constructs a new
`PlayerState`, so `SessionStartRealTime` resets and the elapsed count restarts at zero.

For most games that is a minor annoyance. For this one it fails the governing principle in
`CLAUDE.md`: a session timer that resets on travel **understates how long the player has
been playing**, which is the exact failure mode `SessionStartRealTime`'s own comment says
it was chosen to avoid. The comment defends against `float` and `GetTimeSeconds()` and then
the storage location reintroduces the same understatement by a different route.

**Fix**: move session tracking to a `UGameInstanceSubsystem`, which persists across map
loads. `PlayerState` keeps the per-round, per-player data. This also sets up the persistence
work that Guide 2's closing module flags as the next step — *"session limits that reset when
the application closes are not limits."*

### B11 — `Tick` enabled with an empty body *(Suggestion)*

`AHasardWheel` sets `PrimaryActorTick.bCanEverTick = true` and `Tick` does nothing but call
`Super`. Per-frame cost for nothing. Set it `false` until the spin animation needs it, and
consider whether a timer is the better fit when it does.

### B12 — `HasardCharacter` is unused template leftover *(Suggestion)*

`HasardCharacter.h/.cpp` are pure wizard output — empty constructor, empty `BeginPlay`,
empty `Tick`, empty `SetupPlayerInputComponent`, every wizard comment intact. Nothing
references it; the pawn is `AHasardPlayerPawn`. It compiles, appears in every class picker
beside the real pawn, and invites exactly the wrong selection.

**Fix**: delete both files, or start using it. Also note the parameter
`class UInputComponent* PlayerInputComponent` uses an inline `class` keyword rather than
the file's own forward-declaration style.

### B13 — Module dependencies should be private *(Suggestion)*

`Hasard.Build.cs`:

```csharp
PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
PrivateDependencyModuleNames.AddRange(new string[] {  });
```

Epic's rule: public for modules whose headers appear in **your own public headers**;
private otherwise. `EnhancedInput` types are only forward-declared in `HasardPlayerPawn.h`
and only included in `.cpp` files, so `EnhancedInput` and `InputCore` belong in
`PrivateDependencyModuleNames`. Harmless today, but it leaks the dependency to anything
that later depends on this module.

Worth recording alongside this: a missing module here surfaces as an **unresolved external
symbol at link time**, not a compile error — which Guide 2 already teaches, correctly.

### B14 — Missing `ClampMin` on numeric knobs *(Suggestion)*

`PocketCount = 37`, `SpinDuration = 6.0f`, `BettingWindowSeconds = 30.0f`,
`RealityCheckIntervalSeconds = 15.0f`, `CameraDistance = 250.0f` accept any value a
designer types, including zero and negative. `RealityCheckIntervalSeconds = 0` in
particular sets a looping timer with a zero interval.

```cpp
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Wheel",
          meta = (ClampMin = "1"))
int32 PocketCount = 37;
```

`ClampMin`/`ClampMax` are hard clamps; `UIMin`/`UIMax` only bound the slider.

### B15 — Redundant `UMETA(DisplayName)` *(Suggestion)*

```cpp
Betting UMETA(DisplayName = "Betting"),
```

The display names are identical to the enumerator names, so the tags add nothing. Delete
them unless a name needs to differ. `enum class : uint8` with `UENUM(BlueprintType)` is
otherwise exactly right.

---

## What is already correct

Worth stating, because it is most of the code:

- `TObjectPtr<T>` on every header UPROPERTY, raw pointers in `.cpp` — matches Epic's
  guidance exactly, including the part most projects get backwards.
- `virtual` **and** `override` on every override — the counter-intuitive Epic rule, right
  everywhere.
- `enum class : uint8` under `UENUM(BlueprintType)`.
- Forward declarations in headers, concrete includes in `.cpp`.
- `.generated.h` last in every header.
- `Super::` called in every lifecycle override, with `EndPlay` correctly doing its cleanup
  before `Super::` (`ClearTimer` first).
- `SessionStartRealTime` as `double` from `FPlatformTime::Seconds()`, with a comment that
  explains the choice — a genuine load-bearing comment, and the reason §5 exists.
- `*UEnum::GetValueAsString(CurrentPhase)` rather than a cast to `int32`.
- `GetSessionElapsedSeconds()` as `BlueprintPure` with no side effects.
- Enhanced Input rather than the legacy axis/action system.
- `CreateDefaultSubobject` confined to constructors; `SetupAttachment` used correctly.
- `Category = "Hasard|Subsystem"` on every exposed property.

---

## Suggested order of work

1. **A1** — decide private-vs-protected. It changes every header, so do it before B4.
2. **B3** — create `HasardTypes.h`, move the log category and `EHasardRoundPhase`.
3. **B1** — copyright, one pass over twelve files.
4. **B2** — the camera bug. Real and currently silent.
5. **B10** — move session tracking to a subsystem, before more depends on it.
6. **B4, B5, B6, B7, B8** — one formatting pass, in the same commit as B1.
7. **A2–A7** — write the convention changes down so the next pass does not re-litigate.
8. **B9, B11–B15** — as convenient.

Items 1–5 are the ones that change behavior or would be visible to someone reading this as
a portfolio piece. The rest is hygiene.
