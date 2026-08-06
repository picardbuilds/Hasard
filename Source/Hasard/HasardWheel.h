// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HasardWheel.generated.h"

class UStaticMeshComponent;
class USceneComponent;

UCLASS()
class HASARD_API AHasardWheel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHasardWheel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<USceneComponent> WheelRoot;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<UStaticMeshComponent> WheelMesh;

	UPROPERTY(VisibleAnywhere, Category = "Hasard|Wheel")
	TObjectPtr<UStaticMeshComponent> BallMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Wheel")
	int32 PocketCount = 37;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hasard|Wheel")
	float SpinDuration = 6.0f;

	virtual void PostInitializeComponents() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
