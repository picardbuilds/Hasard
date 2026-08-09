// Copyright Picardbuilds. All Rights Reserved.

#include "HasardWheel.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogHasard, Log, All);

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
}

void AHasardWheel::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogHasard, Warning, TEXT("Wheel: EndPlay"));
	GetWorldTimerManager().ClearTimer(SpinTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void AHasardWheel::StartSpin()
{
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
	return FMath::RandRange(0, PocketCount - 1);
}