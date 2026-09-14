// Copyright Picardbuilds. All Rights Reserved.

#include "HasardWheel.h"
#include "HasardTableLayout.h"
#include "HasardTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"

const TArray<int32>& AHasardWheel::GetPocketSequence()
{
	// A single-zero wheel, clockwise from zero. Reds and blacks alternate and the high
	// and low numbers are spread, so that no arc of the rim is worth more than any other.
	// That is a design, not a pattern, which is why no arithmetic produces it - the same
	// reason UHasardTableLayout::IsRedNumber lists its eighteen numbers instead of
	// computing them.
	static const TArray<int32> PocketSequence =
	{
		0, 32, 15, 19, 4,  21, 2, 25, 17, 34,
		6, 27, 13, 36, 11, 30, 8, 23, 10,  5,
		24, 16, 33, 1, 20, 14, 31, 9, 22, 18,
		29, 7, 28, 12, 35, 3, 26
	};

	return PocketSequence;
}

AHasardWheel::AHasardWheel()
{
	PrimaryActorTick.bCanEverTick = true;

	WheelRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WheelRoot"));
	RootComponent = WheelRoot;

	// Unscaled, and everything that has to stay square hangs off it.
	RimRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RimRoot"));
	RimRoot->SetupAttachment(WheelRoot);

	// The flattening lives here and goes no further, because nothing is parented to it.
	WheelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelMesh"));
	WheelMesh->SetupAttachment(RimRoot);

	RedPockets = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RedPockets"));
	RedPockets->SetupAttachment(RimRoot);

	BlackPockets = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BlackPockets"));
	BlackPockets->SetupAttachment(RimRoot);

	ZeroPocket = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ZeroPocket"));
	ZeroPocket->SetupAttachment(RimRoot);

	// Not on the rim. The ball crosses the pockets rather than turning with them.
	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(WheelRoot);

	UE_LOG(LogHasard, Warning, TEXT("Wheel: Constructor - %s"),
		*GetName());
}

void AHasardWheel::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UE_LOG(LogHasard, Warning, TEXT("Wheel: PostInitializeComponents"));
}

void AHasardWheel::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	// Printed here as well as in BeginPlay, and it has to be. PocketMesh and PocketMaterial
	// are EditDefaultsOnly, so they do not exist until the Blueprint has written its
	// defaults - after the constructor and before this. Printing only from BeginPlay left
	// the instanced components registered with no mesh, and a material assigned to them
	// afterwards was stored but never reached the renderer. 
	BuildPockets();
}

void AHasardWheel::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogHasard, Warning, TEXT("Wheel: BeginPlay"));

	AuditPocketSequence();
	BuildPockets();
	BuildNumerals();
}

void AHasardWheel::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogHasard, Warning, TEXT("Wheel: EndPlay"));
	Super::EndPlay(EndPlayReason);
}

void AHasardWheel::StartSpin()
{
	if (bIsSpinning)
	{
		UE_LOG(LogHasard, Warning, TEXT("Wheel: StartSpin ignored - a spin is already running"));
		return;
	}

	// Decided here, before a single frame of animation has run. The animation is told
	// where to go; it does not get a vote, and neither does the frame rate.
	PendingPocket = DetermineWinningPocket();
	SpinElapsed = 0.0f;
	bIsSpinning = true;

	ApplySpin(0.0f);

	UE_LOG(LogHasard, Warning, TEXT("Wheel: spin started, %.1fs"), SpinDuration);
	UE_LOG(LogHasard, Verbose, TEXT("wheel: this spin is going to pocket %d"), PendingPocket);
}

void AHasardWheel::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsSpinning)
	{
		return;
	}

	SpinElapsed += DeltaSeconds;

	// Position from total elapsed time over the duration - never by adding this frame's
	// movement to last frame's. An accumulated rotation drifts with the frame rate, and a
	// wheel that lands a fraction of a pocket out on a slower machine is a wheel whose
	// result depends on the hardware.
	const float Alpha = FMath::Clamp(SpinElapsed / SpinDuration, 0.0f, 1.0f);

	// Quadratic ease-out: monotonic, decelerating throughout, and exactly 1 at Alpha 1.
	// Cubic was tried first and rejected - it spends the last half second covering under
	// two degrees, which reads as a ball that has stopped and is being nudged home. That
	// is the shape of a hover, and module 5 is about not building one by accident.
	const float Eased = 1.0f - FMath::Square(1.0f - Alpha);

	ApplySpin(Eased);

	if (Alpha >= 1.0f)
	{
		FinishSpin();
	}
}

