// Copyright Picardbuilds. All Rights Reserved.

#include "HasardPayoutTable.h"

const FHasardPayoutRule* UHasardPayoutTable::FindRule(EHasardBetType BetType) const
{
	return Rules.FindByPredicate(
		[BetType](const FHasardPayoutRule& Rule) { return Rule.BetType == BetType; });
}

float UHasardPayoutTable::GetWinProbability(const FHasardPayoutRule& Rule) const
{
	if (PocketCount <= 0)
	{
		return 0.0f;
	}

	return static_cast<float>(Rule.CoveredNumbers) / static_cast<float>(PocketCount);
}

float UHasardPayoutTable::GetTrueOdds(const FHasardPayoutRule& Rule) const
{
	if (Rule.CoveredNumbers <= 0)
	{
		return 0.0f;
	}

	const int32 Losing = PocketCount - Rule.CoveredNumbers;
	return static_cast<float>(Losing) / static_cast<float>(Rule.CoveredNumbers);
}

float UHasardPayoutTable::GetHouseEdge(const FHasardPayoutRule& Rule) const
{
	const float P = GetWinProbability(Rule);
	return 1.0f - (P * static_cast<float>(Rule.PayoutRatio + 1));
}

bool UHasardPayoutTable::TryGetSharedHouseEdge(float& OutEdge) const
{
	if (Rules.Num() == 0)
	{
		return false;
	}

	OutEdge = GetHouseEdge(Rules[0]);

	for (const FHasardPayoutRule& Rule : Rules)
	{
		// A tenth of a percentage point: far wider than float error, far narrower
		// than any real mistake in a row.
		if (!FMath::IsNearlyEqual(GetHouseEdge(Rule), OutEdge, 0.001f))
		{
			return false;
		}
	}

	return true;
}