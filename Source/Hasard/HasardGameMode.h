// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HasardGameMode.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EHasardRoundPhase : uint8
{
	Betting UMETA(DisplayName = "Betting"),
	Spinning UMETA(DisplayName = "Spinning"),
	Settling UMETA(DisplayName = "Settling")
};


UCLASS()
class HASARD_API AHasardGameMode : public AGameModeBase
{
	GENERATED_BODY()

public: 
	AHasardGameMode();

protected: 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hasard|Round")
	EHasardRoundPhase CurrentPhase = EHasardRoundPhase::Betting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Round")
	float BettingWindowSeconds = 30.0f;

	virtual void BeginPlay() override;
	
};
