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

void UHasardTableLayout::GetFeltBounds(FVector2D& OutMin, FVector2D& OutMax) const
{
	// Two bands sit below the grid: dozens against it, even-money beyond them.
	OutMin = FVector2D(
		GridOrigin.X - ZeroBoxWidth,
		GridOrigin.Y - OutsideBandDepth * 2.0f);
	OutMax = FVector2D(
		GridOrigin.X + NumberColumns * CellSizeX + OutsideBandDepth,
		GridOrigin.Y + NumberRows * CellSizeY);
}

const TArray<FHasardBetPosition>& UHasardTableLayout::GetPositions() const
{
	if (CachedPositions.Num() == 0)
	{
		BuildPositions(CachedPositions);
	}

	return CachedPositions;
}

const FHasardBetPosition* UHasardTableLayout::GetPositionById(int32 PositionId) const
{
	// PositionId is the array index by construction, so this is a bounds check, not a search.
	const TArray<FHasardBetPosition>& Positions = GetPositions();
	return Positions.IsValidIndex(PositionId) ? &Positions[PositionId] : nullptr;
}

int32 UHasardTableLayout::FindPositionId(EHasardBetType BetType, const TArray<int32>& Covered) const
{
	TArray<int32> Wanted = Covered;
	Wanted.Sort(); 

	for (const FHasardBetPosition& Position : GetPositions())
	{
		if (Position.BetType == BetType && Position.CoveredNumbers == Wanted)
		{
			return Position.PositionId;
		}
	}

	return INDEX_NONE;
}

