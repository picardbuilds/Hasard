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

	/**
	 * The player is aiming at this actor, at HitLocation, this frame.
	 *
	 * Called every frame the aim stays here, so an implementation must be cheap and must
	 * tolerate being told the same thing repeatedly. It is a question about intent and
	 * not a commitment: nothing may be spent, taken or recorded from inside it.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hasard|Interaction")
	void OnPlayerHover(APawn* InstigatorPawn, const FVector& HitLocation);

	/**
	 * The aim left this actor, or the player did.
	 *
	 * Guaranteed for every hover that started: on looking away, on looking at a different
	 * interactable, and on the pawn being torn down. Anything shown during hover is hidden
	 * here, because this is the only notification an implementation gets - and a preview
	 * left on screen after the aim moved is a preview of a bet the player is not making.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hasard|Interaction")
	void OnPlayerEndHover(APawn* InstigatorPawn);
};
