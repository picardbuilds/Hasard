// Copyright Picardbuilds. All Rights Reserved.

#include "HasardGameMode.h"
#include "HasardBettingComponent.h"
#include "HasardPayoutTable.h"
#include "HasardPlayerController.h"
#include "HasardPlayerState.h"
#include "HasardTableLayout.h"
#include "HasardTypes.h"
#include "HasardWheel.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY(LogHasard);

AHasardGameMode::AHasardGameMode()
{
	PlayerControllerClass = AHasardPlayerController::StaticClass();
	PlayerStateClass = AHasardPlayerState::StaticClass();
}

void AHasardGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogHasard, Warning, TEXT("GameMode BeginPlay - phase %s, betting window: %.1fs"),
		*UEnum::GetValueAsString(CurrentPhase), BettingWindowSeconds);

	AActor* Found = UGameplayStatics::GetActorOfClass(GetWorld(), AHasardWheel::StaticClass());

	if (AHasardWheel* Wheel = Cast<AHasardWheel>(Found))
	{
		Wheel->OnBallSettled.AddDynamic(this, &AHasardGameMode::HandleBallSettled);
		BoundWheel = Wheel;
	}
	else
	{
		UE_LOG(LogHasard, Error, TEXT("GameMode: no wheel in the level - rounds will never resolve"));
	}

	if (!TableLayout)
	{
		UE_LOG(LogHasard, Error, TEXT("GameMode: no table layout on BP_HasardGameMode"));
	}

	if (!PayoutTable)
	{
		UE_LOG(LogHasard, Error, TEXT("GameMode: no payout table on BP_HasardGameMode"));
		return;
	}

	// The audit. Fifteen rows, one line each, and the edge column is the claim
	// this project exists to make - so the game states it out loud on every run.
	for (const FHasardPayoutRule& Rule : PayoutTable->Rules)
	{
		UE_LOG(LogHasard, Warning,
			TEXT("Payout audit: %s pays %d:1, covers %d/%d, true odds %.2f:1, edge %.2f%%"),
			*Rule.DisplayName.ToString(), Rule.PayoutRatio, Rule.CoveredNumbers,
			PayoutTable->PocketCount, PayoutTable->GetTrueOdds(Rule),
			PayoutTable->GetHouseEdge(Rule) * 100.0f);
	}

	float SharedEdge = 0.0f;
	if (!PayoutTable->TryGetSharedHouseEdge(SharedEdge))
	{
		UE_LOG(LogHasard, Error,
			TEXT("Payout audit: the rows do not share one edge. A row is wrong."));
	}

	// Two objects hold a pocket count. The wheel is the physical rim; the payout table is
	// what the edge was priced against. They are genuinely separate facts - pointing the
	// GameMode at an American table is a thing a designer may legitimately do - so they
	// are allowed to differ. They are not allowed to differ *silently*, because the figure
	// on the HUD is computed from one of them and paid out against the other.
	if (BoundWheel && BoundWheel->GetPocketCount() != PayoutTable->PocketCount)
	{
		UE_LOG(LogHasard, Error,
			TEXT("Pocket missmatch: the wheel has %d pockets, the payout table is priced for %d"),
			BoundWheel->GetPocketCount(), PayoutTable->PocketCount);
	}
}

void AHasardGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// The rule the PlayerController already follows: whoever binds, unbinds.
	if (BoundWheel)
	{
		BoundWheel->OnBallSettled.RemoveDynamic(this, &AHasardGameMode::HandleBallSettled);
		BoundWheel = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AHasardGameMode::HandleBallSettled(int32 WinningPocket)
{
	UE_LOG(LogHasard, Warning, TEXT("Ball settled in pocket %d"), WinningPocket);
	ResolveRound(WinningPocket);
}

void AHasardGameMode::ResolveRound(int32 WinningPocket)
{
	CurrentPhase = EHasardRoundPhase::Settling;

	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	UHasardBettingComponent* Betting =
		PlayerPawn ? PlayerPawn->FindComponentByClass<UHasardBettingComponent>() : nullptr;

	if (!PayoutTable)
	{
		UE_LOG(LogHasard, Error, TEXT("ResolveRound: no payout table on BP_HasardGameMode"));
	}
	else if (!TableLayout)
	{
		UE_LOG(LogHasard, Error, TEXT("ResolveRound: no table layout on BP_HasardGameMode"));
	}
	else if (!Betting)
	{
		UE_LOG(LogHasard, Warning, TEXT("ResolveRound: no betting component to settle"));
	}
	else
	{
		Betting->SettleRound(WinningPocket, PayoutTable, TableLayout);
	}

	// Whatever happened above, the table reopens. A round that cannot settle must
	// not strand the game in Settling forever.
	CurrentPhase = EHasardRoundPhase::Betting;
}

void AHasardGameMode::HasardTestDistribution(int32 SpinCount)
{
	if (SpinCount <= 0)
	{
		UE_LOG(LogHasard, Error, TEXT("SpinCount must be positive"));
		return;
	}

	AHasardWheel* Wheel = Cast<AHasardWheel>(
		UGameplayStatics::GetActorOfClass(this, AHasardWheel::StaticClass()));
	if (!Wheel)
	{
		UE_LOG(LogHasard, Error, TEXT("No wheel in the level"));
		return;
	}

	const int32 PocketCount = Wheel->GetPocketCount();

	TArray<int32> Counts;
	Counts.Init(0, PocketCount);

	for (int32 i = 0; i < SpinCount; ++i)
	{
		Counts[Wheel->DetermineWinningPocket()]++;
	}

	const float Expected = static_cast<float>(SpinCount) / PocketCount;

	for (int32 Pocket = 0; Pocket < PocketCount; ++Pocket)
	{
		const float Deviation = (Counts[Pocket] - Expected) / Expected * 100.0f;
		UE_LOG(LogHasard, Display, TEXT("Pocket %2d: %5d hits (%+.1f%% from expected)"),
			Pocket, Counts[Pocket], Deviation);
	}
}

void AHasardGameMode::HasardShowCell(int32 Number)
{
	if (!TableLayout)
	{
		UE_LOG(LogHasard, Error, TEXT("No table layout on BP_HasardGameMode"));
		return;
	}

	if (Number == 0)
	{
		UE_LOG(LogHasard, Display, TEXT("0 has its own box beside the grid, so it has no column or row"));
		return;
	}

	const int32 Column = TableLayout->GetColumnOf(Number);
	const int32 Row = TableLayout->GetRowOf(Number);

	if (Column == INDEX_NONE || Row == INDEX_NONE)
	{
		UE_LOG(LogHasard, Error, TEXT("%d is not a number on this grid"), Number);
		return;
	}

	const FVector2D Center = TableLayout->GetCellCenter(Column, Row);

	UE_LOG(LogHasard, Display, TEXT("%d is column %d, row %d - center (%.1f, %.1f), %s"),
		Number, Column, Row, Center.X, Center.Y,
		TableLayout->IsRedNumber(Number) ? TEXT("red") : TEXT("black"));
}

void AHasardGameMode::HasardAuditLayout()
{
	if (!TableLayout || !PayoutTable)
	{
		UE_LOG(LogHasard, Error,
			TEXT("Assign both TableLayout and PayoutTable on BP_HasardGameMode"));
		return;
	}

	TArray<FHasardBetPosition> Positions;
	TableLayout->BuildPositions(Positions);

	// The claim being tested: every position on a single-zero table pays 2.70% to the house.
	const float ExpectedEdge = 1.0f / static_cast<float>(PayoutTable->PocketCount);

	TMap<EHasardBetType, int32> CountByType;
	int32 Offenders = 0;

	for (const FHasardBetPosition& Position : Positions)
	{
		CountByType.FindOrAdd(Position.BetType)++;

		const FHasardPayoutRule* Rule = PayoutTable->FindRule(Position.BetType);
		if (!Rule)
		{
			UE_LOG(LogHasard, Error, TEXT("Position %d (%s): no payout row for this bet type"),
				Position.PositionId, *Position.DisplayName.ToString());
			++Offenders;
			continue;
		}

		// Two different CoveredNumbers: the position holds the actual numbers,
		// the payout row holds how many there are meant to be. They must agree.
		if (Position.CoveredNumbers.Num() != Rule->CoveredNumbers)
		{
			UE_LOG(LogHasard, Error,
				TEXT("Position %d (%s): covers %d numbers, payout row expects %d"),
				Position.PositionId, *Position.DisplayName.ToString(),
				Position.CoveredNumbers.Num(), Rule->CoveredNumbers);
			++Offenders;
			continue;
		}

		if (!FMath::IsNearlyEqual(PayoutTable->GetHouseEdge(*Rule), ExpectedEdge, 0.0001f))
		{
			UE_LOG(LogHasard, Error, TEXT("Position %d (%s): edge %.2f%%, expected %.2f%%"),
				Position.PositionId, *Position.DisplayName.ToString(),
				PayoutTable->GetHouseEdge(*Rule) * 100.0f, ExpectedEdge * 100.0f);
			++Offenders;
		}
	}

	// The printed felt is generated from BoxSize, so a layout that overlaps two boxes
	// draws one over the other and looks merely odd. The same numbers feed ResolvePosition,
	// which means the odd-looking cell is also the cell whose clicks land somewhere else -
	// and that failure is silent. Checked here rather than in the felt because it is a
	// property of the layout asset, true or false before anything is drawn.
	FVector2D FeltMin, FeltMax;
	TableLayout->GetFeltBounds(FeltMin, FeltMax);

	// Neighboring boxes share an edge exactly. Shrink both sides of every comparison so
	// touching is not reported as overlapping - only a real intersection is.
	const float Shrink = 0.01f;
	int32 Boxes = 0;
	int32 BoxOffenders = 0;

	for (const FHasardBetPosition& Position : Positions)
	{
		if (Position.BoxSize.IsNearlyZero())
		{
			continue;
		}

		++Boxes;

		const FVector2D Min = Position.ChipLocation - Position.BoxSize * 0.5f;
		const FVector2D Max = Position.ChipLocation + Position.BoxSize * 0.5f;

		if (Min.X < FeltMin.X - Shrink || Min.Y < FeltMin.Y - Shrink
			|| Max.X > FeltMax.X + Shrink || Max.Y > FeltMax.Y + Shrink)
		{
			UE_LOG(LogHasard, Error,
				TEXT("Position %d (%s): box runs outside the felt bounds"),
				Position.PositionId, *Position.DisplayName.ToString());
			++BoxOffenders;
		}

		for (const FHasardBetPosition& Other : Positions)
		{
			// Ids ascend with the array, so the pair is only ever tested once.
			if (Other.PositionId <= Position.PositionId || Other.BoxSize.IsNearlyZero())
			{
				continue;
			}

			const FVector2D OtherMin = Other.ChipLocation - Other.BoxSize * 0.5f;
			const FVector2D OtherMax = Other.ChipLocation + Other.BoxSize * 0.5f;

			const bool bOverlaps =
				Min.X < OtherMax.X - Shrink && Max.X > OtherMin.X + Shrink &&
				Min.Y < OtherMax.Y - Shrink && Max.Y > OtherMin.Y + Shrink;

			if (bOverlaps)
			{
				UE_LOG(LogHasard, Error, TEXT("Positions %d (%s) and %d (%s): boxes overlap"),
					Position.PositionId, *Position.DisplayName.ToString(),
					Other.PositionId, *Other.DisplayName.ToString());
				++BoxOffenders;
			}
		}
	}

	for (const TPair<EHasardBetType, int32>& Pair : CountByType)
	{
		UE_LOG(LogHasard, Display, TEXT("%s: %d positions"),
			*UEnum::GetDisplayValueAsText(Pair.Key).ToString(), Pair.Value);
	}

	UE_LOG(LogHasard, Display, TEXT("%d positions generated, %d failing the edge check"),
		Positions.Num(), Offenders);

	UE_LOG(LogHasard, Display, TEXT("%d of them are printed boxes, %d failing the box check"),
		Boxes, BoxOffenders);
}

void AHasardGameMode::HasardShowPocket(int32 Number)
{
	AHasardWheel* Wheel = Cast<AHasardWheel>(
		UGameplayStatics::GetActorOfClass(this, AHasardWheel::StaticClass()));

	if (!Wheel)
	{
		UE_LOG(LogHasard, Error, TEXT("no wheel in level"));
		return;
	}

	const int32 RimIndex = Wheel->GetRimIndexOf(Number);

	if (RimIndex == INDEX_NONE)
	{
		UE_LOG(LogHasard, Error, TEXT("%d is not a pocket on this wheel"), Number);
		return;
	}

	float Degrees = 0.0f;
	Wheel->TryGetPocketAngleDegrees(Number, Degrees);

	const int32 Count = Wheel->GetPocketCount();

	// The two neighbours are printed because they are how you check this against a
	// photograph of a real wheel - and because they are the two numbers a near-miss
	// animation would want to linger on. Worth knowing their names early.
	const int32 Before = Wheel->GetNumberAtRimIndex((RimIndex + Count - 1) % Count);
	const int32 After = Wheel->GetNumberAtRimIndex((RimIndex + 1) % Count);

	UE_LOG(LogHasard, Display,
		TEXT("%d is rim slot %d at %.2f degrees, between %d and %d"),
		Number, RimIndex, Degrees, Before, After);
}