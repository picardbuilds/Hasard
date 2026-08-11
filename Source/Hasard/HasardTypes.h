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
	Trio        UMETA(DisplayName = "Trio"),
	Corner      UMETA(DisplayName = "Corner"),
	Basket      UMETA(DisplayName = "Basket"),
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

/**
 * One place a chip can legally sit, and everything that follows from it.
 *
 * Generated from the layout asset, never authored by hand. A bet stores the
 * PositionId and nothing else, so a bet cannot describe a position that does not exist.
 */
USTRUCT(BlueprintType)
struct FHasardBetPosition
{
	GENERATED_BODY()

	/** Index into the generated array. Stable for a given layout asset. */
	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Layout")
	int32 PositionId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Layout")
	EHasardBetType BetType = EHasardBetType::StraightUp;

	/** Every pocket this position pays on, ascending. Its length decides the payout. */
	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Layout")
	TArray<int32> CoveredNumbers;

	/** Where the chip sits, in the felt's local 2D space, in centimeters. */
	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Layout")
	FVector2D ChipLocation = FVector2D::ZeroVector;

	/** What the hover readout shows the player. FText because a player reads it. */
	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Layout")
	FText DisplayName;
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