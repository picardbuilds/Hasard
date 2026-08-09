// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HasardTypes.h"
#include "HasardInteractable.h"
#include "HasardBetSpot.generated.h"

class UBoxComponent;

UCLASS()
class HASARD_API AHasardBetSpot : public AActor, public IHasardInteractable
{
	GENERATED_BODY()
	
public:	
	AHasardBetSpot();

	virtual void OnPlayerInteract_Implementation(APawn* InstigatorPawn) override;

protected:

	UPROPERTY(VisibleAnywhere, Category = "Hasard|BetSpot")
	TObjectPtr<UBoxComponent> SpotBounds;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Hasard|BetSpot")
	EHasardBetType BetType = EHasardBetType::StraightUp;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "Hasard|BetSpot")
	int32 PrimaryNumber = 0;

	/** Table minimum for this square. Class default of 5, overridable per placement. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hasard|BetSpot", meta = (ClampMin = "1"))
	int32 Stake = 5;
};
