// Copyright Picardbuilds. All Rights Reserved.

#include "HasardGameMode.h"
#include "HasardBettingComponent.h"
#include "HasardPayoutTable.h"
#include "HasardPlayerController.h"
#include "HasardPlayerState.h"
#include "HasardWheel.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogHasard, Log, All);

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

	if (!PayoutTable)
	{
		UE_LOG(LogHasard, Error, TEXT("GameMode: no payout table on BP_HasardGameMode"));
		return;
	}

	// The audit. Thirteen rows, one line each, and the edge column is the claim
	// this module exists to make - so the game states it out loud on every run.
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