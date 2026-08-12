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

	/** Far corner of the number grid, so callers can size the felt without repeating the sum. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	FVector2D GetGridExtent() const;

	/** Builds all 157 bet positions. Deterministic: same asset in, same array out, same order. */
	void BuildPositions(TArray<FHasardBetPosition>& OutPositions) const;

	/** True for the eighteen red numbers on a single-zero wheel. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	bool IsRedNumber(int32 Number) const;

	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	int32 GetNumberColumns() const { return NumberColumns; }

	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	int32 GetNumberRows() const { return NumberRows; }

	UFUNCTION(BlueprintPure, Category = "Hasard|Layout")
	float GetLineTolerance() const { return LineTolerance; }

private:
	/** Adds one position and stamps its PositionId from the array index. */
	void AddPosition(TArray<FHasardBetPosition>& OutPositions, EHasardBetType BetType,
		const TArray<int32>& Covered, const FVector2D& ChipLocation, const FText& DisplayName) const;

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
	float CellSizeX = 12.0f;

	/** Size of one number cell in centimeters, across the three rows. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Layout",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float CellSizeY = 12.0f;

	/** Felt-local position of the near corner of cell (0,0). Everything is measured from here. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Layout",
		meta = (AllowPrivateAccess = "true"))
	FVector2D GridOrigin = FVector2D::ZeroVector;

	/** Width of the zero box, which sits at negative X of the grid and spans all three rows. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Layout",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float ZeroBoxWidth = 12.0f;

	/** Depth of each outside-bet band. Dozens sit against the grid, even-money beyond them. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Layout",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float OutsideBandDepth = 12.0f;

	/**
	 * How close to a line counts as being on it, as a fraction of a cell.
	 * 0.22 means the middle 56% of a cell is a straight up and the rest is a line bet.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Layout",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.05", ClampMax = "0.45"))
	float LineTolerance = 0.22f;
};
