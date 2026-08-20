// Copyright Picardbuilds. All Rights Reserved.


#include "HasardBankrollComponent.h"
#include "HasardSaveGame.h"

UHasardBankrollComponent::UHasardBankrollComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHasardBankrollComponent::BeginPlay()
{
	Super::BeginPlay();

	// What a first sitting looks like. The controller overwrites the balance from the
	// save if the player continues, which happens before the start screen comes down.
	Balance     = StartingBalance;
	SessionStaked = 0;
	SessionWon    = 0;
}

void UHasardBankrollComponent::ApplySave(const UHasardSaveGame& Save, bool bContinuePrevious)
{
	PriorStaked = Save.LifetimeStaked;
	PriorWon = Save.LifetimeWon;

	Balance = bContinuePrevious ? Save.Balance : StartingBalance;

	// Delta of zero: nothing was staked or won, the figures were restored. The HUD needs
	// the broadcast anyway, because it has been showing the defaults until now.
	OnBankrollChanged.Broadcast(Balance, 0, GetSessionNetChange());
}

bool UHasardBankrollComponent::TryStake(int32 Amount)
{
	// Checks if you can bet
	if (Amount <= 0 || Amount > Balance)
	{
		return false;
	}

	Balance -= Amount;
	SessionStaked += Amount;

	OnBankrollChanged.Broadcast(Balance, -Amount, GetSessionNetChange());
	return true;
}

void UHasardBankrollComponent::CreditWinnings(int32 Amount)
{
	if (Amount <= 0) 
	{
		return;
	}

	Balance  += Amount;
	SessionWon += Amount;

	OnBankrollChanged.Broadcast(Balance, Amount, GetSessionNetChange());
}