// Fill out your copyright notice in the Description page of Project Settings.


#include "HasardPlayerPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// Sets default values
AHasardPlayerPawn::AHasardPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	ViewRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ViewRoot"));
	RootComponent = ViewRoot;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(ViewRoot);
	CameraBoom->TargetArmLength = CameraDistance;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = false;

	TableCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TableCamera"));
	TableCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TableCamera->bUsePawnControlRotation = false;

}

// Called when the game starts or when spawned
void AHasardPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called to bind functionality to input
void AHasardPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

