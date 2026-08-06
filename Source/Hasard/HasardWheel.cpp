// Fill out your copyright notice in the Description page of Project Settings.


#include "HasardWheel.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AHasardWheel::AHasardWheel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	WheelRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WheelRoot"));
	RootComponent = WheelRoot;

	WheelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelMesh"));
	WheelMesh->SetupAttachment(WheelRoot);

	BallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BallMesh"));
	BallMesh->SetupAttachment(WheelRoot);
}

// Called when the game starts or when spawned
void AHasardWheel::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AHasardWheel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

