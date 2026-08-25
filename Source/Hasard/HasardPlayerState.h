// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/PlayerState.h"
#include "HasardPlayerState.generated.h"

class UHasardBankrollComponent;

UCLASS()
class HASARD_API AHasardPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AHasardPlayerState();

	/**
	 * Starts the clock and the reality check. Called once the player has chosen how this
	 * sitting begins, never from BeginPlay.
	 *
	 * InPriorSeconds is what previous sittings already cost, carried in from the save so
	 * the lifetime figure can be reported without the session clock ever being wound
	 * forward - those are two different claims and only one of them is about right now.
	 */
	void StartSession(float InPriorSeconds);

	/** Real seconds elapsed this session. Unaffected by pause or time dilation. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Session")
	float GetSessionElapsedSeconds() const;

	/** Every sitting including this one. What the save records. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Session")
	float GetLifeTimeSeconds() const { return PriorSeconds + GetSessionElapsedSeconds(); }

	/**
	 * True once StartSession has stamped the clock.
	 *
	 * Reads the stamp rather than keeping a bool beside it. Module 9 declined that bool
	 * for the guard inside GetSessionElapsedSeconds, and the reason is unchanged: the
	 * stamp already answers this question, and two answers can disagree.
	 */
	UFUNCTION(BlueprintPure, Category = "Hasard|Session")
	bool IsSessionStarted() const { return SessionStartRealTime != 0.0; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** The bankroll. UPROPERTY, or the collector cannot see it. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hasard|Bankroll",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHasardBankrollComponent> BankrollComp;

	/** How often the reality check interrupts. 15s for observation; 300s once the UI exists. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Session",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0", UIMin = "15.0", UIMax = "600.0"))
	float RealityCheckIntervalSeconds = 15.0f;

	/** Wall clock at session start. Deliberately not world time - Module 4. */
	double SessionStartRealTime = 0.0;

	/** Seconds carried in from the save. Read only for the lifetime figure. */
	float PriorSeconds = 0.0f;

	/** Plain struct, not a UObject: nothing for the collector to track. */
	FTimerHandle SessionTimerHandle;

	void RealityCheck();
};
