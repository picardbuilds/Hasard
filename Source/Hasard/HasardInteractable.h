// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HasardInteractable.generated.h"

class APawn;

UINTERFACE(MinimalAPI)
class UHasardInteractable : public UInterface
{
	GENERATED_BODY()
};

class HASARD_API IHasardInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Hasard|Interaction")
	void OnPlayerInteract(APawn* InstigatorPawn);
};
