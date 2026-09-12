// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HasardGameMode.generated.h"

class AHasardWheel;
class UHasardPayoutTable;
class UHasardTableLayout;

UENUM(BlueprintType)
enum class EHasardRoundPhase : uint8
{
	Betting  UMETA(DisplayName = "Betting"),
	Spinning UMETA(DisplayName = "Spinning"),
	Settling UMETA(DisplayName = "Settling")
};

UCLASS()
class HASARD_API AHasardGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHasardGameMode();

	/** Console: HasardTestDistribution 370000 */
	UFUNCTION(Exec)
	void HasardTestDistribution(int32 SpinCount);

	/** Console: HasardShowCell 17 */
	UFUNCTION(Exec)
	void HasardShowCell(int32 Number);

	/** Console: HasardAuditLayout */
	UFUNCTION(Exec)
	void HasardAuditLayout();

	/** Console: HasardShowPocket 17 */
	UFUNCTION(Exec)
	void HasardShowPocket(int32 Number);

	/** Settles one round against the payout table. */
	UFUNCTION(BlueprintCallable, Category = "Hasard|Round")
	void ResolveRound(int32 WinningPocket);

	/** The HUD reads odds through this. Const: nothing may edit the table at runtime. */
	const UHasardPayoutTable* GetPayoutTable() const { return PayoutTable; }

	/** The felt reads its measurements through this. Const for the same reason. */
	const UHasardTableLayout* GetTableLayout() const { return TableLayout; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Bound to the wheel delegate in BeginPlay. UFUNCTION is what AddDynamic resolves by name. */
	UFUNCTION()
	void HandleBallSettled(int32 WinningPocket);

	/** Cached so EndPlay unbinds from the same wheel BeginPlay bound to. */
	UPROPERTY()
	TObjectPtr<AHasardWheel> BoundWheel;

	/** Assigned on BP_HasardGameMode. Payouts never come from a literal. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Round",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHasardPayoutTable> PayoutTable;

	/** Assigned on BP_HasardGameMode. Measurements never come from a literal either. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Round",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHasardTableLayout> TableLayout;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hasard|Round",
		meta = (AllowPrivateAccess = "true"))
	EHasardRoundPhase CurrentPhase = EHasardRoundPhase::Betting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Round",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float BettingWindowSeconds = 30.0f;
};
