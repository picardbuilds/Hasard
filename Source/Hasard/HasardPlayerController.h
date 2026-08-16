// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/PlayerController.h"
#include "HasardPlayerController.generated.h"

class UHasardBankrollComponent;
class UHasardHUDWidget;
class UInputMappingContext;

UCLASS()
class HASARD_API AHasardPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	/** Called by the PlayerState's reality-check timer. */
	void HandleRealityCheck(int32 MinutesElapsed);

	/**
	 * Called by the felt as the aim enters and leaves a bet position.
	 *
	 * The felt does not reach the widget itself. Everything the HUD shows arrives
	 * through here, so there is one object that decides what the player is told.
	 */
	void SetBetPreview(const FText& PreviewText);
	void ClearBetPreview();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Bound to OnBankrollChanged in BeginPlay. AddDynamic resolves this by reflected name. */
	UFUNCTION()
	void HandleBankrollChanged(int32 NewBalance, int32 Delta, int32 SessionNetChange);

	/** Bound to the widget's OnRealityCheckDismissed. Pops the suppression context. */
	void HandleRealityCheckDismissed();

	/** Repeating timer callback. One update a second, because that is the resolution shown. */
	void UpdateSessionTime();

	/** Cached so EndPlay unbinds from the same object BeginPlay bound to. */
	UPROPERTY()
	TObjectPtr<UHasardBankrollComponent> BoundBankroll;

	UPROPERTY(EditDefaultsOnly, Category = "Hasard|UI")
	TSubclassOf<UHasardHUDWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UHasardHUDWidget> HUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Hasard|UI")
	TObjectPtr<UInputMappingContext> RealityCheckContext;

	FTimerHandle ClockTimerHandle;
};