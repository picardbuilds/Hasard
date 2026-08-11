// Copyright Picardbuilds. All Rights Reserved.

#include "HasardTableLayout.h"
#include "HasardTypes.h"

#define LOCTEXT_NAMESPACE "Hasard"

int32 UHasardTableLayout::GetNumberAt(int32 Column, int32 Row) const
{
	if (Column < 0 || Column >= NumberColumns || Row < 0 || Row >= NumberRows)
	{
		return INDEX_NONE;
	}

	return Column * NumberRows + Row + 1;
}

int32 UHasardTableLayout::GetColumnOf(int32 Number) const
{
	if (Number < 1 || Number > NumberColumns * NumberRows)
	{
		return INDEX_NONE;
	}

	return (Number - 1) / NumberRows;
}

int32 UHasardTableLayout::GetRowOf(int32 Number) const
{
	if (Number < 1 || Number > NumberColumns * NumberRows)
	{
		return INDEX_NONE;
	}

	return (Number - 1) % NumberRows;
}

FVector2D UHasardTableLayout::GetCellCenter(int32 Column, int32 Row) const
{
	return FVector2D(
		GridOrigin.X + (static_cast<float>(Column) + 0.5f) * CellSizeX,
		GridOrigin.Y + (static_cast<float>(Row) + 0.5f) * CellSizeY);
}

FVector2D UHasardTableLayout::GetGridExtent() const
{
	return FVector2D(
		GridOrigin.X + NumberColumns * CellSizeX,
		GridOrigin.Y + NumberRows * CellSizeY);
}

bool UHasardTableLayout::IsRedNumber(int32 Number) const
{
	// The eighteen reds on a single-zero wheel. Not derivable from the number:
	// the sequence alternates within each half but breaks at 10/11 and 28/29.
	static const TSet<int32> RedNumbers = {
		1, 3, 5, 7, 9, 12, 14, 16, 18,
		19, 21, 23, 25, 27, 30, 32, 34, 36 };

	return RedNumbers.Contains(Number);
}

#undef LOCTEXT_NAMESPACE