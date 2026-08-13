// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HasardPlayerPawn.generated.h"

class USceneComponent;
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

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Camera")
	TObjectPtr<USceneComponent> ViewRoot;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Camera")
	TObjectPtr<UCameraComponent> TableCamera;

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

	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	void PlaceBet();
	void RequestSpin();
};