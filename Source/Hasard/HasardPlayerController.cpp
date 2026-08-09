// Copyright Picardbuilds. All Rights Reserved.

#include "HasardPlayerController.h"
#include "HasardPlayerState.h"
#include "HasardBankrollComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogHasard, Log, All);

void AHasardPlayerController::BeginPlay()
{
	Super::BeginPlay();

	AHasardPlayerState* PS = GetPlayerState<AHasardPlayerState>();
	if (!PS)
	{
		UE_LOG(LogHasard, Warning,
			TEXT("Controller: no AHasardPlayerState - check Player State Class"));
		return;
	}

	BoundBankroll = PS->FindComponentByClass<UHasardBankrollComponent>();
	if (!BoundBankroll)
	{
		UE_LOG(LogHasard, Warning, TEXT("Controller: player state has no bankroll component"));
		return;
	}

	BoundBankroll->OnBankrollChanged.AddDynamic(
		this, &AHasardPlayerController::HandleBankrollChanged);
}

void AHasardPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundBankroll)
	{
		BoundBankroll->OnBankrollChanged.RemoveDynamic(
			this, &AHasardPlayerController::HandleBankrollChanged);
		BoundBankroll = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AHasardPlayerController::HandleBankrollChanged(int32 NewBalance, int32 Delta,
	int32 SessionNetChange)
{
	UE_LOG(LogHasard, Warning, TEXT("Bankroll: balance %d (%+d), session net %+d"),
		NewBalance, Delta, SessionNetChange);
}