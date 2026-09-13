// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "HasardWheel.generated.h"

class UHasardTableLayout;
class UInstancedStaticMeshComponent;
class UMaterialInterface;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;
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

	/**
	 * The pocket a yaw points at - the inverse of TryGetPocketYaw.
	 *
	 * Used to read the landing back off the transforms rather than off the variable that
	 * produced them, which is what makes the check in FinishSpin worth having.
	 */
	UFUNCTION(BlueprintPure, Category = "Hasard|Wheel")
	int32 GetPocketAtYaw(float Yaw) const;

	/** True from StartSpin until the ball settles. */
	UFUNCTION(BlueprintPure, Category = "Hasard|Wheel")
	bool IsSpinning() const { return bIsSpinning;  }

	/** One spin, no animation. Public so the test and real play share one code path. */
	int32 DetermineWinningPocket() const;

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;

private:
	/** Ends the spin and broadcasts. Nothing outside the wheel finishes a spin. */
	void FinishSpin();

	/** Places the rim and the ball for one point on the eased curve, 0 to 1. */
	void ApplySpin(float Eased);

	/** Walks the sequence on BeginPlay and says out loud whether it is still a wheel. */
	void AuditPocketSequence() const;

	/**
	 * Prints the thirty-seven coloured plates.
	 *
	 * Called from OnConstruction as well as BeginPlay, and it has to be, for the reason
	 * AHasardFelt::BuildSurface records: an instanced component builds its render state
	 * when it registers, and one registered without a mesh never picks up a mesh assigned
	 * afterwards - the value is stored and the renderer keeps drawing the default.
	 */
	void BuildPockets();

	/**
	 * Destroys any numerals and prints a fresh set from the rim sequence.
	 *
	 * BeginPlay only, matching AHasardFelt::BuildLabels. These are a component each rather
	 * than instances, and OnConstruction runs on every property edit in the editor.
	 */
	void BuildNumerals();

	/** Points a component at PocketMesh and PocketMaterial and tints it. */
	void PreparePockets(UInstancedStaticMeshComponent* Component, const FColor& Color) const;


	/**
	 * Which instanced component prints this pocket, which is the same question as what
	 * colour it is: the components are split by colour, so routing and colouring cannot
	 * disagree. The felt owns the red list; this reads it.
	 */
	UInstancedStaticMeshComponent* PocketsFor(int32 Number,
		const UHasardTableLayout* Layout) const;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<USceneComponent> WheelRoot;

	/**
	 * The turning part of the wheel, and the parent of everything printed on it.
	 *
	 * It exists to carry no scale. A parent's scale is applied to a child component
	 * axis by axis in the child's own frame, without being rotated into it, so a rim
	 * mesh flattened to (2, 2, 0.2) squashes a numeral's glyph height by 0.2 while
	 * stretching its width by 2. Hanging the rim's contents off an unscaled node and
	 * letting only WheelMesh carry the flattening removes the whole class of problem.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<USceneComponent> RimRoot;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<UStaticMeshComponent> WheelMesh;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<UStaticMeshComponent> BallMesh;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Pocket")
	TObjectPtr<UInstancedStaticMeshComponent> RedPockets;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Pocket")
	TObjectPtr<UInstancedStaticMeshComponent> BlackPockets;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Pocket")
	TObjectPtr<UInstancedStaticMeshComponent> ZeroPocket;

	/** Every numeral this actor made. UPROPERTY so the collector can see them. */
	UPROPERTY()
	TArray<TObjectPtr<UTextRenderComponent>> Numerals;

	/**
	 * The same asset BP_Felt holds, assigned again here.
	 *
	 * Module 2 read it off the GameMode to avoid a second slot. That worked while the only
	 * thing it fed was BeginPlay. The plates are printed from OnConstruction, where there
	 * is no GameMode at all, so the wheel needs the asset in its own hand. This is not a
	 * second copy of the red list - that list lives once, in UHasardTableLayout.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Wheel",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHasardTableLayout> TableLayout;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Wheel",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float SpinDuration = 6.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Pockets",
		meta = (AllowPrivateAccess = "true"))
	bool bShowPockets = true;

	/** A unit quad in centimetres. The same mesh BP_Felt prints its boxes from. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Pockets",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMesh> PocketMesh;

	/** Needs a vector parameter named by ColorParameterName. The felt's material has one. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Pockets",
		meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UMaterialInterface> PocketMaterial;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Pockets",
		meta = (AllowPrivateAccess = "true"))
	FName ColorParameterName = TEXT("Color");

	/** Distance from the hub to the middle of a plate, in centimetres. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Pockets",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float PocketRadius = 45.0f;

	/** How far a plate reaches in and out from that radius. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Pockets",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float PocketLength = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Pockets",
		meta = (AllowPrivateAccess = "true"))
	float PocketZOffset = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Pockets",
		meta = (AllowPrivateAccess = "true"))
	FColor RedPocketColor = FColor(178, 26, 30);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Pockets",
		meta = (AllowPrivateAccess = "true"))
	FColor BlackPocketColor = FColor(22, 22, 24);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Pockets",
		meta = (AllowPrivateAccess = "true"))
	FColor ZeroPocketColor = FColor(20, 130, 60);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Numerals",
		meta = (AllowPrivateAccess = "true"))
	bool bShowNumerals = true;

	/** Distance from the hub to the middle of a numeral. Sits on the plate, so match it. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Numerals",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float NumeralRadius = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Numerals",
		meta = (AllowPrivateAccess = "true", ClampMin = "0.1"))
	float NumeralTextSize = 6.0f;

	/** Must stay above PocketZOffset, or the numerals sink into their own plates. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Numerals",
		meta = (AllowPrivateAccess = "true"))
	float NumeralZOffset = 0.8f;

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

	/**
	 * One colour for all thirty-seven.
	 *
	 * A real wheel prints white on a coloured pocket, and so does this one now. The colour
	 * of a number is a fact about the pocket, stated once, by the plate under it.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Numerals",
		meta = (AllowPrivateAccess = "true"))
	FColor NumeralColor = FColor(245, 245, 240);

	/** How far the rim turns over the whole spin. Presentation only. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Spin",
		meta = (AllowPrivateAccess = "true"))
	float WheelSweepDegrees = 1080.0f;

	/** How far the ball travels over the whole spin, in the opposite sense to the rim. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Spin",
		meta = (AllowPrivateAccess = "true"))
	float BallSweepDegrees = -2160.0f;

	/** Where the ball rides at the start of the spin. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Spin",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float BallOuterRadius = 58.0f;

	/** Where the ball sits once it is in a pocket. Match this to PocketRadius. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Spin",
		meta = (AllowPrivateAccess = "true", ClampMin = "1.0"))
	float BallPocketRadius = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Spin",
		meta = (AllowPrivateAccess = "true"))
	float BallHeight = 2.0f;

	/** The pocket this spin is travelling to. Decided in StartSpin, before a frame runs. */
	int32 PendingPocket = INDEX_NONE;

	bool bIsSpinning = false;

	/** Seconds since StartSpin. The animation is a function of this and nothing else. */
	float SpinElapsed = 0.0f;

	/** Last applied angles, kept so FinishSpin can read the landing off the result. */
	float WheelYaw = 0.0f;
	float BallYaw = 0.0f;
};