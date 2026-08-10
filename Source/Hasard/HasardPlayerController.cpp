// Copyright Picardbuilds. All Rights Reserved.

#include "HasardPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "InputMappingContext.h"
#include "TimerManager.h"
#include "HasardBankrollComponent.h"
#include "HasardHUDWidget.h"
#include "HasardPlayerState.h"

DEFINE_LOG_CATEGORY_STATIC(LogHasard, Log, All);

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
		}
	}

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
			BoundBankroll->GetTotalStaked(),
			BoundBankroll->GetSessionNetChange());
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
	}

	if (BoundBankroll)
	{
		BoundBankroll->OnBankrollChanged.RemoveDynamic(
			this, &AHasardPlayerController::HandleBankrollChanged);
		BoundBankroll = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void AHasardPlayerController::HandleBankrollChanged(int32 NewBalance, int32 Delta,
	int32 SessionNetChange)
{
	UE_LOG(LogHasard, Warning, TEXT("Bankroll: balance %d (%+d), session net %+d"),
		NewBalance, Delta, SessionNetChange);

	if (HUDWidget && BoundBankroll)
	{
		HUDWidget->SetBankroll(NewBalance, BoundBankroll->GetTotalStaked(), SessionNetChange);
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
		BoundBankroll->GetTotalStaked(),
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