void AHasardWheel::ApplySpin(float Eased)
{
	float PocketYaw = 0.0f;
	TryGetPocketYaw(PendingPocket, PocketYaw);

	WheelYaw = WheelSweepDegrees * Eased;

	// Where the ball must be when Eased reaches 1: the pocket, carried round by the rim
	// it sits in. Everything before that is the same point minus the distance still to
	// travel, so the arrival is exact by construction rather than by a final correction.
	const float SettleBallYaw = WheelSweepDegrees + PocketYaw;
	BallYaw = SettleBallYaw - BallSweepDegrees * (1.0f - Eased);

	// One component turns, and everything printed on the rim is parented to it. That is
	// the whole reason RimRoot exists.
	if (RimRoot)
	{
		RimRoot->SetRelativeRotation(FRotator(0.0f, WheelYaw, 0.0f));
	}

	if (BallMesh)
	{
		const float Radius = FMath::Lerp(BallOuterRadius, BallPocketRadius, Eased);
		const FRotator Facing(0.0f, BallYaw, 0.0f);
		BallMesh->SetRelativeLocation(Facing.RotateVector(FVector(Radius, 0.0f, BallHeight)));
	}
}

void AHasardWheel::FinishSpin()
{
	bIsSpinning = false;

	const int32 WinningPocket = PendingPocket;
	PendingPocket = INDEX_NONE;

	// The invariant the animation exists to keep. Read back off the transforms that were
	// actually applied, not off the number that produced them - so a sign error, a wrong
	// sweep or a rounding fault is caught here rather than by a player who is told they
	// lost while looking at their own number under the ball.
	const int32 Landed = GetPocketAtYaw(BallYaw - WheelYaw);

	if (Landed != WinningPocket)
	{
		UE_LOG(LogHasard, Error,
			TEXT("spin landed the ball on %d and is broadcasting %d"), Landed, WinningPocket);
	}

	UE_LOG(LogHasard, Warning, TEXT("Wheel: ball settled in pocket %d"), WinningPocket);

	OnBallSettled.Broadcast(WinningPocket);
}

int32 AHasardWheel::DetermineWinningPocket() const
{
	return FMath::RandRange(0, GetPocketCount() - 1);
}

int32 AHasardWheel::GetPocketCount() const
{
	return GetPocketSequence().Num();
}

int32 AHasardWheel::GetNumberAtRimIndex(int32 RimIndex) const
{
	const TArray<int32>& Sequence = GetPocketSequence();
	return Sequence.IsValidIndex(RimIndex) ? Sequence[RimIndex] : INDEX_NONE;
}

int32 AHasardWheel::GetRimIndexOf(int32 Number) const
{
	// IndexOfByKey already returns INDEX_NONE when it finds nothing, which is the
	// answer this function wants for a number that is not on the wheel.
	return GetPocketSequence().IndexOfByKey(Number);
}

bool AHasardWheel::TryGetPocketAngleDegrees(int32 Number, float& OutDegrees) const
{
	const int32 RimIndex = GetRimIndexOf(Number);

	if (RimIndex == INDEX_NONE)
	{
		OutDegrees = 0.0f;
		return false;
	}

	OutDegrees = 360.0f * static_cast<float>(RimIndex) / static_cast<float>(GetPocketCount());
	return true;
}

bool AHasardWheel::TryGetPocketYaw(int32 Number, float& OutYaw) const
{
	float Degrees = 0.0f;

	if (!TryGetPocketAngleDegrees(Number, Degrees))
	{
		OutYaw = 0.0f;
		return false;
	}

	// The one negation in the project. Everything that places something on the rim, and
	// everything that reads something off it, goes through this function.
	OutYaw = -Degrees;
	return true;
}

