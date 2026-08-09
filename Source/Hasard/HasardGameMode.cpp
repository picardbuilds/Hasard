// Copyright Picardbuilds. All Rights Reserved.


#include "HasardGameMode.h"
#include "HasardPlayerController.h"
#include "HasardPlayerState.h"
#include "HasardWheel.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogHasard, Log, All);

AHasardGameMode::AHasardGameMode()
{
	PlayerControllerClass = AHasardPlayerController::StaticClass();
	PlayerStateClass = AHasardPlayerState::StaticClass();
}

void AHasardGameMode::BeginPlay() 
{
	Super::BeginPlay();

	UE_LOG(LogHasard, Warning, TEXT("GameMode BeginPlay - phase %s, betting window: %.1fs"),
		*UEnum::GetValueAsString(CurrentPhase), BettingWindowSeconds);

	AActor* Found = UGameplayStatics::GetActorOfClass(GetWorld(), AHasardWheel::StaticClass());

	BoundWheel = Cast<AHasardWheel>(Found);
	if (BoundWheel)
	{
		BoundWheel->OnBallSettled.AddDynamic(this, &AHasardGameMode::HandleBallSettled);
	}
	else
	{
		UE_LOG(LogHasard, Warning, TEXT("GameMode: no wheel in the level, nothing to bind"));
	}
}

void AHasardGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundWheel)
	{
		BoundWheel->OnBallSettled.RemoveDynamic(this, &AHasardGameMode::HandleBallSettled);
		BoundWheel = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AHasardGameMode::HandleBallSettled(int32 WinningPocket)
{
	UE_LOG(LogHasard, Warning, TEXT("Ball settled in pocket %d"), WinningPocket);
}