// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "HasardPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class HASARD_API AHasardPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	/** Real seconds elapsed this session. Unaffected by pause or time dilation. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Session")
	float GetSessionElapsedSeconds() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** How often the reality check interrupts. 15s for observation; 300s once the UI exists. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Session")
	float RealityCheckIntervalSeconds = 15.0f;

	/**
	 * Wall clock at session start, from FPlatformTime::Seconds().
	 * Deliberately not GetTimeSeconds(): world time stops on pause and scales with
	 * dilation, either of which understates how long the player has been here.
	 * double, not float - the engine offsets this value past 2^24 so float precision
	 * degrades to whole seconds.
	 */
	double SessionStartRealTime = 0.0;

	FTimerHandle SessionTimerHandle;

	void RealityCheck();

};
