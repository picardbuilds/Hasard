// Copyright Picardbuilds. All Rights Reserved.


#include "HasardInteractionComponent.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"

UHasardInteractionComponent::UHasardInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UHasardInteractionComponent::TraceForInteractable(FHitResult& OutHit) const 
{
	OutHit = FHitResult();

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return false;
	}
	
	const UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	OwnerPawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FVector End = EyeLocation + (EyeRotation.Vector() * TraceDistance);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerPawn);

	const bool bHit = World->SweepSingleByChannel(
		OutHit, EyeLocation, End, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(TraceRadius), Params);

	DrawDebugLine(World, EyeLocation, End,
		bHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 1.5f);

	// A sphere sweep that starts already overlapping reports bStartPenetrating and
	// leaves ImpactPoint at the trace start, which would resolve to a bet nowhere near
	// where the player was looking. Refuse it rather than guess.
	if (bHit && OutHit.bStartPenetrating)
	{
		OutHit = FHitResult();
		return false;
	}

	return bHit;
}