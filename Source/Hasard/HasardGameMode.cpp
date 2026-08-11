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
	else if (!Betting)
	{
		UE_LOG(LogHasard, Warning, TEXT("ResolveRound: no betting component to settle"));
	}
	else
	{
		Betting->SettleRound(WinningPocket, PayoutTable);
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