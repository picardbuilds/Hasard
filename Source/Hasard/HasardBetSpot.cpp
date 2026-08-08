// Copyright Picardbuilds. All Rights Reserved.


#include "HasardBetSpot.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogHasard, Log, All);

AHasardBetSpot::AHasardBetSpot()
{
	PrimaryActorTick.bCanEverTick = false;

	SpotBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("SpotBounds"));
	RootComponent = SpotBounds;

	// A felt square: wide and flat. Tune when have real table dimension.
	SpotBounds->SetBoxExtent(FVector(25.0f, 25.0f, 2.0f));

	// Set here rather than per-Blueprint: a bet spot that does not block a
	// Visibility trace is not a bet spot. This is a rule, not a placement choice.
	SpotBounds->SetCollisionProfileName(TEXT("BlockAll"));
}

void AHasardBetSpot::OnPlayerInteract_Implementation(APawn* InstigatorPawn)
{
	UE_LOG(LogHasard, Warning, TEXT("BetSpot: %s on %d, from %s"),
		*UEnum::GetValueAsString(BetType), PrimaryNumber, *GetNameSafe(InstigatorPawn));
}