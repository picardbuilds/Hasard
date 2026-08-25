// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/PlayerController.h"
#include "HasardPlayerController.generated.h"

class UHasardBankrollComponent;
class UHasardHUDWidget;
class UHasardSaveGame;
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

	/** Reads the slot, then puts the choice on screen with last sitting's figures on it. */
	void ShowStartScreen();

	/** Bound to the widget's OnSessionStartChosen. Nothing plays until this has run. */
	void HandleSessionStartChosen(bool bContinuePrevious);

	/** Repeating timer callback. One update a second, because that is the resolution shown. */
	void UpdateSessionTime();

	/**
	 * Writes the record to the slot.
	 *
	 * Cheap and safe to call often - four places do. It refuses rather than writing a
	 * record with a hole in it, and it writes nothing at all before the player has
	 * chosen how the sitting begins.
	 */
	void CommitSave();

	/** Cached so EndPlay unbinds from the same object BeginPlay bound to. */
	UPROPERTY()
	TObjectPtr<UHasardBankrollComponent> BoundBankroll;

	UPROPERTY(EditDefaultsOnly, Category = "Hasard|UI")
	TSubclassOf<UHasardHUDWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<UHasardHUDWidget> HUDWidget;

	/** What the slot held, or null on a first run or a version this build will not read. */
	UPROPERTY()
	TObjectPtr<UHasardSaveGame> LoadedSave;

	UPROPERTY(EditDefaultsOnly, Category = "Hasard|UI")
	TObjectPtr<UInputMappingContext> RealityCheckContext;

	/**
	 * Pushed above the reality check while the start screen is up.
	 *
	 * The same blocking trick as IMC_RealityCheck, one priority higher, because the two
	 * can overlap: the reality-check timer does not start until the session does, but a
	 * context left at the same priority would be a race worth not having.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Hasard|UI")
	TObjectPtr<UInputMappingContext> StartScreenContext;

	FTimerHandle ClockTimerHandle;
};