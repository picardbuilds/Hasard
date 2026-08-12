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

	/** Records a bet. Returns false if the stake is not positive, or the type unsettleable. */
	UFUNCTION(BlueprintCallable, Category = "Hasard|Betting")
	bool PlaceBet(EHasardBetType BetType, int32 PrimaryNumber, int32 Stake);

	/** Drops every active bet. Called between rounds. */
	UFUNCTION(BlueprintCallable, Category = "Hasard|Betting")
	void ClearAllBets();

	/**
	 * Pays every winning bet against the table, then clears the round.
	 * The GameMode owns both assets and passes them in; the component owns the bets.
	 * The layout is what says which pockets are red, so red and black cannot be
	 * settled without it.
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
