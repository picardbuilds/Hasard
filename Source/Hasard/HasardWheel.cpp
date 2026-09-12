// Copyright Picardbuilds. All Rights Reserved.

#include "HasardWheel.h"
#include "HasardTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

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
	PrimaryActorTick.bCanEverTick = false;

	WheelRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WheelRoot"));
	RootComponent = WheelRoot;

	WheelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelMesh"));
	WheelMesh->SetupAttachment(WheelRoot);

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

void AHasardWheel::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogHasard, Warning, TEXT("Wheel: BeginPlay"));

	AuditPocketSequence();
}

void AHasardWheel::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogHasard, Warning, TEXT("Wheel: EndPlay"));
	GetWorldTimerManager().ClearTimer(SpinTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void AHasardWheel::StartSpin()
{
	if (GetWorldTimerManager().IsTimerActive(SpinTimerHandle))
	{
		UE_LOG(LogHasard, Warning, TEXT("Wheel: StartSpin ignored - a spin is already running"));
		return;
	}

	GetWorldTimerManager().SetTimer(SpinTimerHandle, this,
		&AHasardWheel::FinishSpin, SpinDuration, false);
}

void AHasardWheel::FinishSpin()
{
	const int32 WinningPocket = DetermineWinningPocket();
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