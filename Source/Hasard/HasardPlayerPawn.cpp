// Copyright Picardbuilds. All Rights Reserved.

#include "HasardPlayerPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HasardBettingComponent.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "HasardGameMode.h"
#include "HasardInteractionComponent.h"
#include "HasardInteractable.h"
#include "HasardWheel.h"
#include "Kismet/GameplayStatics.h"

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

	InteractionComp =
		CreateDefaultSubobject<UHasardInteractionComponent>(TEXT("InteractionComp"));
}

void AHasardPlayerPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// HERE, not in the constructor: Blueprint overrides land afterwards.
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

	if (const APlayerController* PC = Cast<APlayerController>(Controller))
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
		EIC->BindAction(LookAction,     ETriggerEvent::Triggered, this, &AHasardPlayerPawn::Look);
		EIC->BindAction(PlaceBetAction, ETriggerEvent::Started,   this, &AHasardPlayerPawn::PlaceBet);
		EIC->BindAction(SpinAction,     ETriggerEvent::Started,   this, &AHasardPlayerPawn::RequestSpin);
	}
	else
	{
		UE_LOG(LogHasard, Error, TEXT("Not on EnhancedInputComponent"));
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
	if (!InteractionComp)
	{
		return;
	}

	AActor* Hit = InteractionComp->TraceForInteractable();
	if (Hit && Hit->Implements<UHasardInteractable>())
	{
		IHasardInteractable::Execute_OnPlayerInteract(Hit, this);
	}
}

void AHasardPlayerPawn::RequestSpin()
{
	AActor* Found = UGameplayStatics::GetActorOfClass(GetWorld(), AHasardWheel::StaticClass());
	if (AHasardWheel* Wheel = Cast<AHasardWheel>(Found))
	{
		Wheel->StartSpin();
	}
	else
	{
		UE_LOG(LogHasard, Warning, TEXT("Spin requested, but there is no wheel in the level"));
	}
}

void AHasardPlayerPawn::SettleRound(int32 WinningPocket)
{
	// Through the GameMode now, because the GameMode owns the payout table. The
	// console test and the wheel therefore travel the same path.
	if (AHasardGameMode* GM = GetWorld()->GetAuthGameMode<AHasardGameMode>())
	{
		GM->ResolveRound(WinningPocket);
	}
}