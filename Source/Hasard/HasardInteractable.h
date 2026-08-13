// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HasardInteractable.generated.h"

class APawn;

UINTERFACE(MinimalAPI)
class UHasardInteractable : public UInterface
{
	GENERATED_BODY()
};

class HASARD_API IHasardInteractable
{
	GENERATED_BODY()

public:
	/**
	 * The player interacted with this actor at HitLocation, in world space.
	 *
	 * The location is part of the interaction, not a detail of it. On a felt, where the
	 * chip lands *is* the bet, so an interface that only says "something was clicked"
	 * cannot express a split. Implementations free to ignore it are welcome to.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hasard|Interaction")
	void OnPlayerInteract(APawn* InstigatorPawn, const FVector& HitLocation);
};
