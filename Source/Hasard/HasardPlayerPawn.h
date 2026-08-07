// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HasardPlayerPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;

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

public:	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

};
