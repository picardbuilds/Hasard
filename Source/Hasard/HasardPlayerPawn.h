// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HasardPlayerPawn.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;
class UInputMappingContext;
class UInputAction;
class UHasardBettingComponent;
class UHasardInteractionComponent;
struct FInputActionValue;

UCLASS()
class HASARD_API AHasardPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	AHasardPlayerPawn();

	/** Console: SettleRound 17 */
	UFUNCTION(Exec)
	void SettleRound(int32 WinningPocket);

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	/** Distance from the table. Applied in OnConstruction, so edits apply right away. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Camera",
		meta = (AllowPrivateAccess = "true"))
	float CameraDistance = 250.0f;

	/** Which view the round starts in. The table has been filmed from behind since guide 2. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Camera",
		meta = (AllowPrivateAccess = "true"))
	bool bStartInThirdPerson = true;

	/** Current view. Never assigned outside ToggleView, and never read except by ApplyViewMode. */
	bool bThirdPerson = true;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Camera")
	TObjectPtr<USceneComponent> ViewRoot;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Camera")
	TObjectPtr<UCameraComponent> TableCamera;

	/**
	 * The player, seen from behind.
	 *
	 * The mesh, its animation class and its transform are all set on this component in
	 * BP_PlayerPawn rather than in code. There is no path to a content pack anywhere in
	 * this file, which is deliberate: an asset path an agent cannot open is a guess, and
	 * a guess that compiles is worse than one that does not.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Hasard|Camera")
	TObjectPtr<USkeletalMeshComponent> BodyMesh;

	/**
	 * Movement without a Character. A Character brings a capsule, gravity, crouching and
	 * a whole animation contract; a player at a table needs none of it and would have to
	 * be told to ignore all of it. This slides a pawn about and stops.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Hasard|Movement")
	TObjectPtr<UFloatingPawnMovement> Movement;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Betting")
	TObjectPtr<UHasardBettingComponent> BettingComp;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Interaction")
	TObjectPtr<UHasardInteractionComponent> InteractionComp;

	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Input")
	TObjectPtr<UInputMappingContext> TableMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Input")
	TObjectPtr<UInputAction> LookAction;

	/** Axis2D. X is strafe, Y is forward - the mapping in IMC_Table decides which key is which. */
	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Input")
	TObjectPtr<UInputAction> PlaceBetAction;

	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Input")
	TObjectPtr<UInputAction> SpinAction;

	/** Digital. Lives in IMC_Table, so the pause menu and the reality check both suppress it. */
	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Input")
	TObjectPtr<UInputAction> ToggleViewAction;

	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	void PlaceBet();
	void RequestSpin();
	void ToggleView();

	/**
	 * Puts the camera and the body where bThirdPerson says they go.
	 *
	 * One function decides both, because they are one decision. Two places setting the
	 * arm length and the visibility separately is how you end up looking at the inside
	 * of your own head.
	 */
	void ApplyViewMode();
};