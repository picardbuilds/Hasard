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

void UHasardTableLayout::AddPosition(TArray<FHasardBetPosition>& OutPositions,
	EHasardBetType BetType, const TArray<int32>& Covered,
	const FVector2D& ChipLocation, const FText& DisplayName) const
{
	FHasardBetPosition Position;
	Position.PositionId = OutPositions.Num();   // the index is the id, so they cannot drift
	Position.BetType = BetType;
	Position.CoveredNumbers = Covered;
	Position.CoveredNumbers.Sort();
	Position.ChipLocation = ChipLocation;
	Position.DisplayName = DisplayName;

	OutPositions.Add(MoveTemp(Position));
}

void UHasardTableLayout::BuildPositions(TArray<FHasardBetPosition>& OutPositions) const
{
	OutPositions.Reset();
	OutPositions.Reserve(157);

	const float GridFarY = GridOrigin.Y + NumberRows * CellSizeY;
	const float ZeroCenterX = GridOrigin.X - ZeroBoxWidth * 0.5f;
	const float ZeroCenterY = GridOrigin.Y + NumberRows * CellSizeY * 0.5f;

	// 1. Zero, then one straight up per cell.
	AddPosition(OutPositions, EHasardBetType::StraightUp, { 0 },
		FVector2D(ZeroCenterX, ZeroCenterY), LOCTEXT("Zero", "0"));

	for (int32 Column = 0; Column < NumberColumns; ++Column)
	{
		for (int32 Row = 0; Row < NumberRows; ++Row)
		{
			const int32 Number = GetNumberAt(Column, Row);
			AddPosition(OutPositions, EHasardBetType::StraightUp, { Number },
				GetCellCenter(Column, Row), FText::AsNumber(Number));
		}
	}

	// 2. Splits within a column: the chip sits on the line between two stacked cells.
	for (int32 Column = 0; Column < NumberColumns; ++Column)
	{
		for (int32 Row = 0; Row < NumberRows - 1; ++Row)
		{
			const int32 Low = GetNumberAt(Column, Row);
			const int32 High = GetNumberAt(Column, Row + 1);
			AddPosition(OutPositions, EHasardBetType::Split, { Low, High },
				FVector2D(GetCellCenter(Column, Row).X,
					GridOrigin.Y + (Row + 1) * CellSizeY),
				FText::Format(LOCTEXT("SplitFmt", "{0}/{1}"),
					FText::AsNumber(Low), FText::AsNumber(High)));
		}
	}

	// 3. Splits across columns: the chip sits on the line between two side-by-side cells.
	for (int32 Column = 0; Column < NumberColumns - 1; ++Column)
	{
		for (int32 Row = 0; Row < NumberRows; ++Row)
		{
			const int32 Left = GetNumberAt(Column, Row);
			const int32 Right = GetNumberAt(Column + 1, Row);
			AddPosition(OutPositions, EHasardBetType::Split, { Left, Right },
				FVector2D(GridOrigin.X + (Column + 1) * CellSizeX,
					GetCellCenter(Column, Row).Y),
				FText::Format(LOCTEXT("SplitFmt", "{0}/{1}"),
					FText::AsNumber(Left), FText::AsNumber(Right)));
		}
	}

	// 4. The three splits that pair zero with the first column.
	for (int32 Row = 0; Row < NumberRows; ++Row)
	{
		const int32 Number = GetNumberAt(0, Row);
		AddPosition(OutPositions, EHasardBetType::Split, { 0, Number },
			FVector2D(GridOrigin.X, GetCellCenter(0, Row).Y),
			FText::Format(LOCTEXT("ZeroSplitFmt", "0/{0}"), FText::AsNumber(Number)));
	}

	// 5. Streets: one whole column of three, chip on the far edge.
	for (int32 Column = 0; Column < NumberColumns; ++Column)
	{
		TArray<int32> Covered;
		for (int32 Row = 0; Row < NumberRows; ++Row)
		{
			Covered.Add(GetNumberAt(Column, Row));
		}

		AddPosition(OutPositions, EHasardBetType::Street, Covered,
			FVector2D(GetCellCenter(Column, 0).X, GridFarY),
			FText::Format(LOCTEXT("StreetFmt", "Street {0}-{1}"),
				FText::AsNumber(Covered[0]), FText::AsNumber(Covered.Last())));
	}

	// 6. The two trios. Three numbers for 11:1, same as a street, and the only
	//    other way to reach zero with three.
	AddPosition(OutPositions, EHasardBetType::Trio, { 0, 1, 2 },
		FVector2D(GridOrigin.X, GridOrigin.Y + CellSizeY), LOCTEXT("Trio012", "Trio 0/1/2"));
	AddPosition(OutPositions, EHasardBetType::Trio, { 0, 2, 3 },
		FVector2D(GridOrigin.X, GridOrigin.Y + 2 * CellSizeY), LOCTEXT("Trio023", "Trio 0/2/3"));

	// 7. Corners: the chip sits where four cells meet.
	for (int32 Column = 0; Column < NumberColumns - 1; ++Column)
	{
		for (int32 Row = 0; Row < NumberRows - 1; ++Row)
		{
			const TArray<int32> Covered = {
				GetNumberAt(Column, Row), GetNumberAt(Column, Row + 1),
				GetNumberAt(Column + 1, Row), GetNumberAt(Column + 1, Row + 1) };

			AddPosition(OutPositions, EHasardBetType::Corner, Covered,
				FVector2D(GridOrigin.X + (Column + 1) * CellSizeX,
					GridOrigin.Y + (Row + 1) * CellSizeY),
				FText::Format(LOCTEXT("CornerFmt", "Corner {0}"),
					FText::AsNumber(GetNumberAt(Column, Row))));
		}
	}

	// 8. The basket. Four numbers for 8:1 on a single-zero wheel, which is why
	//    the shared-edge audit still passes. The American five-number bet does not.
	AddPosition(OutPositions, EHasardBetType::Basket, { 0, 1, 2, 3 },
		FVector2D(GridOrigin.X, GridFarY), LOCTEXT("Basket", "Basket 0/1/2/3"));

	// 9. Six lines: two adjacent streets, chip on the far edge between them.
	for (int32 Column = 0; Column < NumberColumns - 1; ++Column)
	{
		TArray<int32> Covered;
		for (int32 Offset = 0; Offset < 2; ++Offset)
		{
			for (int32 Row = 0; Row < NumberRows; ++Row)
			{
				Covered.Add(GetNumberAt(Column + Offset, Row));
			}
		}

		AddPosition(OutPositions, EHasardBetType::SixLine, Covered,
			FVector2D(GridOrigin.X + (Column + 1) * CellSizeX, GridFarY),
			FText::Format(LOCTEXT("SixLineFmt", "Six line {0}-{1}"),
				FText::AsNumber(Covered[0]), FText::AsNumber(Covered.Last())));
	}

	// 10. Column bets. A grid row is a 2:1 column: the box sits past the last cell.
	for (int32 Row = 0; Row < NumberRows; ++Row)
	{
		TArray<int32> Covered;
		for (int32 Column = 0; Column < NumberColumns; ++Column)
		{
			Covered.Add(GetNumberAt(Column, Row));
		}

		AddPosition(OutPositions, EHasardBetType::Column, Covered,
			FVector2D(GridOrigin.X + NumberColumns * CellSizeX + OutsideBandDepth * 0.5f,
				GetCellCenter(0, Row).Y),
			FText::Format(LOCTEXT("ColumnFmt", "Column {0}"), FText::AsNumber(Row + 1)));
	}

	// 11. Dozens. Four grid columns each, in the band on the near side.
	for (int32 Dozen = 0; Dozen < 3; ++Dozen)
	{
		TArray<int32> Covered;
		for (int32 Number = Dozen * 12 + 1; Number <= Dozen * 12 + 12; ++Number)
		{
			Covered.Add(Number);
		}

		AddPosition(OutPositions, EHasardBetType::Dozen, Covered,
			FVector2D(GridOrigin.X + (Dozen * 4 + 2) * CellSizeX,
				GridOrigin.Y - OutsideBandDepth * 0.5f),
			FText::Format(LOCTEXT("DozenFmt", "Dozen {0}"), FText::AsNumber(Dozen + 1)));
	}

	// 12. The six even-money bets, in the band beyond the dozens.
	const float EvenMoneyY = GridOrigin.Y - OutsideBandDepth * 1.5f;
	const float EvenMoneySpan = NumberColumns * CellSizeX / 6.0f;

	TArray<int32> Low, High, Even, Odd, Red, Black;
	for (int32 Number = 1; Number <= NumberColumns * NumberRows; ++Number)
	{
		(Number <= 18 ? Low : High).Add(Number);
		(Number % 2 == 0 ? Even : Odd).Add(Number);
		(IsRedNumber(Number) ? Red : Black).Add(Number);
	}

	const TArray<EHasardBetType> EvenMoneyTypes = {
		EHasardBetType::Low, EHasardBetType::Even, EHasardBetType::Red,
		EHasardBetType::Black, EHasardBetType::Odd, EHasardBetType::High };

	const TArray<TArray<int32>> EvenMoneySets = { Low, Even, Red, Black, Odd, High };

	const TArray<FText> EvenMoneyNames = {
		LOCTEXT("Low", "1-18"), LOCTEXT("Even", "Even"), LOCTEXT("Red", "Red"),
		LOCTEXT("Black", "Black"), LOCTEXT("Odd", "Odd"), LOCTEXT("High", "19-36") };

	for (int32 Index = 0; Index < EvenMoneyTypes.Num(); ++Index)
	{
		AddPosition(OutPositions, EvenMoneyTypes[Index], EvenMoneySets[Index],
			FVector2D(GridOrigin.X + (Index + 0.5f) * EvenMoneySpan, EvenMoneyY),
			EvenMoneyNames[Index]);
	}
}

#undef LOCTEXT_NAMESPACE