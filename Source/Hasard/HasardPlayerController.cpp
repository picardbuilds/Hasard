// Copyright Picardbuilds. All Rights Reserved.

#include "HasardPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "HasardBankrollComponent.h"
#include "HasardGameMode.h"
#include "HasardPayoutTable.h"
#include "HasardHUDWidget.h"
#include "HasardPlayerState.h"
#include "HasardSaveGame.h"
#include "HasardTypes.h"

void AHasardPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UHasardHUDWidget>(this, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
			HUDWidget->OnRealityCheckDismissed.AddUObject(
				this, &AHasardPlayerController::HandleRealityCheckDismissed);
			HUDWidget->OnSessionStartChosen.AddUObject(
				this, &AHasardPlayerController::HandleSessionStartChosen);
		}
	}

	// The one figure true of every bet, published once because it never changes -
	// and because it never changing is the entire point of showing it.
	if (HUDWidget)
	{
		const AHasardGameMode* GM = GetWorld()->GetAuthGameMode<AHasardGameMode>();
		const UHasardPayoutTable* Table = GM ? GM->GetPayoutTable() : nullptr;

		float SharedEdge = 0.0f;
		if (Table && Table->TryGetSharedHouseEdge(SharedEdge))
		{
			HUDWidget->SetHouseEdge(SharedEdge * 100.0f);
		}
		else
		{
			HUDWidget->SetHouseEdgeUnknown();
		}
	}

	// Before the player state is looked up, so a misconfigured Player State Class leaves
	// the player looking at a start screen rather than at a table that silently does not
	// keep score. The warnings below still say what is wrong.
	ShowStartScreen();

	AHasardPlayerState* PS = GetPlayerState<AHasardPlayerState>();
	if (!PS)
	{
		UE_LOG(LogHasard, Warning,
			TEXT("Controller: no AHasardPlayerState - check Player State Class"));
		return;
	}

	BoundBankroll = PS->FindComponentByClass<UHasardBankrollComponent>();
	if (!BoundBankroll)
	{
		UE_LOG(LogHasard, Warning, TEXT("Controller: player state has no bankroll component"));
		return;
	}

	BoundBankroll->OnBankrollChanged.AddDynamic(
		this, &AHasardPlayerController::HandleBankrollChanged);

	if (HUDWidget)
	{
		HUDWidget->SetBankroll(BoundBankroll->GetBalance(),
			BoundBankroll->GetSessionStaked(),
			BoundBankroll->GetSessionNetChange());

		HUDWidget->SetLifetime(BoundBankroll->GetLifetimeStaked(),
			BoundBankroll->GetLifetimeNetChange());
	}

	GetWorldTimerManager().SetTimer(ClockTimerHandle, this,
		&AHasardPlayerController::UpdateSessionTime, 1.0f, true);
}

void AHasardPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(ClockTimerHandle);

	if (HUDWidget)
	{
		HUDWidget->OnRealityCheckDismissed.RemoveAll(this);
		HUDWidget->OnSessionStartChosen.RemoveAll(this);
	}

	if (BoundBankroll)
	{
		BoundBankroll->OnBankrollChanged.RemoveDynamic(
			this, &AHasardPlayerController::HandleBankrollChanged);
		BoundBankroll = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AHasardPlayerController::ShowStartScreen()
{
	if (!HUDWidget)
	{
		return;
	}

	LoadedSave = Cast<UHasardSaveGame>(UGameplayStatics::LoadGameFromSlot(
		UHasardSaveGame::SlotName, UHasardSaveGame::UserIndex));

	if (LoadedSave && LoadedSave->Version != UHasardSaveGame::CurrentVersion)
	{
		// Refused rather than migrated. A field whose meaning changed would make
		// LifetimeStaked a number about something else, and no guess is honest.
		UE_LOG(LogHasard, Warning,
			TEXT("Save is a version of %d, this build reads %d - ignoring it"),
			LoadedSave->Version, UHasardSaveGame::CurrentVersion);

		LoadedSave = nullptr;
	}

	if (StartScreenContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = 
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(StartScreenContext, 20);
		}
	}

	SetShowMouseCursor(true);
	SetInputMode(FInputModeGameAndUI()
		.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock)
		.SetHideCursorDuringCapture(false));

	HUDWidget->ShowStartScreen(LoadedSave != nullptr,
		LoadedSave ? LoadedSave->Balance : 0,
		LoadedSave ? LoadedSave->LifetimeStaked : 0,
		LoadedSave ? LoadedSave->LifetimeWon - LoadedSave->LifetimeStaked : 0);

}

