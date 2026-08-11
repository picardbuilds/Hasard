// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HasardTypes.generated.h"

/** One category for the whole project. Defined once, in HasardGameMode.cpp. */
DECLARE_LOG_CATEGORY_EXTERN(LogHasard, Log, All);

UENUM(BlueprintType)
enum class EHasardBetType : uint8
{
	StraightUp  UMETA(DisplayName = "Straight Up"),
	Split       UMETA(DisplayName = "Split"),
	Street      UMETA(DisplayName = "Street"),
	Corner      UMETA(DisplayName = "Corner"),
	SixLine     UMETA(DisplayName = "Six Line"),
	Column      UMETA(DisplayName = "Column"),
	Dozen       UMETA(DisplayName = "Dozen"),
	Red         UMETA(DisplayName = "Red"),
	Black       UMETA(DisplayName = "Black"),
	Even        UMETA(DisplayName = "Even"),
	Odd         UMETA(DisplayName = "Odd"),
	Low         UMETA(DisplayName = "Low 1-18"),
	High        UMETA(DisplayName = "High 19-36")
};

USTRUCT(BlueprintType)
struct FHasardBet
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Bet")
	EHasardBetType BetType = EHasardBetType::StraightUp;

	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Bet")
	int32 PrimaryNumber = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Bet")
	int32 Stake = 0;
};