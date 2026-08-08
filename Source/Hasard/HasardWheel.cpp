// Copyright Picardbuilds. All Rights Reserved.

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

	UE_LOG(LogHasard, Warning, TEXT("Wheel: Constructor - %s"), 
		*GetName());
}

void AHasardWheel::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UE_LOG(LogHasard, Warning, TEXT("Wheel: PostInitializeComponents"));
}

void AHasardWheel::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogHasard, Warning, TEXT("Wheel: BeginPlay"));
}

void AHasardWheel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AHasardWheel::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UE_LOG(LogHasard, Warning, TEXT("Wheel: EndPlay"));
	Super::EndPlay(EndPlayReason);
}
