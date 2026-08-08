// Copyright Picardbuilds. All Rights Reserved.


#include "HasardGameMode.h"
#include "HasardPlayerController.h"
#include "HasardPlayerState.h"

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
}