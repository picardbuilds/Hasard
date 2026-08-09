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

	/** Real seconds elapsed this session. Unaffected by pause or time dilation. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Session")
	float GetSessionElapsedSeconds() const;

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

	/** Plain struct, not a UObject: nothing for the collector to track. */
	FTimerHandle SessionTimerHandle;

	void RealityCheck();
};
