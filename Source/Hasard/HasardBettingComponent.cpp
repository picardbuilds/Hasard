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
 * Trio and Basket both include zero, and PrimaryNumber cannot say which trio a
 * chip on 0/1/2 means as against one on 0/2/3. Neither is settleable from this
 * struct, so neither may be accepted. Module 2 replaces PrimaryNumber with a
 * position id and both become ordinary.
 */
static bool IsSettleableBetType(EHasardBetType BetType)
{
	return BetType != EHasardBetType::Trio && BetType != EHasardBetType::Basket;
}

/**
 * Inserting an enumerator into EHasardBetType without adding a case below produces
 * a bet that takes the stake and can never win, and nothing reports it - no compiler
 * warning, no log line, no failed test. This assert is what turns that into a build
 * failure. It catches an insertion anywhere before High, which is how Trio and Basket
 * arrived; it does not catch an append after High, so keep High last.
 */
static_assert(static_cast<uint8>(EHasardBetType::High) == 14,
	"EHasardBetType changed. Add a case to BetCoversPocket, add a rule to "
	"DA_EuropeanPayouts, then update this count.");

/**
 * Does this bet cover the winning pocket?
 *
 * PrimaryNumber is the lowest number of the group, and the groups follow the felt's
 * three-column layout: a street is P, P+1, P+2 along a row; a corner is P, P+1, P+3,
 * P+4; a six line is P through P+5. Splits are the horizontal kind only, because
 * FHasardBet carries a single number and a vertical split cannot be expressed in one.
 *
 * Zero is deliberately excluded from every outside bet. That one pocket is the entire
 * house edge, and writing it as a special case is more honest than hiding it in a range.
 *
 * Layout is a reference, not a pointer: SettleRound has already refused the round if
 * the asset is missing, so there is nothing to check for here.
 */
static bool BetCoversPocket(const FHasardBet& Bet, int32 Pocket,
	const UHasardTableLayout& Layout)
{
	const int32 P = Bet.PrimaryNumber;

	switch (Bet.BetType)
	{
	case EHasardBetType::StraightUp: return Pocket == P;
	case EHasardBetType::Split:      return Pocket == P || Pocket == P + 1;
	case EHasardBetType::Street:     return Pocket >= P && Pocket <= P + 2;
	case EHasardBetType::Corner:     return Pocket == P || Pocket == P + 1
		                                 || Pocket == P + 3 || Pocket == P + 4;
	case EHasardBetType::SixLine:    return Pocket >= P && Pocket <= P + 5;
	case EHasardBetType::Column:     return Pocket != 0 && (Pocket % 3) == (P % 3);
	case EHasardBetType::Dozen:      return Pocket >= P && Pocket <= P + 11;
	case EHasardBetType::Red:        return Pocket != 0 && Layout.IsRedNumber(Pocket);
	case EHasardBetType::Black:      return Pocket != 0 && !Layout.IsRedNumber(Pocket);
	case EHasardBetType::Even:       return Pocket != 0 && (Pocket % 2) == 0;
	case EHasardBetType::Odd:        return Pocket != 0 && (Pocket % 2) == 1;
	case EHasardBetType::Low:        return Pocket >= 1 && Pocket <= 18;
	case EHasardBetType::High:       return Pocket >= 19 && Pocket <= 36;

	case EHasardBetType::Trio:
	case EHasardBetType::Basket:
		// PlaceBet refuses these, so arriving here means one got in another way.
		// Say so rather than returning a quiet no.
		UE_LOG(LogHasard, Error, TEXT("BetCoversPocket: %s cannot be settled from PrimaryNumber"),
			*UEnum::GetValueAsString(Bet.BetType));
		return false;
	}

	// Unreachable while the assert above holds. Kept because the compiler requires a
	// return, and it is the line a missing case would fall through to.
	return false;
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

	// Refuse before the money moves. A stake taken for a bet that cannot win is the
	// worst failure this component has, and it is the one that reports nothing.
	if (!IsSettleableBetType(BetType))
	{
		UE_LOG(LogHasard, Error,
			TEXT("PlaceBet refused: %s needs a bet position, which arrives in module 2"),
			*UEnum::GetValueAsString(BetType));
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
	Bet.BetType       = BetType;
	Bet.PrimaryNumber = PrimaryNumber;
	Bet.Stake         = Stake;

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
		// The same refusal. Without the layout there is no red list, so a red bet
		// would settle as black - silently, and only on half the spins.
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
		if (!BetCoversPocket(Bet, WinningPocket, *TableLayout))
		{
			continue;
		}

		const FHasardPayoutRule* Rule = PayoutTable->FindRule(Bet.BetType);
		if (!Rule)
		{
			UE_LOG(LogHasard, Error, TEXT("SettleRound: no rule for %s, bet not paid"),
				*UEnum::GetValueAsString(Bet.BetType));
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