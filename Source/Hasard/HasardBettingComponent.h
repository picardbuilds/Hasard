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

	UFUNCTION(BlueprintCallable, Category = "Hasard|Betting")
	bool PlaceBet(EHasardBetType BetType, int32 PrimaryNumber, int32 Stake);

	UFUNCTION(BlueprintCallable, Category = "Hasard|Betting")
	void ClearAllBets();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Betting")
	int32 ChipDenomination = 5;
};
