// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HasardPlayerPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class HASARD_API AHasardPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	AHasardPlayerPawn();

protected:
	virtual void BeginPlay() override;

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

	void Look(const FInputActionValue& Value);
	void PlaceBet();
	void RequestSpin();

public:	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

};