int32 UHasardTableLayout::ResolvePosition(const FVector2D& LocalPoint) const
{
	FVector2D FeltMin, FeltMax;
	GetFeltBounds(FeltMin, FeltMax);

	if (LocalPoint.X < FeltMin.X || LocalPoint.X > FeltMax.X ||
		LocalPoint.Y < FeltMin.Y || LocalPoint.Y > FeltMax.Y)
	{
		return INDEX_NONE;   // off the felt: not a bet, and not an error either
	}

	// Nearest chip location among the positions a filter allows. Used only for the
	// outside bets, which are discrete boxes with no lines to sit on.
	const auto NearestWhere = [this, &LocalPoint](auto&& Filter) -> int32
	{
		int32 BestId = INDEX_NONE;
		float BestDistSq = TNumericLimits<float>::Max();

		for (const FHasardBetPosition& Position : GetPositions())
		{
			if (!Filter(Position))
			{
				continue;
			}

			const float DistSq = FVector2D::DistSquared(LocalPoint, Position.ChipLocation);
			if (DistSq < BestDistSq)
			{
				BestDistSq = DistSq;
				BestId = Position.PositionId;
			}
		}

		return BestId;
	};

	const float GridFarX = GridOrigin.X + NumberColumns * CellSizeX;

	// Region 1 - the bands below the grid. Dozens against the numbers, even-money beyond.
	if (LocalPoint.Y < GridOrigin.Y)
	{
		const bool bDozenBand = LocalPoint.Y > GridOrigin.Y - OutsideBandDepth;

		return NearestWhere([bDozenBand](const FHasardBetPosition& Position)
		{
			if (bDozenBand)
			{
				return Position.BetType == EHasardBetType::Dozen;
			}

			return Position.BetType == EHasardBetType::Low
				|| Position.BetType == EHasardBetType::High
				|| Position.BetType == EHasardBetType::Even
				|| Position.BetType == EHasardBetType::Odd
				|| Position.BetType == EHasardBetType::Red
				|| Position.BetType == EHasardBetType::Black;
		});
	}

	// Region 2 - the three column boxes, past the last cell.
	if (LocalPoint.X > GridFarX)
	{
		return NearestWhere([](const FHasardBetPosition& Position)
		{
			return Position.BetType == EHasardBetType::Column;
		});
	}

	const float FracY = (LocalPoint.Y - GridOrigin.Y) / CellSizeY;
	const int32 Row = FMath::Clamp(FMath::FloorToInt(FracY), 0, NumberRows - 1);
	const float OffsetY = FracY - (static_cast<float>(Row) + 0.5f);

	const bool bNearHorizontal = FMath::Abs(OffsetY) > (0.5f - LineTolerance);
	const bool bFarEdge = bNearHorizontal && OffsetY > 0.0f && Row == NumberRows - 1;

	// Region 3 - the zero seam. A chip on the line x = GridOrigin.X is not a bet on
	// column one: it pairs zero with whatever it is beside. Five different answers
	// share this one line, which is why a plain "is it left of the grid" test is wrong.
	if (FMath::Abs(LocalPoint.X - GridOrigin.X) <= LineTolerance * CellSizeX)
	{
		if (bFarEdge)
		{
			return FindPositionId(EHasardBetType::Basket, { 0, 1, 2, 3 });
		}

		if (bNearHorizontal)
		{
			const int32 AdjacentRow = Row + (OffsetY > 0.0f ? 1 : -1);
			if (AdjacentRow >= 0 && AdjacentRow < NumberRows)
			{
				const int32 LowerRow = FMath::Min(Row, AdjacentRow);
				return FindPositionId(EHasardBetType::Trio,
					{ 0, GetNumberAt(0, LowerRow), GetNumberAt(0, LowerRow + 1) });
			}
		}

		return FindPositionId(EHasardBetType::Split, { 0, GetNumberAt(0, Row) });
	}

	// Region 4 - inside the zero box itself.
	if (LocalPoint.X < GridOrigin.X)
	{
		return FindPositionId(EHasardBetType::StraightUp, { 0 });
	}

	// Region 5 - the number grid.
	const float FracX = (LocalPoint.X - GridOrigin.X) / CellSizeX;
	const int32 Column = FMath::Clamp(FMath::FloorToInt(FracX), 0, NumberColumns - 1);
	const float OffsetX = FracX - (static_cast<float>(Column) + 0.5f);

	const bool bNearVertical = FMath::Abs(OffsetX) > (0.5f - LineTolerance);

	const int32 AdjacentColumn = Column + (OffsetX > 0.0f ? 1 : -1);
	const int32 AdjacentRow = Row + (OffsetY > 0.0f ? 1 : -1);
	const bool bColumnInRange = AdjacentColumn >= 0 && AdjacentColumn < NumberColumns;
	const bool bRowInRange = AdjacentRow >= 0 && AdjacentRow < NumberRows;

	// The far edge carries the streets and the six lines, so it is answered before the
	// corner test. A chip up there is never a corner - there is no fourth cell above it.
	if (bFarEdge)
	{
		if (bNearVertical && bColumnInRange)
		{
			const int32 BaseColumn = FMath::Min(Column, AdjacentColumn);

			TArray<int32> Covered;
			for (int32 Offset = 0; Offset < 2; ++Offset)
			{
				for (int32 CoveredRow = 0; CoveredRow < NumberRows; ++CoveredRow)
				{
					Covered.Add(GetNumberAt(BaseColumn + Offset, CoveredRow));
				}
			}

			return FindPositionId(EHasardBetType::SixLine, Covered);
		}

		TArray<int32> Covered;
		for (int32 CoveredRow = 0; CoveredRow < NumberRows; ++CoveredRow)
		{
			Covered.Add(GetNumberAt(Column, CoveredRow));
		}

		return FindPositionId(EHasardBetType::Street, Covered);
	}

	if (bNearVertical && bNearHorizontal && bColumnInRange && bRowInRange)
	{
		return FindPositionId(EHasardBetType::Corner, {
			GetNumberAt(Column, Row), GetNumberAt(AdjacentColumn, Row),
			GetNumberAt(Column, AdjacentRow), GetNumberAt(AdjacentColumn, AdjacentRow) });
	}

	if (bNearVertical && bColumnInRange)
	{
		return FindPositionId(EHasardBetType::Split,
			{ GetNumberAt(Column, Row), GetNumberAt(AdjacentColumn, Row) });
	}

	if (bNearHorizontal && bRowInRange)
	{
		return FindPositionId(EHasardBetType::Split,
			{ GetNumberAt(Column, Row), GetNumberAt(Column, AdjacentRow) });
	}

	return FindPositionId(EHasardBetType::StraightUp, { GetNumberAt(Column, Row) });
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
	const FVector2D& ChipLocation, const FText& DisplayName, const FVector2D& BoxSize) const
{
	FHasardBetPosition Position;
	Position.PositionId = OutPositions.Num();   // the index is the id, so they cannot drift
	Position.BetType = BetType;
	Position.CoveredNumbers = Covered;
	Position.CoveredNumbers.Sort();
	Position.ChipLocation = ChipLocation;
	Position.DisplayName = DisplayName;
	Position.BoxSize = BoxSize;

	OutPositions.Add(MoveTemp(Position));
}

