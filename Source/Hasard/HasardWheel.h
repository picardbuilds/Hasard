// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "HasardWheel.generated.h"

class UHasardTableLayout;
class UStaticMeshComponent;
class USceneComponent;
class UTextRenderComponent;

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

	/**
	 * Local yaw of the pocket holding Number.
	 *
	 * The rim is described clockwise from zero; yaw runs counter-clockwise. The negation
	 * that reconciles the two lives here and nowhere else, so the numerals and the ball
	 * cannot end up disagreeing about which way round the wheel goes.
	 */
	UFUNCTION(BlueprintPure, Category = "Hasard|Wheel")
	bool TryGetPocketYaw(int32 Number, float& OutYaw) const;

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

	/** Destroys any numerals and prints a fresh set from the rim sequence. */
	void BuildNumerals();

	/** Red, black, or green for zero. The felt owns the red list; this reads it. */
	FColor NumeralColorFor(int32 Number, const UHasardTableLayout* Layout) const;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<USceneComponent> WheelRoot;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<UStaticMeshComponent> WheelMesh;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<UStaticMeshComponent> BallMesh;

	/** Every numeral this actor made. UPROPERTY so the collector can see them. */
	UPROPERTY()
	TArray<TObjectPtr<UTextRenderComponent>> Numerals;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Wheel",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float SpinDuration = 6.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Numerals",
		meta = (AllowPrivateAccess = "true"))
	bool bShowNumerals = true;

	/** Distance from the hub to the middle of a numeral, in centimetres. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Numerals",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float NumeralRadius = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Numerals",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float NumeralTextSize = 6.0f;

	/** Lifted off the rim so the numerals are not co-planar with whatever is under them. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Numerals",
		meta = (AllowPrivateAccess = "true"))
	float NumeralZOffset = 0.5f;

	/**
	 * Base orientation of a numeral before its own pocket yaw is added.
	 *
	 * Pitch 90 lays the text flat on a horizontal rim, exactly as LabelRotation does on
	 * the felt. If the numerals come out mirrored or upside-down, this is the knob - the
	 * felt needed the same tuning and for the same reason.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Numerals",
		meta = (AllowPrivateAccess = "true"))
	FRotator NumeralRotation = FRotator(90.0f, 180.0f, 0.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Numerals",
		meta = (AllowPrivateAccess = "true"))
	FColor RedNumeralColor = FColor(200, 32, 32);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Numerals",
		meta = (AllowPrivateAccess = "true"))
	FColor BlackNumeralColor = FColor(24, 24, 24);

	/** Zero is the house edge. It does not share a colour with anything else on the rim. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Numerals",
		meta = (AllowPrivateAccess = "true"))
	FColor ZeroNumeralColor = FColor(26, 132, 72);

	/** Plain struct, not a UObject: no UPROPERTY needed. */
	FTimerHandle SpinTimerHandle;
};