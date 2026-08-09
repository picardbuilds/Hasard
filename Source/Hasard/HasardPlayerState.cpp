// Copyright Picardbuilds. All Rights Reserved.

#include "HasardPlayerState.h"
#include "HasardBankrollComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogHasard, Log, All);

AHasardPlayerState::AHasardPlayerState()
{
	BankrollComp = CreateDefaultSubobject<UHasardBankrollComponent>(TEXT("BankrollComp"));
}

void AHasardPlayerState::BeginPlay()
{
	Super::BeginPlay();

	SessionStartRealTime = FPlatformTime::Seconds();

	GetWorldTimerManager().SetTimer(
		SessionTimerHandle, this, &AHasardPlayerState::RealityCheck,
		RealityCheckIntervalSeconds, true);
}

void AHasardPlayerState::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SessionTimerHandle);
	Super::EndPlay(EndPlayReason);
}

float AHasardPlayerState::GetSessionElapsedSeconds() const
{
	return static_cast<float>(FPlatformTime::Seconds() - SessionStartRealTime);
}

void AHasardPlayerState::RealityCheck()
{
	UE_LOG(LogHasard, Warning, TEXT("REALITY CHECK - %.0f seconds this session"),
		GetSessionElapsedSeconds());
}