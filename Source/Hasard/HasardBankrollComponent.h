// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HasardBankrollComponent.generated.h"

class UHasardSaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnBankrollChanged,
	int32, NewBalance, int32, Delta, int32, SessionNetChange);

UCLASS( ClassGroup=(Hasard), meta=(BlueprintSpawnableComponent) )
class HASARD_API UHasardBankrollComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UHasardBankrollComponent();

	/** BlueprintAssignable requires a dynamic multicast delegate. Nothing else qualifies. */
	UPROPERTY(BlueprintAssignable, Category = "Hasard|Bankroll")
	FOnBankrollChanged OnBankrollChanged;

	/** Deducts Amount and broadcasts. Returns false and changes nothing if it will not cover. */
	UFUNCTION(BlueprintCallable, Category = "Hasard|Bankroll")
	bool TryStake(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Hasard|Bankroll")
	void CreditWinnings(int32 Amount);

	/**
	 * Restores the record, and the balance only if the player chose to continue.
	 *
	 * The two lines below are the whole of the new-session decision, and the asymmetry is
	 * the point: the lifetime figures are read unconditionally, so no choice on the start
	 * screen can make what the player has already spent look smaller.
	 */
	void ApplySave(const UHasardSaveGame& Save, bool bContinuePrevious);

	UFUNCTION(BlueprintPure, Category = "Hasard|Bankroll")
	int32 GetBalance() const { return Balance; }
	
	/** This sitting only. The HUD shows it beside the lifetime figure, never instead of it. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Bankroll")
	int32 GetSessionStaked() const { return SessionStaked; }

	UFUNCTION(BlueprintPure, Category = "Hasard|Bankroll")
	int32 GetSessionNetChange() const { return SessionWon - SessionStaked; }

	/**
	 * Prior sittings plus this one, computed rather than stored.
	 *
	 * A third pair of counters kept in step with these would be the same fact in two
	 * places, and the day they disagreed the smaller pair would be the one on screen.
	 */
	UFUNCTION(BlueprintPure, Category = "Hasard|Bankroll")
	int32 GetLifetimeStaked() const { return PriorStaked + SessionStaked; }

	UFUNCTION(BlueprintPure, Category = "Hasard|Bankroll")
	int32 GetLifetimeWon() const { return PriorWon + SessionWon; }

	UFUNCTION(BlueprintPure, Category = "Hasard|Bankroll")
	int32 GetLifetimeNetChange() const { return GetLifetimeWon() - GetLifetimeStaked(); }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Bankroll", meta = (ClampMin = "0"))
	int32 StartingBalance = 500;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hasard|Bankroll")
	int32 Balance = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hasard|Bankroll")
	int32 SessionStaked = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hasard|Bankroll")
	int32 SessionWon = 0;

	/** Carried in from the save and never written by play. Only ApplySave sets these. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hasard|Bankroll")
	int32 PriorStaked = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hasard|Bankroll")
	int32 PriorWon = 0;
};
