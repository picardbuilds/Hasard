// Fill out your copyright notice in the Description page of Project Settings.


#include "HasardWheel.h"
#include "Components/StaticMeshComponent.h"


DEFINE_LOG_CATEGORY_STATIC(LogHasard, Log, All);
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

	UE_LOG(LogHasard, Warning, TEXT("Wheel: Constructor - %s (CDO: %s)"), 
		*GetName(), HasAnyFlags(RF_ClassDefaultObject) ? TEXT("yes") : TEXT("no"));
}

void AHasardWheel::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UE_LOG(LogHasard, Warning, TEXT("Wheel: PostInitializeComponents"));
}

// Called when the game starts or when spawned
void AHasardWheel::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogHasard, Warning, TEXT("Wheel: BeginPlay"));
	
}

// Called every frame
void AHasardWheel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AHasardWheel::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogHasard, Warning, TEXT("Wheel: EndPlay"));
	Super::EndPlay(EndPlayReason);
}