int32 AHasardWheel::GetPocketAtYaw(float Yaw) const
{
	const int32 Count = GetPocketCount();

	if (Count <= 0)
	{
		return INDEX_NONE;
	}

	// Back through the negation, then wrapped into [0, 360). Fmod alone keeps the sign of
	// its argument, so a negative yaw needs the second pass to land in range.
	const float Degrees = FMath::Fmod(FMath::Fmod(-Yaw, 360.0f) + 360.0f, 360.0f);
	const float DegreesPerPocket = 360.0f / static_cast<float>(Count);

	// Round, not truncate: a yaw a hair short of a pocket centre belongs to that pocket,
	// not to the one behind it. The modulo catches the wrap at the top of the last pocket.
	const int32 RimIndex = FMath::RoundToInt(Degrees / DegreesPerPocket) % Count;

	return GetNumberAtRimIndex(RimIndex);
}

void AHasardWheel::AuditPocketSequence() const
{
	const TArray<int32>& Sequence = GetPocketSequence();
	const int32 Count = Sequence.Num();

	TSet<int32> Seen;
	int32 Offenders = 0;

	for (int32 RimIndex = 0; RimIndex < Count; RimIndex++)
	{
		const int32 Number = Sequence[RimIndex];

		if (Number < 0 || Number >= Count)
		{
			UE_LOG(LogHasard, Error,
				TEXT("Rim audit: slot %d holds %d, which is not a pocket on a wheel of %d"),
				RimIndex, Number, Count);
			++Offenders;
			continue;
		}

		bool bAlreadySeen = false;
		Seen.Add(Number, &bAlreadySeen);

		if (bAlreadySeen)
		{
			UE_LOG(LogHasard, Error, TEXT("Rim audit: slot %d repeats %d"), RimIndex, Number);
			++Offenders;
		}
	}

	// The claim, stated on every run: this is a wheel. Every number present, once each,
	// none invented. A distinct count below the pocket count is a repeat and an omission
	// in the same list, which is the typo a person cannot see and a spin will not reveal.
	UE_LOG(LogHasard, Warning,
		TEXT("Rim audit: %d pockets, %d distinct, %d offenders, %.4f degrees each"),
		Count, Seen.Num(), Offenders, 360.0f / static_cast<float>(Count));
}

void AHasardWheel::PreparePockets(UInstancedStaticMeshComponent* Component,
	const FColor& Color) const
{
	if (!Component || !PocketMesh || !PocketMaterial)
	{
		return;
	}

	Component->SetStaticMesh(PocketMesh);

	UMaterialInstanceDynamic* Instance =
		Component->CreateDynamicMaterialInstance(0, PocketMaterial);

	if (Instance && !ColorParameterName.IsNone())
	{
		// FromSRGBColor, not the FLinearColor constructor: these are authored as sRGB in
		// the details panel, and treating those bytes as linear washes every one of them
		// out. The felt learned this the same way.
		Instance->SetVectorParameterValue(ColorParameterName, FLinearColor::FromSRGBColor(Color));
	}
}

UInstancedStaticMeshComponent* AHasardWheel::PocketsFor(int32 Number,
	const UHasardTableLayout* Layout) const
{
	// Zero first. It is not in the red set, so asking IsRedNumber first would print it in
	// the black plate - the same ordering trap the felt met in guide 3 module 4.
	if (Number == 0)
	{
		return ZeroPocket;
	}

	return (Layout && Layout->IsRedNumber(Number)) ? RedPockets : BlackPockets;
}

