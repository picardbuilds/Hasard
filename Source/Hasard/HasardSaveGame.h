// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "HasardSaveGame.generated.h"

/**
 * Everything that outlives a sitting.
 *
 * Deliberately not just a balance. What has to survive a launch is the record of what
 * playing has cost, and the balance is one line of it - a game that restores your money
 * and forgets your losses shows you a smaller number every time you open it.
 */
UCLASS()
class HASARD_API UHasardSaveGame : public USaveGame
{
	GENERATED_BODY()

public: 
	/** The one slot this game ever writes. One player, one record. */
	static const TCHAR* SlotName;

	/** No profiles and no split screen, so this is a constant rather than a parameter. */
	static constexpr int32 UserIndex = 0;

	/**
	 * Bumped when a field below changes meaning, not when one is added.
	 *
	 * A save written by an older version is refused rather than read. A misread
	 * LifetimeStaked is a false statement about spend, and there is no safe guess.
	 */
	static constexpr int32 CurrentVersion = 1;

	UPROPERTY()
	int32 Version = CurrentVersion;

	/** What was left when the last sitting ended. The only field a new session ignores. */
	UPROPERTY()
	int32 Balance = 0;

	/** Across every sitting. Nothing in this project ever writes a smaller value here. */
	UPROPERTY()
	int32 LifetimeStaked = 0;

	UPROPERTY()
	int32 LifetimeWon = 0;

	/** Wall-clock seconds across every sitting, on the same terms as the session clock. */
	UPROPERTY()
	float LifetimeSeconds = 0.0f;

	UPROPERTY()
	int32 SessionsPlayed = 0;
};
