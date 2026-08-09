// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HasardBankrollComponent.generated.h"

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

	UFUNCTION(BlueprintPure, Category = "Hasard|Bankroll")
	int32 GetBalance() const { return Balance; }
	
	UFUNCTION(BlueprintPure, Category = "Hasard|Bankroll")
	int32 GetTotalStaked() const { return TotalStaked; }

	UFUNCTION(BlueprintPure, Category = "Hasard|Bankroll")
	int32 GetSessionNetChange() const { return TotalWon - TotalStaked; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Bankroll", meta = (ClampMin = "0"))
	int32 StartingBalance = 500;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hasard|Bankroll")
	int32 Balance = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hasard|Bankroll")
	int32 TotalStaked = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hasard|Bankroll")
	int32 TotalWon = 0;
		
};
