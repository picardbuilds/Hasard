// Fill out your copyright notice in the Description page of Project Settings.


#include "HasardCharacter.h"

// Sets default values
AHasardCharacter::AHasardCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AHasardCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHasardCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AHasardCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

