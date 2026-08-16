// Copyright Picardbuilds. All Rights Reserved.

#include "HasardChip.h"
#include "HasardTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

AHasardChip::AHasardChip()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	// No collision, and this one is load bearing. The felt is found by a Visibility
	// sweep; a chip that blocked that sweep would make the position underneath it
	// unclickable, so the second chip on any bet could never be placed.
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	// A 1 cm disc casting a shadow is cost with no picture to show for it, and there
	// will be a lot of them.
	Mesh->SetCastShadow(false);
}

void AHasardChip::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Here rather than BeginPlay, so a denomination reads correctly in the editor and
	// in the Blueprint preview rather than only once the game is running.
	ApplyChipColor();
}

void AHasardChip::SetPreviewMaterial(UMaterialInterface* PreviewMaterial)
{
	if (!Mesh || !PreviewMaterial)
	{
		return;
	}

	Mesh->SetMaterial(0, PreviewMaterial);

	// Re-apply, because the material just changed underneath it. SetMaterial discards
	// the dynamic instance ChipColor was written into, so without this a preview would
	// come out whatever color the preview material ships with - and a preview in the
	// wrong denomination's color is worse than no preview.
	ApplyChipColor();
}

void AHasardChip::ApplyChipColor()
{
	if (!Mesh || ColorParameterName.IsNone())
	{
		return;
	}

	if (!Mesh->GetMaterial(0))
	{
		// Loud, because a chip with no material is a chip the player cannot tell apart
		// from any other denomination.
		UE_LOG(LogHasard, Warning, TEXT("Chip %s has no material, so ChipColor shows nothing"),
			*GetNameSafe(this));
		return;
	}

	// Creates the dynamic instances it needs. Quiet when the material has no parameter
	// of that name: a chip painted by a texture instead is a decision, not a fault.
	Mesh->SetVectorParameterValueOnMaterials(ColorParameterName, FVector(ChipColor));
}