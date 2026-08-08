// Copyright Picardbuilds. All Rights Reserved.

#include "HasardPlayerPawn.h"
#include "Camera/CameraComponent.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HasardBettingComponent.h"
#include "HasardTypes.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"

DEFINE_LOG_CATEGORY_STATIC(LogHasard, Log, All);

AHasardPlayerPawn::AHasardPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = false;

	ViewRoot = CreateDefaultSubobject<USceneComponent>(TEXT("ViewRoot"));
	RootComponent = ViewRoot;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(ViewRoot);
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bDoCollisionTest = false;

	TableCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TableCamera"));
	TableCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TableCamera->bUsePawnControlRotation = false;

	BettingComp = CreateDefaultSubobject<UHasardBettingComponent>(TEXT("BettingComp"));
}

void AHasardPlayerPawn::OnConstruction(const FTransform& Transform) 
{
	Super::OnConstruction(Transform);

	if (CameraBoom)
	{
		CameraBoom->TargetArmLength = CameraDistance;
	}
}

void AHasardPlayerPawn::BeginPlay()
{
	Super::BeginPlay();
}

void AHasardPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (TableMappingContext)
			{
				Subsystem->AddMappingContext(TableMappingContext, 0);
			}
		}
	}

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EIC->BindAction(LookAction,		ETriggerEvent::Triggered, this, &AHasardPlayerPawn::Look);
		EIC->BindAction(PlaceBetAction, ETriggerEvent::Started,   this, &AHasardPlayerPawn::PlaceBet);
		EIC->BindAction(SpinAction,		ETriggerEvent::Started,   this, &AHasardPlayerPawn::RequestSpin);
	}

	else
	{
		UE_LOG(LogHasard, Warning, TEXT("Not on EnhancedInputComponent"));
	}
}

void AHasardPlayerPawn::Look(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	AddControllerYawInput(Input.X);
	AddControllerPitchInput(Input.Y);
}

void AHasardPlayerPawn::PlaceBet()
{
	UE_LOG(LogHasard, Warning, TEXT("PlaceBet pressed"));

	if (BettingComp)
	{
		BettingComp->PlaceBet(EHasardBetType::StraightUp, 17, 5);
	}
}

void AHasardPlayerPawn::RequestSpin()
{
	UE_LOG(LogHasard, Warning, TEXT("Spin requested"));
}