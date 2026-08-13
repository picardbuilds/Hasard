// Copyright Picardbuilds. All Rights Reserved.

#include "HasardBettingComponent.h"
#include "HasardTypes.h"
#include "HasardBankrollComponent.h"
#include "HasardPayoutTable.h"
#include "HasardPlayerState.h"
#include "HasardTableLayout.h"
#include "GameFramework/Pawn.h"

/**
 * The bankroll lives on the PlayerState, not on this component. Both PlaceBet and
 * SettleRound need it, so the walk is written once. File-static: no header entry.
 */
static UHasardBankrollComponent* FindBankroll(const UActorComponent* Self)
{
	APawn* OwnerPawn = Self ? Cast<APawn>(Self->GetOwner()) : nullptr;
	AHasardPlayerState* PS = OwnerPawn ? OwnerPawn->GetPlayerState<AHasardPlayerState>() : nullptr;
	return PS ? PS->FindComponentByClass<UHasardBankrollComponent>() : nullptr;
}

/**
 * Every bet type needs a row in the payout table and a block in BuildPositions, and
 * neither of those is checked by the compiler. This assert is what turns adding an
 * enumerator into a build failure rather than a bet that quietly never pays.
 *
 * It fires on an insertion anywhere before High, which is how Trio and Basket arrived.
 * It does not fire on an append after High, so keep High last.
 *
 * IsNamedOnFelt is not listed here because it needs no reminder: its switch has no
 * default, so the compiler refuses a new enumerator on its own.
 */
static_assert(static_cast<uint8>(EHasardBetType::High) == 14,
	"EHasardBetType changed. Add a rule to DA_EuropeanPayouts, add a block to "
	"UHasardTableLayout::BuildPositions, then update this count.");

UHasardBettingComponent::UHasardBettingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UHasardBettingComponent::PlaceBet(int32 PositionId, int32 Stake)
{
	if (Stake <= 0 || PositionId == INDEX_NONE)
	{
		return false;
	}

	UHasardBankrollComponent* Bank = FindBankroll(this);

	// Take the money FIRST. A bet that was never paid for must never enter the array.
	if (!Bank || !Bank->TryStake(Stake))
	{
		UE_LOG(LogHasard, Warning, TEXT("PlaceBet refused: stake of %d not available"), Stake);
		return false;
	}

	// Named fields, not FHasardBet{ ... } - the Module 9 warning still applies.
	FHasardBet Bet;
	Bet.PositionId = PositionId;
	Bet.Stake      = Stake;

	ActiveBets.Add(Bet);

	UE_LOG(LogHasard, Warning, TEXT("Bets: %d, total staked: %d"),
		ActiveBets.Num(), GetTotalStaked());

	return true;
}

void UHasardBettingComponent::SettleRound(int32 WinningPocket,
	const UHasardPayoutTable* PayoutTable, const UHasardTableLayout* TableLayout)
{
	const int32 Placed = ActiveBets.Num();
	int32 Won = 0;
	int32 Returned = 0;

	if (!PayoutTable)
	{
		// Refuse to guess. Paying from a literal is the thing this module removes.
		UE_LOG(LogHasard, Error, TEXT("SettleRound: no payout table, paying nobody"));
		ActiveBets.Empty();
		return;
	}

	if (!TableLayout)
	{
		// Without the layout a stored id means nothing, so no bet can be evaluated.
		UE_LOG(LogHasard, Error, TEXT("SettleRound: no table layout, paying nobody"));
		ActiveBets.Empty();
		return;
	}

	UHasardBankrollComponent* Bank = FindBankroll(this);

	if (!Bank)
	{
		// The same refusal as the missing table. A summary line reporting money
		// returned when nothing was credited is worse than no summary at all.
		UE_LOG(LogHasard, Error, TEXT("SettleRound: no bankroll, paying nobody"));
		ActiveBets.Empty();
		return;
	}

	for (const FHasardBet& Bet : ActiveBets)
	{
		const FHasardBetPosition* Position = TableLayout->GetPositionById(Bet.PositionId);
		if (!Position)
		{
			// A stored id with no position means the layout changed under a live bet.
			UE_LOG(LogHasard, Error, TEXT("SettleRound: bet on id %d, which is not a position"),
				Bet.PositionId);
			continue;
		}

		// The whole of what used to be a thirteen-case switch. A position knows the
		// numbers it covers, so zero needs no special case and no bet type does either.
		if (!Position->CoveredNumbers.Contains(WinningPocket))
		{
			continue;
		}

		const FHasardPayoutRule* Rule = PayoutTable->FindRule(Position->BetType);
		if (!Rule)
		{
			UE_LOG(LogHasard, Error, TEXT("SettleRound: no rule for %s, bet not paid"),
				*UEnum::GetValueAsString(Position->BetType));
			continue;
		}

		// Stake back plus winnings: TryStake already took the stake when the bet
		// was placed, so a 35:1 winner returns 36 times it, not 35.
		const int32 Payout = Bet.Stake * (Rule->PayoutRatio + 1);
		Bank->CreditWinnings(Payout);

		++Won;
		Returned += Payout;
	}

	ActiveBets.Empty();

	UE_LOG(LogHasard, Warning, TEXT("SettleRound on %d: %d placed, %d won, %d returned"),
		WinningPocket, Placed, Won, Returned);
}

void UHasardBettingComponent::ClearAllBets()
{
	ActiveBets.Empty();
	UE_LOG(LogHasard, Warning, TEXT("ClearAllBets"));
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