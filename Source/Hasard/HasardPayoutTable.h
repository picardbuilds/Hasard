// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HasardTypes.h"
#include "HasardPayoutTable.generated.h"

/**
 * One row of the table. No accessors and no private block: for a data asset the
 * data is the interface, because nothing has to happen when a value changes.
 */
USTRUCT(BlueprintType)
struct FHasardPayoutRule
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Payout")
	EHasardBetType BetType = EHasardBetType::StraightUp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Payout",
		meta = (ClampMin = "0"))
	int32 PayoutRatio = 35;

	/** How many pockets this bet covers. With PocketCount, this is the whole truth. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Payout",
		meta = (ClampMin = "1"))
	int32 CoveredNumbers = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Payout")
	FText DisplayName;
};

UCLASS(BlueprintType)
class HASARD_API UHasardPayoutTable : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	/** The rule for a bet type, or nullptr when the asset defines none. */
	const FHasardPayoutRule* FindRule(EHasardBetType BetType) const;

	UFUNCTION(BlueprintPure, Category = "Hasard|Payout")
	float GetWinProbability(const FHasardPayoutRule& Rule) const;

	/** Odds against, as "X to 1". */
	UFUNCTION(BlueprintPure, Category = "Hasard|Payout")
	float GetTrueOdds(const FHasardPayoutRule& Rule) const;

	/** As a fraction: 0.027 is 2.70 percent. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Payout")
	float GetHouseEdge(const FHasardPayoutRule& Rule) const;

	/**
	 * True when every rule shares one edge, which a correctly authored wheel
	 * guarantees. False means a row is mistyped, and no single figure may be shown.
	 */
	bool TryGetSharedHouseEdge(float& OutEdge) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Payout")
	TArray<FHasardPayoutRule> Rules;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Payout",
		meta = (ClampMin = "1"))
	int32 PocketCount = 37;
};
