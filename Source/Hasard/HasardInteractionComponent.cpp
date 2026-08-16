// Copyright Picardbuilds. All Rights Reserved.


#include "HasardInteractionComponent.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HasardInteractable.h"
#include "HasardTypes.h"

UHasardInteractionComponent::UHasardInteractionComponent()
{
	// The aim has to be known before the click, not because of it. Nothing else in this
	// project ticks, and this is the one thing that genuinely cannot be event-driven:
	// there is no notification for "the player is now looking somewhere else".
	PrimaryComponentTick.bCanEverTick = true;
}

void UHasardInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateHover();
}

void UHasardInteractionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// The felt outlives the pawn on level teardown, so without this it keeps a ghost
	// chip and a readout for a player that no longer exists.
	ClearHover();

	Super::EndPlay(EndPlayReason);
}

void UHasardInteractionComponent::UpdateHover()
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	FHitResult Hit;
	AActor* HitActor = TraceForInteractable(Hit) ? Hit.GetActor() : nullptr;

	if (HitActor && !HitActor->Implements<UHasardInteractable>())
	{
		HitActor = nullptr;
	}

	if (!HitActor)
	{
		ClearHover();
		return;
	}

	// Ending the old hover before starting the new one, so an implementation that is
	// both never sees itself hovered twice with no end in between.
	if (HoveredActor.Get() != HitActor)
	{
		ClearHover();
		HoveredActor = HitActor;
	}

	HoveredHit = Hit;

	IHasardInteractable::Execute_OnPlayerHover(HitActor, OwnerPawn, Hit.ImpactPoint);
}

void UHasardInteractionComponent::ClearHover()
{
	if (AActor* Previous = HoveredActor.Get())
	{
		IHasardInteractable::Execute_OnPlayerEndHover(Previous, Cast<APawn>(GetOwner()));
	}

	HoveredActor = nullptr;
	HoveredHit = FHitResult();
}

bool UHasardInteractionComponent::GetHoveredHit(FHitResult& OutHit) const
{
	OutHit = HoveredHit;
	return HoveredActor.IsValid() && HoveredHit.bBlockingHit;
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

	// The camera, not the pawn. GetActorEyesViewPoint returns the pawn's location lifted
	// by BaseEyeHeight, and the camera sits at the far end of a spring arm - so the ray
	// matched the view direction while starting somewhere else entirely, and the
	// crosshair could not agree with where the trace landed.
	if (const APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
	{
		PC->GetPlayerViewPoint(EyeLocation, EyeRotation);
	}
	else
	{
		OwnerPawn->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	}

	const FVector End = EyeLocation + (EyeRotation.Vector() * TraceDistance);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerPawn);

	const bool bHit = World->SweepSingleByChannel(
		OutHit, EyeLocation, End, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(TraceRadius), Params);

	if (bDrawDebugTrace)
	{
		// Lifetime of one frame, because this runs every frame now. The old 1.0 second
		// would draw sixty overlapping lines and read as a solid bar.
		DrawDebugLine(World, EyeLocation, End,
			bHit ? FColor::Green : FColor::Red, false, -1.0f, 0, 1.0f);
	}

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