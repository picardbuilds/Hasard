// Copyright Picardbuilds. All Rights Reserved.

#include "HasardBettingComponent.h"
#include "HasardTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogHasard, Log, All);

UHasardBettingComponent::UHasardBettingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UHasardBettingComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UHasardBettingComponent::PlaceBet(EHasardBetType BetType, int32 PrimaryNumber, int32 Stake)
{
	UE_LOG(LogHasard, Warning, TEXT("PlaceBet: type %s on %d for %d"),
		*UEnum::GetValueAsString(BetType), PrimaryNumber, Stake);
	return true;
}

void UHasardBettingComponent::ClearAllBets() 
{
	UE_LOG(LogHasard, Warning, TEXT("ClearAllBets"));
}
