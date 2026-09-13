// Copyright Picardbuilds. All Rights Reserved.

#include "HasardPlayerPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/SpringArmComponent.h"
#include "HasardBettingComponent.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "HasardGameMode.h"
#include "HasardInteractionComponent.h"
#include "HasardInteractable.h"
#include "HasardWheel.h"
#include "HasardTypes.h"
#include "Kismet/GameplayStatics.h"

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

	BodyMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(ViewRoot);

	// No collision on the body. The pawn is moved by FloatingPawnMovement, and a colliding
	// mesh would both push the camera about and sit in front of every trace the interaction
	// component fires - which would mean aiming at the felt and hitting your own arm.
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BodyMesh->SetGenerateOverlapEvents(false);

	Movement = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("Movement"));

	// A walk, not a sprint. The defaults are 1200 uu/s, which crosses the whole felt in
	// under a second and makes aiming at a 24 cm cell a matter of luck.
	Movement->MaxSpeed = 250.0f;
	Movement->Acceleration = 1200.0f;
	Movement->Deceleration = 2400.0f;

	BettingComp = CreateDefaultSubobject<UHasardBettingComponent>(TEXT("BettingComp"));

	InteractionComp =
		CreateDefaultSubobject<UHasardInteractionComponent>(TEXT("InteractionComp"));
}

void AHasardPlayerPawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// HERE, not in the constructor: Blueprint overrides land afterwards. bStartInThirdPerson
	// is one of them, which is why the view is applied from this function and not that one.
	bThirdPerson = bStartInThirdPerson;
	ApplyViewMode();
}

void AHasardPlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	// Again here. OnConstruction gives the editor viewport the right picture; this is the
	// one that decides what the player starts the round looking at.
	bThirdPerson = bStartInThirdPerson;
	ApplyViewMode();
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
		EIC->BindAction(LookAction,       ETriggerEvent::Triggered, this, &AHasardPlayerPawn::Look);
		EIC->BindAction(MoveAction,       ETriggerEvent::Triggered, this, &AHasardPlayerPawn::Move);
		EIC->BindAction(PlaceBetAction,   ETriggerEvent::Started,   this, &AHasardPlayerPawn::PlaceBet);
		EIC->BindAction(SpinAction,       ETriggerEvent::Started,   this, &AHasardPlayerPawn::RequestSpin);
		EIC->BindAction(ToggleViewAction, ETriggerEvent::Started,   this, &AHasardPlayerPawn::ToggleView);
	}
	else
	{
		UE_LOG(LogHasard, Error, TEXT("Not on EnhancedInputComponent"));
	}
}

void AHasardPlayerPawn::ToggleView()
{
	bThirdPerson = !bThirdPerson;
	ApplyViewMode();

	UE_LOG(LogHasard, Warning, TEXT("View: %s"),
		bThirdPerson ? TEXT("third person") : TEXT("first person"));
}

void AHasardPlayerPawn::ApplyViewMode()
{
	if (CameraBoom)
	{
		// Zero puts the camera back at ViewRoot, which is where the head is. CameraDistance
		// pulls it to where this table has been filmed from since guide 2.
		CameraBoom->TargetArmLength = bThirdPerson ? CameraDistance : 0.0f;
	}

	if (BodyMesh)
	{
		// The whole body, not the head alone. At arm length zero the camera is inside the
		// skull, and hiding one bone to fix that is a longer argument than it is worth.
		BodyMesh->SetVisibility(bThirdPerson, true);
	}
}

void AHasardPlayerPawn::Look(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	AddControllerYawInput(Input.X);
	AddControllerPitchInput(Input.Y);
}

void AHasardPlayerPawn::Move(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();

	// Yaw only. Pitch belongs to the camera: looking down at the felt must not mean
	// walking into the floor, and looking up must not lift you off it.
	const FRotator YawOnly(0.0f, GetControlRotation().Yaw, 0.0f);

	AddMovementInput(YawOnly.RotateVector(FVector::ForwardVector), Input.Y);
	AddMovementInput(YawOnly.RotateVector(FVector::RightVector), Input.X);
}

void AHasardPlayerPawn::PlaceBet()
{
	if (!InteractionComp)
	{
		return;
	}

	// This frame's hover, not a fresh trace. The two would almost always agree, and the
	// times they would not are the times it matters: a trace taken a frame after the one
	// that drew the preview can resolve to the neighboring position, and the player
	// would be charged for a bet other than the one they were looking at.
	FHitResult Hit;
	if (!InteractionComp->GetHoveredHit(Hit))
	{
		return;
	}

	AActor* HitActor = Hit.GetActor();
	if (HitActor && HitActor->Implements<UHasardInteractable>())
	{
		// ImpactPoint, not the actor location: on the felt the point is the bet.
		IHasardInteractable::Execute_OnPlayerInteract(HitActor, this, Hit.ImpactPoint);
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