// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HasardInteractable.h"
#include "HasardFelt.generated.h"

class UBoxComponent;
class UHasardTableLayout;
class UTextRenderComponent;
struct FHasardBetPosition;

/**
 * The betting surface. One actor for all 157 positions.
 *
 * There is deliberately no actor per bet: 83 of the 157 positions are lines and corners
 * between cells, so they have no area of their own and nothing to own a collision volume.
 * Instead this is a single box the trace can hit, and the layout asset turns the impact
 * point into a position id. The felt draws itself from the same numbers, so the picture
 * and the hit test cannot disagree.
 */
UCLASS()
class HASARD_API AHasardFelt : public AActor, public IHasardInteractable
{
	GENERATED_BODY()
	
public:	
	AHasardFelt();

	virtual void OnPlayerInteract_Implementation(APawn* InstigatorPawn,
		const FVector& HitLocation) override;

	/** Redraws the debug overlay. A details-panel button: an actor gets no exec routing. */
	UFUNCTION(CallInEditor, Category = "Hasard|Felt")
	void DrawFelt();

	/** Rebuilds the felt labels after editing the layout or a color. Same button treatment. */
	UFUNCTION(CallInEditor, Category = "Hasard|Felt")
	void RebuildLabels();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

private:	
	/** World point to the felt's local 2D space, in centimeters. Z is discarded. */
	FVector2D WorldToFeltLocal(const FVector& WorldPoint) const;

	/** Felt-local 2D back to world, on the surface. The inverse of the above. */
	FVector FeltLocalToWorld(const FVector2D& LocalPoint) const;

	/** Resizes the collision box to whatever the layout says the felt measures. */
	void SyncBoxToLayout();

	/** Persistent debug lines for the grid, the bands, and every chip location. */
	void DrawDebugLayout() const;

	/**
	 * Destroys every existing label, then makes one per bet the player reads by name.
	 *
	 * Called from BeginPlay rather than OnConstruction: OnConstruction runs on every
	 * property edit and on every level load, and creating components there means
	 * managing their destruction against a construction script that may not have run.
	 */
	void BuildLabels();

	/** The color a number is printed in. Zero is not black, and that is the point. */
	FColor LabelColorFor(const FHasardBetPosition& Position,
		const UHasardTableLayout& Layout) const;

	/** Fixed origin. Everything else is measured and offset from here. */
	UPROPERTY(VisibleAnywhere, Category = "Hasard|Felt")
	TObjectPtr<USceneComponent> Root;

	/** The trace target. Sized from the layout, never authored by hand. */
	UPROPERTY(VisibleAnywhere, Category = "Hasard|Felt")
	TObjectPtr<UBoxComponent>  Surface;

	/**
	 * Assigned on BP_Felt, and it must be the same asset the GameMode holds.
	 * Two different layouts would put the chip somewhere the settlement does not expect.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Felt",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHasardTableLayout> TableLayout;

	/**
	 * What one click stakes. A single value until chip denominations exist; it is a
	 * property rather than a literal so the number is visible and tunable meanwhile.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Felt",
		meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 StakePerClick = 5;

	/** Draws the grid and all 157 chip locations on BeginPlay. Debug builds only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Felt",
		meta = (AllowPrivateAccess = "true"))
	bool bDrawDebugLayout = true;

	/** Height of the collision box. The felt is flat; this is just enough to trace against. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Felt",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float SurfaceThickness = 2.0f;

	/** Every label this actor made. UPROPERTY so the collector can see them. */
	UPROPERTY()
	TArray<TObjectPtr<UTextRenderComponent>> Labels;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Labels",
		meta = (AllowPrivateAccess = "true"))
	bool bShowLabels = true;

	/** Cap height in centimeters. A 24 cm cell takes about 8 before it crowds the lines. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Labels",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.5"))
	float LabelTextSize = 8.0f;

	/** Lifts the text off the felt so it does not z-fight with the surface. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Labels",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float LabelZOffset = 0.2f;

	/**
	 * Lays the text flat and decides which side of the table it reads from.
	 * If the numbers come out mirrored, add 180 to Yaw - that is the only axis at issue.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Labels",
		meta = (AllowPrivateAccess = "true"))
	FRotator LabelRotation = FRotator(90.0f, 180.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Labels",
		meta = (AllowPrivateAccess = "true"))
	FColor RedNumberColor = FColor(200, 30, 30);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Labels",
		meta = (AllowPrivateAccess = "true"))
	FColor BlackNumberColor = FColor(20, 20, 20);

	/**
	 * Zero gets its own color because zero is the entire house edge. It is not one of
	 * the blacks and must never be printed as though it were.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Labels",
		meta = (AllowPrivateAccess = "true"))
	FColor ZeroLabelColor = FColor(20, 130, 60);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Labels",
		meta = (AllowPrivateAccess = "true"))
	FColor OutsideLabelColor = FColor(40, 40, 40);
};
