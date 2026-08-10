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

/**
 * Abstract, because the four BindWidget properties can only be satisfied by a
 * Blueprint subclass. Instantiating this class directly would leave them null.
 */
UCLASS(Abstract)
class HASARD_API UHasardHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Bound by the controller in BeginPlay, unbound in EndPlay. */
	FOnRealityCheckDismissed OnRealityCheckDismissed;

	void SetBankroll(int32 Balance, int32 TotalStaked, int32 SessionNetChange);

	void SetSessionTime(float ElapsedSeconds);

	/** Blueprint owns the panel entirely - C++ never touches layout. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Hasard|UI")
	void ShowRealityCheck(int32 MinutesElapsed, int32 TotalStaked, int32 SessionNetChange);

	/** The panel's dismiss button calls this; the controller pops the context. */
	UFUNCTION(BlueprintCallable, Category = "Hasard|UI")
	void NotifyRealityCheckDismissed();

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
};
