// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HasardInteractable.h"
#include "HasardFelt.generated.h"

class AHasardChip;
class UBoxComponent;
class UHasardBettingComponent;
class UHasardTableLayout;
class UInstancedStaticMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;
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

	/** Reprints the cloth, the lines and every colored box. Same button treatment. */
	UFUNCTION(CallInEditor, Category = "Hasard|Felt")
	void RebuildSurface();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
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

	/**
	 * Prints the whole felt: cloth, then a white plate per box, then the color inset on it.
	 *
	 * Called from BeginPlay and from the button, never from OnConstruction. It creates
	 * material instances, and OnConstruction runs on every property edit - one dynamic
	 * instance per nudge of a color slider is churn with nothing to show for it.
	 */
	void BuildSurface();

	/** Points a component at BoxMesh and SurfaceMaterial, and returns its color instance. */
	UMaterialInstanceDynamic* PrepareSurface(UStaticMeshComponent* Component,
		const FColor& Color) const;

	/**
	 * Which instanced component prints this box, which is the same question as what color
	 * it is: the components are split by color, so routing and coloring cannot disagree.
	 *
	 * Zero is not routed here. It is one quad with its own component, because it is the
	 * one box whose color is a statement about the odds rather than a decoration.
	 */
	UInstancedStaticMeshComponent* BoxesFor(const FHasardBetPosition& Position,
		const UHasardTableLayout& Layout) const;

	/** A box centered on a felt-local point, laid flat at the given height. */
	FTransform BoxTransform(const FVector2D& Center, const FVector2D& Size, float Height) const;

	/**
	 * What a position is printed as, which is not always what it is called.
	 *
	 * A real layout prints the three column boxes as "2 to 1" rather than naming them,
	 * so the felt says that. DisplayName still says "Column 2", because a click log and
	 * the bet readout have to name which one - "2 to 1" identifies a payout, not a bet.
	 */
	FText LabelTextFor(const FHasardBetPosition& Position) const;

	/** Which of the two rotations a position reads at. See LabelRotationTurned. */
	FRotator LabelRotationFor(const FHasardBetPosition& Position) const;

	/** Puts one chip on a position, on top of whatever is already there. */
	void SpawnChip(const FHasardBetPosition& Position);

	/** Destroys every chip and forgets every stack height. */
	void ClearChips();

	/** Bound to the betting component's OnBetsCleared. No UFUNCTION: not a dynamic delegate. */
	void HandleBetsCleared();

	/** The chip a click would place, or null. Its value is what a click costs. */
	const AHasardChip* GetChipDefault() const;

	/** Fixed origin. Everything else is measured and offset from here. */
	UPROPERTY(VisibleAnywhere, Category = "Hasard|Felt")
	TObjectPtr<USceneComponent> Root;

	/** The trace target. Sized from the layout, never authored by hand. */
	UPROPERTY(VisibleAnywhere, Category = "Hasard|Felt")
	TObjectPtr<UBoxComponent> Surface;

	/** The green cloth the whole layout is printed on. One quad, sized from the bounds. */
	UPROPERTY(VisibleAnywhere, Category = "Hasard|Surface")
	TObjectPtr<UStaticMeshComponent> Cloth;

	/**
	 * The white plate behind every box, one instance per box at full size.
	 *
	 * There is no separate line geometry. Each colored box is inset by BorderWidth on
	 * every side, so the plate showing round its edge *is* the printed line - which means
	 * a line can never be drawn somewhere a box is not, and the two cannot drift apart.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Hasard|Surface")
	TObjectPtr<UInstancedStaticMeshComponent> Lines;

	/**
	 * The colored faces, split by color rather than by bet type.
	 *
	 * Three components and about 120 instances for the whole felt, against one component
	 * per box. That is a draw-call decision made now because it is nearly free now: at
	 * VR framerates the same table has to render twice per frame, and sixty-odd separate
	 * primitives is the kind of thing that is painful to unpick after the fact.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Hasard|Surface")
	TObjectPtr<UInstancedStaticMeshComponent> RedBoxes;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Surface")
	TObjectPtr<UInstancedStaticMeshComponent> BlackBoxes;

	/** Every outside bet. Green is the table's own color, not an absence of one. */
	UPROPERTY(VisibleAnywhere, Category = "Hasard|Surface")
	TObjectPtr<UInstancedStaticMeshComponent> GreenBoxes;

	/**
	 * Zero, alone, on its own component.
	 *
	 * One quad does not need instancing, and putting it on the outside-bet component
	 * would have made zero the same color as a dozen box by construction. It is a single
	 * pocket with a color that means something, so it gets a mesh that can only be it.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Hasard|Surface")
	TObjectPtr<UStaticMeshComponent> ZeroBox;

	/**
	 * Assigned on BP_Felt, and it must be the same asset the GameMode holds.
	 * Two different layouts would put the chip somewhere the settlement does not expect.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Felt",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHasardTableLayout> TableLayout;

	/**
	 * Draws the grid and all 157 chip locations on BeginPlay. Debug builds only.
	 *
	 * Off by default since the felt prints itself: the debug lines and the printed lines
	 * are drawn from the same numbers, so leaving both on shows one picture twice and
	 * makes a real disagreement harder to see rather than easier.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Felt",
		meta = (AllowPrivateAccess = "true"))
	bool bDrawDebugLayout = false;

	/** Height of the collision box. The felt is flat; this is just enough to trace against. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Felt",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float SurfaceThickness = 2.0f;

	/** Every label this actor made. UPROPERTY so the collector can see them. */
	UPROPERTY()
	TArray<TObjectPtr<UTextRenderComponent>> Labels;

	/**
	 * The chip a click places. Assigned on BP_Felt.
	 *
	 * The class carries the stake, the mesh, the scale and the color together, so the
	 * felt never holds a stake of its own to disagree with. Swapping this for a ten
	 * changes what a click costs and what it looks like in one edit.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Chips",
		meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AHasardChip> ChipClass;

	/** Every chip currently on the felt. UPROPERTY so the collector can see them. */
	UPROPERTY()
	TArray<TObjectPtr<AHasardChip>> Chips;

	/**
	 * How many chips already sit on each position, keyed by PositionId.
	 *
	 * Not a UPROPERTY: two ints per entry and no UObject in sight, so there is nothing
	 * for the collector to track and nothing worth serializing into the map.
	 */
	TMap<int32, int32> StackCounts;

	/**
	 * The component this felt is listening to. Weak, because the pawn owns it and can
	 * be destroyed first - a raw pointer here would be a dangling unbind in EndPlay.
	 */
	TWeakObjectPtr<UHasardBettingComponent> BoundBetting;

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

	/**
	 * The rotation for labels that read from the end of the table rather than the side.
	 *
	 * Only the three "2 to 1" boxes use it, because only they sit at the end of the
	 * grid. A separate property rather than LabelRotation with 90 added to Yaw: the
	 * working values for the main rotation are still being found by eye, and deriving
	 * one from the other would make every correction move both groups at once.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Labels",
		meta = (AllowPrivateAccess = "true"))
	FRotator LabelRotationTurned = FRotator(90.0f, 90.0f, 0.0f);

	/**
	 * One ink for every numeral and every word on the felt.
	 *
	 * A real layout prints white on the colored box, so the box carries the red/black
	 * distinction and the text does not repeat it. Colored numerals plus colored boxes
	 * would be two sources for one fact, and the day they disagree the felt is lying
	 * about which pocket a cell belongs to.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Labels",
		meta = (AllowPrivateAccess = "true"))
	FColor NumeralColor = FColor(245, 245, 240);

	/** Whether to print the cloth, lines and boxes at all. Off leaves the debug view. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Surface",
		meta = (AllowPrivateAccess = "true"))
	bool bShowSurface = true;

	/** The unit quad every box is an instance of. /Engine/BasicShapes/Plane is 100 cm square. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Surface",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> BoxMesh;

	/**
	 * The material every printed surface uses, with one vector parameter for its color.
	 *
	 * The same asset five times over with five dynamic instances, rather than five
	 * authored materials: the colors below are the tunable thing, and an artist changing
	 * a felt green should not have to open a material graph to do it.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Surface",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> SurfaceMaterial;

	/** The vector parameter each color is written into. Matches AHasardChip's default. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Surface",
		meta = (AllowPrivateAccess = "true"))
	FName ColorParameterName = TEXT("Color");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Surface",
		meta = (AllowPrivateAccess = "true"))
	FColor ClothColor = FColor(12, 82, 48);

	/** The printed lines. Not pure white: cloth takes ink slightly off, and it reads better. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Surface",
		meta = (AllowPrivateAccess = "true"))
	FColor LineColor = FColor(235, 235, 228);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Surface",
		meta = (AllowPrivateAccess = "true"))
	FColor RedBoxColor = FColor(178, 26, 30);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Surface",
		meta = (AllowPrivateAccess = "true"))
	FColor BlackBoxColor = FColor(22, 22, 24);

	/**
	 * Zero gets its own color because zero is the entire house edge. It is not one of
	 * the blacks and must never be printed as though it were.
	 *
	 * This rule moved from the numeral to the box and did not soften on the way. A green
	 * zero is the one visual cue that the wheel is not an even bet, and CLAUDE.md's
	 * anti-patterns forbid removing it as surely as they forbid misstating the odds.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Surface",
		meta = (AllowPrivateAccess = "true"))
	FColor ZeroBoxColor = FColor(20, 130, 60);

	/** Dozens, columns and the even-money boxes. Cloth-colored on a real table. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Surface",
		meta = (AllowPrivateAccess = "true"))
	FColor OutsideBoxColor = FColor(14, 92, 54);

	/**
	 * How far each colored box is inset from its white plate, in centimeters.
	 *
	 * This is the line width, and it is a width rather than a line because the lines are
	 * not drawn: they are the plate showing through. Between two neighboring boxes the
	 * player sees twice this, which is why 0.6 reads as a normal printed rule on a 24 cm
	 * cell rather than as a gap.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Surface",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float BorderWidth = 0.6f;

	/** How far the cloth extends past the printed area on every side. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Surface",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.0"))
	float ClothMargin = 14.0f;

	/**
	 * Gap between the cloth, the plate and the color, in centimeters.
	 *
	 * Three coplanar quads z-fight, and the artifact is view-dependent flicker rather
	 * than a static error - the sort of thing that looks fine on the machine it was built
	 * on. LabelZOffset must stay larger than twice this, or the numerals sink into the box.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Surface",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.001"))
	float SurfaceZStep = 0.03f;
};
