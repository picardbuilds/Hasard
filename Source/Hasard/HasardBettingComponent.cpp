// Copyright Picardbuilds. All Rights Reserved.

#include "HasardBettingComponent.h"
#include "HasardTypes.h"
#include "HasardBankrollComponent.h"
#include "HasardPlayerState.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogHasard, Log, All);

/**
 * Deliberately incomplete: it judges straight-up bets and nothing else, so a red
 * bet on a red number reads as a loss. Module 12 replaces it with a real rule read
 * from the payout table. File-static, so it needs no header entry and no UFUNCTION.
 */
static bool BetWins(const FHasardBet& Bet, int32 WinningPocket)
{
	return Bet.BetType == EHasardBetType::StraightUp && Bet.PrimaryNumber == WinningPocket;
}

UHasardBettingComponent::UHasardBettingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UHasardBettingComponent::PlaceBet(EHasardBetType BetType, int32 PrimaryNumber, int32 Stake)
{
	if (Stake <= 0)
	{
		return false;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	AHasardPlayerState* PS = OwnerPawn ? OwnerPawn->GetPlayerState<AHasardPlayerState>() : nullptr;
	UHasardBankrollComponent* Bank =
		PS ? PS->FindComponentByClass<UHasardBankrollComponent>() : nullptr;

	// Take the money FIRST. A bet that was never paid for must never enter the array.
	if (!Bank || !Bank->TryStake(Stake))
	{
		UE_LOG(LogHasard, Warning, TEXT("PlaceBet refused: stake of %d not available"), Stake);
		return false;
	}

	// Named fields, not FHasardBet{ ... } - the Module 9 warning still applies.
	FHasardBet Bet;
	Bet.BetType       = BetType;
	Bet.PrimaryNumber = PrimaryNumber;
	Bet.Stake         = Stake;

	ActiveBets.Add(Bet);

	UE_LOG(LogHasard, Warning, TEXT("Bets: %d, total staked: %d"),
		ActiveBets.Num(), GetTotalStaked());

	return true;
}

void UHasardBettingComponent::ClearAllBets() 
{
	ActiveBets.Empty();
	UE_LOG(LogHasard, Warning, TEXT("ClearAllBets"));
}

void UHasardBettingComponent::SettleRound(int32 WinningPocket)
{
	const int32 Placed = ActiveBets.Num();

	// Backwards: RemoveAt shifts every later element down into the slot you just left.
	for (int32 i = ActiveBets.Num() - 1; i >= 0; --i)
	{
		if (!BetWins(ActiveBets[i], WinningPocket))
		{
			ActiveBets.RemoveAt(i);
		}
	}

	UE_LOG(LogHasard, Warning, TEXT("SettleRound on %d: %d bets placed, %d won"),
		WinningPocket, Placed, ActiveBets.Num());
}

int32 UHasardBettingComponent::GetTotalStaked() const 
{
	int32 Total = 0;
	for (const FHasardBet& Bet : ActiveBets) 
	{
		Total += Bet.Stake;
	}
	return Total;
}