// Copyright Picardbuilds. All Rights Reserved.

#include "HasardPlayerState.h"
#include "HasardBankrollComponent.h"
#include "HasardTypes.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "HasardPlayerController.h"

AHasardPlayerState::AHasardPlayerState()
{
	BankrollComp = CreateDefaultSubobject<UHasardBankrollComponent>(TEXT("BankrollComp"));
}

void AHasardPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AHasardPlayerState::StartSession(float InPriorSeconds)
{
	PriorSeconds = InPriorSeconds;

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
	// Before StartSession the stamp is still zero, and FPlatformTime::Seconds() is offset
	// past 2^24 on Windows - so without this the HUD would open on about 194 days played.
	if (SessionStartRealTime == 0.0)
	{
		return 0.0f;
	}

	return static_cast<float>(FPlatformTime::Seconds() - SessionStartRealTime);
}

void AHasardPlayerState::RealityCheck()
{
	UE_LOG(LogHasard, Warning, TEXT("REALITY CHECK - %.0f seconds this session"),
		GetSessionElapsedSeconds());

	if (AHasardPlayerController* PC = Cast<AHasardPlayerController>(GetPlayerController()))
	{
		PC->HandleRealityCheck(FMath::FloorToInt(GetSessionElapsedSeconds() / 60.0f));
	}
}