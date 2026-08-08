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

AActor* UHasardInteractionComponent::TraceForInteractable() const 
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return nullptr;
	}
	
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	OwnerPawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FVector End = EyeLocation + (EyeRotation.Vector() * TraceDistance);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerPawn);

	FHitResult Hit;
	const bool bHit = World->SweepSingleByChannel(
		Hit, EyeLocation, End, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(TraceRadius), Params);

	DrawDebugLine(World, EyeLocation, End,
		bHit ? FColor::Green : FColor::Red, false, 1.0f, 0, 1.5f);

	return bHit ? Hit.GetActor() : nullptr;
}