// Copyright Picardbuilds. All Rights Reserved.


#include "HasardBankrollComponent.h"

UHasardBankrollComponent::UHasardBankrollComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHasardBankrollComponent::BeginPlay()
{
	Super::BeginPlay();

	Balance     = StartingBalance;
	TotalStaked = 0;
	TotalWon    = 0;
}

bool UHasardBankrollComponent::TryStake(int32 Amount)
{
	// Checks if you can bet
	if (Amount <= 0 || Amount > Balance)
	{
		return false;
	}

	Balance -= Amount;
	TotalStaked += Amount;

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
	TotalWon += Amount;

	OnBankrollChanged.Broadcast(Balance, Amount, GetSessionNetChange());
}