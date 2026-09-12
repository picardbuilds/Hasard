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

	/**
	 * The physical order of the numbers around the rim, clockwise from zero.
	 *
	 * Not a setting and not a preference. This is what a single-zero wheel is, and
	 * nothing else in the project may hold a second copy of it.
	 */
	static const TArray<int32>& GetPocketSequence();

	/** How many pockets. The length of the sequence, not a number anyone can type. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Wheel")
	int32 GetPocketCount() const;

	/** The number sitting in rim slot RimIndex. INDEX_NONE if that slot does not exist. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Wheel")
	int32 GetNumberAtRimIndex(int32 RimIndex) const;

	/** Which rim slot holds Number. INDEX_NONE if it is not a pocket on this wheel. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Wheel")
	int32 GetRimIndexOf(int32 Number) const;

	/**
	 * Degrees clockwise from the zero pocket to Number.
	 *
	 * Try rather than a plain getter because zero sits at 0.0 degrees - so a failed
	 * lookup returning 0.0 would be indistinguishable from a correct answer.
	 */
	UFUNCTION(BlueprintPure, Category = "Hasard|Wheel")
	bool TryGetPocketAngleDegrees(int32 Number, float& OutDegrees) const;

	/** One spin, no animation. Public so the test and real play share one code path. */
	int32 DetermineWinningPocket() const;

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	/** Timer callback. Nothing outside the wheel finishes a spin. */
	void FinishSpin();

	/** Walks the sequence on BeginPlay and says out loud whether it is still a wheel. */
	void AuditPocketSequence() const;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<USceneComponent> WheelRoot;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<UStaticMeshComponent> WheelMesh;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<UStaticMeshComponent> BallMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Wheel",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float SpinDuration = 6.0f;

	/** Plain struct, not a UObject: no UPROPERTY needed. */
	FTimerHandle SpinTimerHandle;
};