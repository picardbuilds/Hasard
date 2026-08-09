// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HasardGameMode.generated.h"

class AHasardWheel;

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
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Bound to the wheel delegate in BeginPlay. UFUNCTION is what AddDynamic resolves by name. */
	UFUNCTION()
	void HandleBallSettled(int32 WinningPocket);

	UPROPERTY()
	TObjectPtr<AHasardWheel> BoundWheel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hasard|Round",
		meta = (AllowPrivateAccess = "true"))
	EHasardRoundPhase CurrentPhase = EHasardRoundPhase::Betting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Round",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float BettingWindowSeconds = 30.0f;
};