void UHasardTableLayout::BuildPositions(TArray<FHasardBetPosition>& OutPositions) const
{
	OutPositions.Reset();
	OutPositions.Reserve(157);

	const float GridFarY = GridOrigin.Y + NumberRows * CellSizeY;
	const float ZeroCenterX = GridOrigin.X - ZeroBoxWidth * 0.5f;
	const float ZeroCenterY = GridOrigin.Y + NumberRows * CellSizeY * 0.5f;

	// 1. Zero, then one straight up per cell. Zero's box spans all three rows, which is
	//    why it is the one straight up that is not a single cell.
	AddPosition(OutPositions, EHasardBetType::StraightUp, { 0 },
		FVector2D(ZeroCenterX, ZeroCenterY), LOCTEXT("Zero", "0"),
		FVector2D(ZeroBoxWidth, NumberRows * CellSizeY));

	for (int32 Column = 0; Column < NumberColumns; ++Column)
	{
		for (int32 Row = 0; Row < NumberRows; ++Row)
		{
			const int32 Number = GetNumberAt(Column, Row);
			AddPosition(OutPositions, EHasardBetType::StraightUp, { Number },
				GetCellCenter(Column, Row), FText::AsNumber(Number),
				FVector2D(CellSizeX, CellSizeY));
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
			FText::Format(LOCTEXT("ColumnFmt", "Column {0}"), FText::AsNumber(Row + 1)),
			FVector2D(OutsideBandDepth, CellSizeY));
	}

	// 11. Dozens. Four grid columns each, in the band on the near side.
	for (int32 Dozen = 0; Dozen < 3; ++Dozen)
	{
		TArray<int32> Covered;
		for (int32 Number = Dozen * 12 + 1; Number <= Dozen * 12 + 12; ++Number)
		{
			Covered.Add(Number);
		}

		// "1st 12" rather than "Dozen 1": it is what a real felt prints, and unlike the
		// column boxes it still names which dozen, so the log and the readout keep working.
		static const FText DozenNames[3] = {
			LOCTEXT("Dozen1", "1st 12"), LOCTEXT("Dozen2", "2nd 12"), LOCTEXT("Dozen3", "3rd 12") };

		AddPosition(OutPositions, EHasardBetType::Dozen, Covered,
			FVector2D(GridOrigin.X + (Dozen * 4 + 2) * CellSizeX,
				GridOrigin.Y - OutsideBandDepth * 0.5f),
			DozenNames[Dozen],
			FVector2D(4 * CellSizeX, OutsideBandDepth));
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
			EvenMoneyNames[Index],
			FVector2D(EvenMoneySpan, OutsideBandDepth));
	}
}

#if WITH_EDITOR
void UHasardTableLayout::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	// Any measurement moves every position, so the cache is thrown away rather than
	// patched. Rebuilding 157 entries costs nothing next to getting one of them wrong.
	CachedPositions.Reset();
}
#endif

#undef LOCTEXT_NAMESPACE