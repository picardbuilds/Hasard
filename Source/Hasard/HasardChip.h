// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HasardChip.generated.h"

class UStaticMeshComponent;

/**
 * One chip on the felt, and the denomination it represents.
 *
 * An actor per chip rather than instances of one mesh: a player reaches a handful of
 * positions by hand, so the count never grows enough to justify losing the ability to
 * hold a reference to a single chip. The felt owns them and destroys them on settle.
 *
 * The class is the denomination. A five and a ten are two Blueprint subclasses, each
 * carrying its own value, color and mesh, so what a chip is worth and what it looks
 * like can never disagree - they are properties of the same asset.
 */
UCLASS()
class HASARD_API AHasardChip : public AActor
{
	GENERATED_BODY()

public:
	AHasardChip();

	virtual void OnConstruction(const FTransform& Transform) override;

	/** Thickness of one chip, so the felt can stack them without measuring the mesh. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Chip")
	float GetChipHeight() const { return ChipHeight; }

	/**
	 * What placing this chip stakes.
	 *
	 * The chip carries the number, not the felt. A felt that knew the stake and a chip
	 * that knew its appearance would be one value in two places, and the pair could
	 * drift into a chip reading 25 that costs 5.
	 */
	UFUNCTION(BlueprintPure, Category = "Hasard|Chip")
	int32 GetChipValue() const { return ChipValue; }

private:
	/** Pushes ChipColor into the mesh material. Called from OnConstruction, so the editor shows it. */
	void ApplyChipColor();

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Chip")
	TObjectPtr<UStaticMeshComponent> Mesh;

	/**
	 * How far up the next chip in a stack sits, in centimeters.
	 *
	 * A property rather than the mesh bounds: the mesh is a scaled cylinder and its
	 * bounds are a rendering detail, while stack spacing is a decision about how a
	 * stack reads. Reading it off the bounds would tie one to the other for no gain.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Chip",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float ChipHeight = 1.0f;

	/** The stake one of these places. ClampMin 1: a chip worth nothing is not a chip. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Chip",
		meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 ChipValue = 5;

	/** How this denomination reads at a glance. Real tables color by value for exactly this reason. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Chip",
		meta = (AllowPrivateAccess = "true"))
	FLinearColor ChipColor = FLinearColor(0.10f, 0.35f, 0.75f);

	/**
	 * The vector parameter ChipColor is written into. A name rather than a hard-coded
	 * "Color" so a later material can call it whatever it likes without a code change.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Chip",
		meta = (AllowPrivateAccess = "true"))
	FName ColorParameterName = TEXT("Color");
};
