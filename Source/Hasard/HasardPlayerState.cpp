// Fill out your copyright notice in the Description page of Project Settings.


#include "HasardPlayerState.h"
#include "TimerManager.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogHasard, Log, All);

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
	UE_LOG(LogHasard, Warning, TEXT("REALITY CHECK - %.0f seconds this session"), GetSessionElapsedSeconds());
}