// Copyright Picardbuilds. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HasardTypes.h"
#include "HasardBettingComponent.generated.h"


UCLASS( ClassGroup=(Hasard), meta=(BlueprintSpawnableComponent) )
class HASARD_API UHasardBettingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHasardBettingComponent();

	/** Records a bet. Returns false if the stake is not positive. */
	UFUNCTION(BlueprintCallable, Category = "Hasard|Betting")
	bool PlaceBet(EHasardBetType BetType, int32 PrimaryNumber, int32 Stake);

	/** Drops every active bet. Called between rounds. */
	UFUNCTION(BlueprintCallable, Category = "Hasard|Betting")
	void ClearAllBets();

	/** Removes every losing bet. The pawn exposes an Exec wrapper for console testing. */
	UFUNCTION(BlueprintCallable, Category = "Hasard|Betting")
	void SettleRound(int32 WinningPocket);

	UFUNCTION(BlueprintPure, Category = "Hasard|Betting")
	int32 GetTotalStaked() const;

protected:

	/** UPROPERTY so the array serializes and Blueprint can read it. */
	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Betting")
	TArray<FHasardBet> ActiveBets;
};
