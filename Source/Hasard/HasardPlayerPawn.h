// Copyright Picardbuilds. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HasardPlayerPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UHasardBettingComponent;
struct FInputActionValue;

UCLASS()
class HASARD_API AHasardPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	AHasardPlayerPawn();

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
protected:
	virtual void BeginPlay() override;

	/** Distance from the table. Applied in OnConstruction, so edits apply right away. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Camera")
	float CameraDistance = 250.0f;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Camera")
	TObjectPtr<USceneComponent> ViewRoot;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Camera")
	TObjectPtr<UCameraComponent> TableCamera;

	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Input")
	TObjectPtr<UInputMappingContext> TableMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Input")
	TObjectPtr<UInputAction> PlaceBetAction;

	UPROPERTY(EditDefaultsOnly, Category = "Hasard|Input")
	TObjectPtr<UInputAction> SpinAction;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Betting")
	TObjectPtr<UHasardBettingComponent> BettingComp;

	void Look(const FInputActionValue& Value);
	void PlaceBet();
	void RequestSpin();
};
