// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HasardTypes.h"
#include "HasardBettingComponent.generated.h"

class UHasardPayoutTable;
class UHasardTableLayout;

UCLASS(ClassGroup=(Hasard), meta=(BlueprintSpawnableComponent))
class HASARD_API UHasardBettingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHasardBettingComponent();

	/**
	 * Records a bet on one generated position. Returns false if the stake is not
	 * positive, the id is INDEX_NONE, or the bankroll cannot cover it. It cannot
	 * check that the id exists - it holds no layout - so callers pass what
	 * ResolvePosition gave them and nothing else.
	 *
	 * No bet type and no numbers: the position owns those. That is what makes an
	 * unsettleable bet impossible to express rather than merely discouraged.
	 */
	UFUNCTION(BlueprintCallable, Category = "Hasard|Betting")
	bool PlaceBet(int32 PositionId, int32 Stake);

	/** Drops every active bet. Called between rounds. */
	UFUNCTION(BlueprintCallable, Category = "Hasard|Betting")
	void ClearAllBets();

	/**
	 * Pays every winning bet against the table, then clears the round.
	 * The GameMode owns both assets and passes them in; the component owns the bets.
	 * The layout is what turns a stored id back into the numbers it covers.
	 */
	UFUNCTION(BlueprintCallable, Category = "Hasard|Betting")
	void SettleRound(int32 WinningPocket, const UHasardPayoutTable* PayoutTable,
		const UHasardTableLayout* TableLayout);

	UFUNCTION(BlueprintPure, Category = "Hasard|Betting")
	int32 GetTotalStaked() const;

private:
	/** UPROPERTY so the array serializes and Blueprint can read it. */
	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Betting",
		meta = (AllowPrivateAccess = "true"))
	TArray<FHasardBet> ActiveBets;
};
