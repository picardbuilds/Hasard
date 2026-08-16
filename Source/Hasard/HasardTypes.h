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

	/**
	 * Index into the generated array, stamped from its own position on append.
	 * The array is append-only for that reason: a sort or a remove leaves every
	 * id pointing at the wrong position, and nothing would report it.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Layout")
	int32 PositionId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Layout")
	EHasardBetType BetType = EHasardBetType::StraightUp;

	/** Every pocket this position pays on, ascending. The audit checks this count against the payout rule. */
	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Layout")
	TArray<int32> CoveredNumbers;

	/** Where the chip sits, in the felt's local 2D space, in centimeters. */
	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Layout")
	FVector2D ChipLocation = FVector2D::ZeroVector;

	/**
	 * The rectangle this position is printed as, centered on ChipLocation, in centimeters.
	 *
	 * Zero for the 108 line bets, and that is the test rather than a separate flag: a split
	 * sits on a boundary and a corner on an intersection, so neither owns any area to print.
	 * "Has a box" and "is drawn" are therefore the same question, asked of one field.
	 *
	 * Generated here rather than derived in the felt. A felt that computed its own
	 * rectangles from CellSizeX would be a second copy of the layout arithmetic, which is
	 * the thing ChipLocation exists to avoid - and a picture that disagrees with the hit
	 * test is worse than no picture, because it looks authoritative.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Layout")
	FVector2D BoxSize = FVector2D::ZeroVector;

	/** What the hover readout shows the player. FText because a player reads it. */
	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Layout")
	FText DisplayName;
};

/**
 * A chip on the felt: which position, and how much.
 *
 * BetType and the covered numbers are not stored here. They belong to the position,
 * and duplicating them would let a bet disagree with the table it was placed on.
 * Settlement looks the position up by id, so an id that does not exist is a loud
 * error rather than a bet that quietly never wins - but the stake is already taken
 * by then. Ids come from ResolvePosition, which returns a generated id or
 * INDEX_NONE and nothing else. PlaceBet refuses only INDEX_NONE, so an id
 * synthesized anywhere else is the one route to a paid bet that cannot win.
 */
USTRUCT(BlueprintType)
struct FHasardBet
{
	GENERATED_BODY()

	/** Index into the layout's generated positions. INDEX_NONE is not a bet. */
	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Bet")
	int32 PositionId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Hasard|Bet")
	int32 Stake = 0;
};