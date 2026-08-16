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

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/**
	 * Traces from the player's viewpoint and reports the whole hit.
	 *
	 * Returns the FHitResult rather than the actor, because the impact point is what
	 * turns a click on the felt into one of 157 bets. Check bBlockingHit on the way out.
	 */
	UFUNCTION(BlueprintCallable, Category = "Hasard|Interaction")
	bool TraceForInteractable(FHitResult& OutHit) const;

	/**
	 * This frame's aim, already traced, or false when it is on nothing.
	 *
	 * The click path reads this instead of tracing again, and that is the point of it
	 * existing. Two traces one frame apart can land on two different positions, which
	 * would let the player be charged for a bet other than the one they were shown.
	 */
	bool GetHoveredHit(FHitResult& OutHit) const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Interaction", meta = (ClampMin = "1.0"))
	float TraceDistance = 500.0f;

	/**
	 * Sphere radius for the sweep. Zero gives a plain line trace, which is what this
	 * wants.
	 *
	 * A swept sphere reports where the sphere first touched, and on an angled view that
	 * is displaced from the line of sight by up to the radius. The line-bet band is
	 * LineTolerance x CellSizeX - 3.6 cm at the defaults - so a 5 cm sphere was wider
	 * than the feature it was trying to land on.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Interaction", meta = (ClampMin = "0.0"))
	float TraceRadius = 0.0f;

	/** Draws the aim ray. Off by default: this now traces every frame, not every click. */
	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Interaction")
	bool bDrawDebugTrace = false;

private:
	/** Traces, then moves the hover from whatever held it to whatever holds it now. */
	void UpdateHover();

	/** Ends the current hover, if there is one. Every way out of hovering comes here. */
	void ClearHover();

	/** What this frame's trace hit. Only meaningful while HoveredActor is valid. */
	FHitResult HoveredHit;

	/**
	 * The interactable under the aim right now.
	 *
	 * Weak, because this is scenery the component does not own. A felt destroyed while
	 * hovered would leave a raw pointer that ClearHover then calls OnPlayerEndHover on.
	 */
	TWeakObjectPtr<AActor> HoveredActor;
};
