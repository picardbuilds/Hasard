// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "HasardInteractionComponent.generated.h"

UCLASS( ClassGroup=(Hasard), meta=(BlueprintSpawnableComponent) )
class HASARD_API UHasardInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHasardInteractionComponent();

	/**
	 * Traces from the owning pawn's eyes and reports the whole hit.
	 *
	 * Returns the FHitResult rather than the actor, because the impact point is what
	 * turns a click on the felt into one of 157 bets. Check bBlockingHit on the way out.
	 */
	UFUNCTION(BlueprintCallable, Category = "Hasard|Interaction")
	bool TraceForInteractable(FHitResult& OutHit) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Interaction", meta = (ClampMin = "1.0"))
	float TraceDistance = 500.0f;

	/** Sphere radius for the sweep. Zero gives a plain line trace. */
	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Interaction", meta = (ClampMin = "0.0"))
	float TraceRadius = 5.0f;
};
