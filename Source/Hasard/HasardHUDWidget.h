// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HasardHUDWidget.generated.h"

class UTextBlock;

/**
 * Broadcast when the player dismisses the reality check.
 * Non-dynamic: only C++ binds to it, so there is no reason to pay for reflection.
 */
DECLARE_MULTICAST_DELEGATE(FOnRealityCheckDismissed);

/** True to continue the last sitting, false to start a new one. Same reasoning: C++ only. */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSessionStartChosen, bool bContinuePrevious);

/**
 * The HUD's C++ half. Layout and styling are authored in WBP_HUD; this class owns
 * the contract between the controller and those widgets.
 *
 * Abstract, because the BindWidget properties can only be satisfied by a
 * Blueprint subclass. Instantiating this class directly would leave them null.
 */
UCLASS(Abstract)
class HASARD_API UHasardHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Bound by the controller in BeginPlay, unbound in EndPlay. */
	FOnRealityCheckDismissed OnRealityCheckDismissed;

	/** Same. The controller cannot start the session until this says how. */
	FOnSessionStartChosen OnSessionStartChosen;

	void SetBankroll(int32 Balance, int32 SessionStaked, int32 SessionNetChange);

	void SetSessionTime(float ElapsedSeconds);

	/**
	 * What every sitting has cost, this one included.
	 *
	 * Beside the session figures rather than replacing them. Either one alone is a true
	 * number that leaves a false impression - the session hides what came before it, and
	 * the lifetime hides what is happening right now.
	 */
	void SetLifetime(int32 LifetimeStaked, int32 LifetimeNetChange);

	/** Blueprint owns the panel entirely - C++ never touches layout. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Hasard|UI")
	void ShowRealityCheck(int32 MinutesElapsed, int32 TotalStaked, int32 SessionNetChange);

	/** The panel's dismiss button calls this; the controller pops the context. */
	UFUNCTION(BlueprintCallable, Category = "Hasard|UI")
	void NotifyRealityCheckDismissed();

	/**
	 * The start screen, with last sitting's figures so the choice is an informed one.
	 *
	 * bHasSave false means there is nothing to continue, and the panel is expected to
	 * offer only a new session rather than a disabled button the player can wonder about.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Hasard|UI")
	void ShowStartScreen(bool bHasSave, int32 SavedBalance, int32 LifetimeStaked,
		int32 LifetimeNetChange);

	UFUNCTION(BlueprintImplementableEvent, Category = "Hasard|UI")
	void HideStartScreen();

	/** Both start-screen buttons call this; the bool is which one. */
	UFUNCTION(BlueprintCallable, Category = "Hasard|UI")
	void NotifySessionStartChosen(bool bContinuePrevious);

	/** The one figure that is true of every bet on the table. */
	void SetHouseEdge(float EdgePercent);

	/** Rows disagree, so no single figure is true. Say that rather than quoting one. */
	void SetHouseEdgeUnknown();

	/** The bet under the aim, priced. Written every time the aim moves to a new position. */
	void SetBetPreview(const FText& InPreviewText);

	/** No bet under the aim. Blank, never the last price. */
	void ClearBetPreview();

private:
	/** Each name must match a Text Block of that name in WBP_HUD, or the Blueprint refuses to compile. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> BalanceText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StakedText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NetText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TimeText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> OddsText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PreviewText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> LifetimeText;
};