void AHasardPlayerController::HandleSessionStartChosen(bool bContinuePrevious)
{
	if (StartScreenContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(StartScreenContext);
		}
	}

	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());

	if (HUDWidget)
	{
		HUDWidget->HideStartScreen();
	}

	// No save is not an error. It is a first sitting, and the defaults already describe
	// one - which is why there is no separate branch here for it.
	if (LoadedSave && BoundBankroll)
	{
		BoundBankroll->ApplySave(*LoadedSave, bContinuePrevious);
	}

	if (AHasardPlayerState* PS = GetPlayerState<AHasardPlayerState>())
	{
		PS->StartSession(LoadedSave ? LoadedSave->LifetimeSeconds : 0.0f);
	}

	UE_LOG(LogHasard, Warning, TEXT("Sessions started -%s"),
		bContinuePrevious ? TEXT("continuing the last one") : TEXT("new"));
}

void AHasardPlayerController::HandleBankrollChanged(int32 NewBalance, int32 Delta,
	int32 SessionNetChange)
{
	UE_LOG(LogHasard, Warning, TEXT("Bankroll: balance %d (%+d), session net %+d"),
		NewBalance, Delta, SessionNetChange);

	if (HUDWidget && BoundBankroll)
	{
		HUDWidget->SetBankroll(NewBalance, BoundBankroll->GetSessionStaked(), SessionNetChange);

		HUDWidget->SetLifetime(BoundBankroll->GetLifetimeStaked(),
			BoundBankroll->GetLifetimeNetChange());
	}
}

void AHasardPlayerController::UpdateSessionTime()
{
	if (!HUDWidget)
	{
		return;
	}

	// The same accessor the log uses, so the clock and the log cannot disagree.
	if (const AHasardPlayerState* PS = GetPlayerState<AHasardPlayerState>())
	{
		HUDWidget->SetSessionTime(PS->GetSessionElapsedSeconds());
	}
}

void AHasardPlayerController::SetBetPreview(const FText& PreviewText)
{
	if (HUDWidget)
	{
		HUDWidget->SetBetPreview(PreviewText);
	}
}

void AHasardPlayerController::ClearBetPreview()
{
	if (HUDWidget)
	{
		HUDWidget->ClearBetPreview();
	}
}

void AHasardPlayerController::HandleRealityCheck(int32 MinutesElapsed)
{
	if (!HUDWidget || !BoundBankroll)
	{
		return;
	}

	if (RealityCheckContext)
	{
		// Suppress table input while the check is up - Module 6's contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(RealityCheckContext, 10);
		}
	}

	// The context stops gameplay input; this hands the mouse to Slate so the
	// dismiss button can actually be clicked. Two different jobs.
	SetShowMouseCursor(true);
	SetInputMode(FInputModeGameAndUI()
		.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock)
		.SetHideCursorDuringCapture(false));

	HUDWidget->ShowRealityCheck(MinutesElapsed,
		BoundBankroll->GetSessionStaked(),
		BoundBankroll->GetSessionNetChange());
}

void AHasardPlayerController::HandleRealityCheckDismissed()
{
	if (RealityCheckContext)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(RealityCheckContext);
		}
	}

	SetShowMouseCursor(false);
	SetInputMode(FInputModeGameOnly());
}