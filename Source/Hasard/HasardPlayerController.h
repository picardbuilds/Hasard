// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HasardPlayerController.generated.h"

class UHasardBankrollComponent;

UCLASS()
class HASARD_API AHasardPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Bound to OnBankrollChanged in BeginPlay. AddDynamic resolves this by reflected name. */
	UFUNCTION()
	void HandleBankrollChanged(int32 NewBalance, int32 Delta, int32 SessionNetChange);

	/** Cached so EndPlay unbinds from the same object BeginPlay bound to. */
	UPROPERTY()
	TObjectPtr<UHasardBankrollComponent> BoundBankroll;
};


