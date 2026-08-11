// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "HasardWheel.generated.h"

class UStaticMeshComponent;
class USceneComponent;

/** One parameter means two entries after the name: the type, then the variable name. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBallSettled, int32, WinningPocket);

UCLASS()
class HASARD_API AHasardWheel : public AActor
{
	GENERATED_BODY()

public:
	AHasardWheel();

	/** Fired once per spin, when the ball settles. The wheel does not know who listens. */
	UPROPERTY(BlueprintAssignable, Category = "Hasard|Wheel")
	FOnBallSettled OnBallSettled;

	UFUNCTION(BlueprintCallable, Category = "Hasard|Wheel")
	void StartSpin();

	UFUNCTION(BlueprintPure, Category = "Hasard|Wheel")
	int32 GetPocketCount() const { return PocketCount; }

	/** One spin, no animation. Public so the test and real play share one code path. */
	int32 DetermineWinningPocket() const;

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Timer callback. Nothing outside the wheel finishes a spin. */
	void FinishSpin();

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<USceneComponent> WheelRoot;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<UStaticMeshComponent> WheelMesh;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<UStaticMeshComponent> BallMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Wheel",
		meta = (AllowPrivateAccess = "true", ClampMin = "1"))
	int32 PocketCount = 37;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Wheel",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float SpinDuration = 6.0f;

	/** Plain struct, not a UObject: no UPROPERTY needed. */
	FTimerHandle SpinTimerHandle;
};