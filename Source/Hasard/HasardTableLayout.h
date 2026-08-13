// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HasardTypes.h"
#include "HasardTableLayout.generated.h"

/**
 * Every measurement of the felt, in one asset.
 *
 * The visible layout and the click detection are both generated from these numbers,
 * so they cannot disagree with each other. Nothing else in the project may hard-code
 * a cell size or a grid origin.
 */
UCLASS(BlueprintType)
class HASARD_API UHasardTableLayout : public UDataAsset
{
	GENERATED_BODY()

public:
	/** The number at a grid cell. Column 0 row 0 is 1; column 0 row 2 is 3; column 1 row 0 is 4. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	int32 GetNumberAt(int32 Column, int32 Row) const;

	/** Grid column holding a number, 0-11. INDEX_NONE for zero or an out-of-range number. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	int32 GetColumnOf(int32 Number) const;

	/** Grid row holding a number, 0-2. INDEX_NONE for zero or an out-of-range number. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	int32 GetRowOf(int32 Number) const;

	/** Center of one cell, in the felt's local 2D space. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	FVector2D GetCellCenter(int32 Column, int32 Row) const;

	/** Far corner of the number grid. Not the felt - the zero box and the outside bands extend past it. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	FVector2D GetGridExtent() const;

	/**
	 * The whole felt, including the zero box and both outside bands.
	 *
	 * Every caller that needs a size asks for it here rather than adding the parts up,
	 * because a felt drawn from one sum and traced against another is a felt where
	 * clicks land in the wrong place near the edges.
	 */
	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	void GetFeltBounds(FVector2D& OutMin, FVector2D& OutMax) const;

	/**
	 * All 157 positions, built once and reused.
	 *
	 * Derived data, so it is deliberately not a UPROPERTY: serializing it would put a
	 * stale copy in the .uasset that could disagree with the numbers above it.
	 */
	const TArray<FHasardBetPosition>& GetPositions() const;

	/** One position by id, or null. Null means the id was never valid, which is worth logging. */
	const FHasardBetPosition* GetPositionById(int32 PositionId) const;

	/**
	 * Which position a chip at this felt-local point means, or INDEX_NONE for nowhere.
	 *
	 * This is the croupier reading the felt, and it is the only place that decides what
	 * a click means. LineTolerance is what makes a point near a boundary a line bet.
	 */
	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	int32 ResolvePosition(const FVector2D& LocalPoint) const;

	/** True for the eighteen red numbers on a single-zero wheel. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	bool IsRedNumber(int32 Number) const;

	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	int32 GetNumberColumns() const { return NumberColumns; }

	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	int32 GetNumberRows() const { return NumberRows; }

	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	float GetLineTolerance() const { return LineTolerance; }

	/** Raw values, exposed so a caller drawing grid lines need not rebuild them from cell centers. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	FVector2D GetGridOrigin() const { return GridOrigin; }

	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	FVector2D GetCellSize() const { return FVector2D(CellSizeX, CellSizeY); }

	/** Builds all 157 bet positions. Deterministic: same asset in, same array out, same order. */
	void BuildPositions(TArray<FHasardBetPosition>& OutPositions) const;

#if WITH_EDITOR
	/** Editing any measurement invalidates the cache, or the felt keeps drawing the old numbers. */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	/** Adds one position and stamps its PositionId from the array index. */
	void AddPosition(TArray<FHasardBetPosition>& OutPositions, EHasardBetType BetType,
		const TArray<int32>& Covered, const FVector2D& ChipLocation, const FText& DisplayName) const;

	/** Finds a generated position by type and covered set. Used only by ResolvePosition. */
	int32 FindPositionId(EHasardBetType BetType, const TArray<int32>& Covered) const;

	/** Built on first request, cleared when a property changes. Never serialized. */
	mutable TArray<FHasardBetPosition> CachedPositions;

	/** Twelve columns of three. Editable only because a future variant may differ. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Layout",
		meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 NumberColumns = 12;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Layout",
		meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 NumberRows = 3;

	/** Size of one number cell in centimeters, along the twelve columns. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Layout",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float CellSizeX = 24.0f;

	/** Size of one number cell in centimeters, across the three rows. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Layout",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float CellSizeY = 24.0f;

	/** Felt-local position of the near corner of cell (0,0). Everything is measured from here. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Layout",
		meta = (AllowPrivateAccess = "true"))
	FVector2D GridOrigin = FVector2D::ZeroVector;

	/** Width of the zero box, which sits at negative X of the grid and spans all three rows. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Layout",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float ZeroBoxWidth = 24.0f;

	/** Depth of each outside-bet band. Dozens sit against the grid, even-money beyond them. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Layout",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float OutsideBandDepth = 24.0f;

	/**
	 * How close to a line counts as being on it, as a fraction of a cell.
	 * 0.15 means the middle 70% of a cell is a straight up and the rest is a line bet.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Layout",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.05", ClampMax = "0.45"))
	float LineTolerance = 0.15f;
};