void AHasardWheel::BuildPockets()
{
	// Clear first and unconditionally, before the bShowPockets test. OnConstruction runs
	// again on every property edit, and a pass that only added would stack a second rim
	// on the first.
	RedPockets->ClearInstances();
	BlackPockets->ClearInstances();
	ZeroPocket->ClearInstances();

	UInstancedStaticMeshComponent* const Plates[] = { RedPockets, BlackPockets, ZeroPocket };

	for (UInstancedStaticMeshComponent* Component : Plates)
	{
		Component->SetVisibility(bShowPockets);
	}

	if (!bShowPockets)
	{
		return;
	}

	if (!PocketMesh || !PocketMaterial)
	{
		// Loud, because the alternative is a wheel that still decides the result while
		// showing the player nothing to read it off.
		UE_LOG(LogHasard, Error,
			TEXT("Wheel: BP_Wheel needs both PocketMesh and PocketMaterial to print the rim"));
		return;
	}

	if (!TableLayout)
	{
		UE_LOG(LogHasard, Error,
			TEXT("Wheel: BP_Wheel has no Table Layout - the rim cannot be coloured"));
	}

	PreparePockets(RedPockets, RedPocketColor);
	PreparePockets(BlackPockets, BlackPocketColor);
	PreparePockets(ZeroPocket, ZeroPocketColor);

	const int32 Count = GetPocketCount();

	// A plate is exactly as wide as the arc it owns, measured at its own radius, so the
	// thirty-seven meet without a gap and without overlapping. Typing a width instead
	// would be a second statement of how many pockets there are, and the rim already
	// says that once.
	const float DegreesPerPocket = 360.0f / static_cast<float>(Count);
	const float PlateWidth =
		2.0f * PocketRadius * FMath::Tan(FMath::DegreesToRadians(DegreesPerPocket * 0.5f));

	// PocketMesh is a unit quad measured in centimetres, so the scale is the size wanted
	// over the mesh's own. Read it from the asset rather than assuming 100: swapping in a
	// differently sized quad should change nothing the player sees.
	const FVector MeshSize = PocketMesh->GetBounds().BoxExtent * 2.0f;
	const float MeshX = FMath::IsNearlyZero(MeshSize.X) ? 1.0f : MeshSize.X;
	const float MeshY = FMath::IsNearlyZero(MeshSize.Y) ? 1.0f : MeshSize.Y;

	for (const int32 Number : GetPocketSequence())
	{
		float Yaw = 0.0f;
		if (!TryGetPocketYaw(Number, Yaw))
		{
			continue;
		}

		const FRotator Facing(0.0f, Yaw, 0.0f);

		PocketsFor(Number, TableLayout)->AddInstance(FTransform(
			Facing,
			Facing.RotateVector(FVector(PocketRadius, 0.0f, PocketZOffset)),
			FVector(PocketLength / MeshX, PlateWidth / MeshY, 1.0f)));
	}

	UE_LOG(LogHasard, Warning,
		TEXT("Wheel: printed %d pockets - %d red, %d black, %d zero"),
		RedPockets->GetInstanceCount() + BlackPockets->GetInstanceCount()
		        + ZeroPocket->GetInstanceCount(),
		RedPockets->GetInstanceCount(), BlackPockets->GetInstanceCount(),
		ZeroPocket->GetInstanceCount());
}

void AHasardWheel::BuildNumerals()
{
	// Destroy first and unconditionally, before the bShowNumerals test. Turning numerals
	// off and rebuilding has to remove them, not leave the previous set standing.
	for (const TObjectPtr<UTextRenderComponent>& Existing : Numerals)
	{
		if (Existing)
		{
			Existing->DestroyComponent();
		}
	}

	Numerals.Reset();

	if (!bShowNumerals || !RimRoot)
	{
		return;
	}

	for (const int32 Number : GetPocketSequence())
	{
		float Yaw = 0.0f;
		if (!TryGetPocketYaw(Number, Yaw))
		{
			continue;
		}

		UTextRenderComponent* Numeral = NewObject<UTextRenderComponent>(this);

		// Movable before registering. A component made with NewObject is static, and
		// moving a registered static component warns and is then ignored.
		Numeral->SetMobility(EComponentMobility::Movable);
		Numeral->RegisterComponent();

		// Attached to the unscaled rim node rather than to the flattened mesh, so the rim
		// still carries the numbers round and nothing squashes them on the way.
		Numeral->AttachToComponent(RimRoot, FAttachmentTransformRules::KeepRelativeTransform);

		const FRotator Facing(0.0f, Yaw, 0.0f);
		Numeral->SetRelativeLocation(
			Facing.RotateVector(FVector(NumeralRadius, 0.0f, NumeralZOffset)));
		Numeral->SetRelativeRotation(
			FRotator(NumeralRotation.Pitch, NumeralRotation.Yaw + Yaw, NumeralRotation.Roll));
		Numeral->SetText(FText::AsNumber(Number));
		Numeral->SetWorldSize(NumeralTextSize);
		Numeral->SetHorizontalAlignment(EHTA_Center);
		Numeral->SetVerticalAlignment(EVRTA_TextCenter);
		Numeral->SetTextRenderColor(NumeralColor);
		
		Numerals.Add(Numeral);
	}

	UE_LOG(LogHasard, Warning, TEXT("Wheel: printed %d numerals"), Numerals.Num());
}