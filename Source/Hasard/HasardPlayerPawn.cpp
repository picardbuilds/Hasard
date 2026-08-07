// Fill out your copyright notice in the Description page of Project Settings.


#include "HasardPlayerPawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/LocalPlayer.h"

DEFINE_LOG_CATEGORY_STATIC(LogHasard, Log, All);

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
}

void AHasardPlayerPawn::RequestSpin()
{
	UE_LOG(LogHasard, Warning, TEXT("Spin requested"));
}