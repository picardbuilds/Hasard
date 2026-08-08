// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HasardInteractionComponent.generated.h"


UCLASS( ClassGroup=(Hasard), meta=(BlueprintSpawnableComponent) )
class HASARD_API UHasardInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHasardInteractionComponent();

	/** Traces from the owning pawn eyes. Returns the blocking actor, or null. */
	UFUNCTION(BlueprintCallable, Category = "Hasard|Interaction")
	AActor* TraceForInteractable() const;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Interaction", meta = (ClampMin = "1.0"))
	float TraceDistance = 500.0f;

	/** Sphere radius for the sweep. Zero gives a plain line trace. */
	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Interaction", meta = (ClampMin = "0.0"))
	float TraceRadius = 5.0f;
};
