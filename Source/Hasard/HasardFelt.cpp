// Copyright Picardbuilds. All Rights Reserved.

#include "HasardFelt.h"
#include "HasardBettingComponent.h"
#include "HasardChip.h"
#include "HasardPayoutTable.h"
#include "HasardPlayerController.h"
#include "HasardTableLayout.h"
#include "HasardTypes.h"
#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

/**
 * Which bets get their name printed on the felt.
 *
 * The line bets do not, and that is not an omission: a split has no box of its own, so
 * "17/18" would have to be printed on the line between two numbers. A real layout leaves
 * those unlabeled too - the preview names them instead.
 *
 * No default case, deliberately. Adding an enumerator without deciding whether it is
 * named on the felt then fails the build rather than shipping a bet nobody can read.
 */
static bool IsNamedOnFelt(EHasardBetType BetType)
{
	switch (BetType)
	{
	case EHasardBetType::StraightUp:
	case EHasardBetType::Dozen:
	case EHasardBetType::Column:
	case EHasardBetType::Red:
	case EHasardBetType::Black:
	case EHasardBetType::Even:
	case EHasardBetType::Odd:
	case EHasardBetType::Low:
	case EHasardBetType::High:
		return true;

	case EHasardBetType::Split:
	case EHasardBetType::Street:
	case EHasardBetType::Trio:
	case EHasardBetType::Corner:
	case EHasardBetType::Basket:
	case EHasardBetType::SixLine:
		return false;
	}
	// Unreachable while every enumerator is listed above. Kept because the compiler
	// requires a return, and it is the line a missing case would fall through to.
	return false;
}

/**
 * The zero pocket's own straight up.
 *
 * Asked of the covered numbers rather than the position id, because the id is an index
 * into a generated array and "zero happens to be built first" is not a fact worth
 * depending on. What makes this position zero is that it covers pocket zero and nothing
 * else, which stays true however BuildPositions is reordered.
 */
static bool IsZeroPosition(const FHasardBetPosition& Position)
{
	return Position.BetType == EHasardBetType::StraightUp
		&& Position.CoveredNumbers.Num() == 1
		&& Position.CoveredNumbers[0] == 0;
}

AHasardFelt::AHasardFelt()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Surface = CreateDefaultSubobject<UBoxComponent>(TEXT("Surface"));
	Surface->SetupAttachment(Root);

	// Visibility is the channel the interaction trace sweeps on.
	Surface->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Surface->SetCollisionResponseToAllChannels(ECR_Ignore);
	Surface->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	Cloth = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cloth"));
	Cloth->SetupAttachment(Root);

	Lines = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Lines"));
	Lines->SetupAttachment(Root);

	RedBoxes = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RedBoxes"));
	RedBoxes->SetupAttachment(Root);

	BlackBoxes = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("BlackBoxes"));
	BlackBoxes->SetupAttachment(Root);

	GreenBoxes = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GreenBoxes"));
	GreenBoxes->SetupAttachment(Root);

	ZeroBox = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZeroBox"));
	ZeroBox->SetupAttachment(Root);

	// Every printed surface is scenery, and Surface above is the only thing the trace may
	// hit. A quad that blocked Visibility would be found instead of the box underneath it,
	// and every click on the felt would resolve against the wrong actor - the same failure
	// the chips avoid, arriving from the other direction.
	UStaticMeshComponent* const Printed[] = {
		Cloth, ZeroBox, Lines, RedBoxes, BlackBoxes, GreenBoxes };

	for (UStaticMeshComponent* Component : Printed)
	{
		Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Component->SetCollisionResponseToAllChannels(ECR_Ignore);
		Component->SetCastShadow(false);
	}
}

void AHasardFelt::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Resized here as well as in BeginPlay, so assigning the asset in the editor
	// shows the right box immediately rather than after the next Play.
	SyncBoxToLayout();

	// And printed here too. BoxMesh and SurfaceMaterial are EditDefaultsOnly, so they do
	// not exist until the Blueprint has written its defaults - which happens after the
	// constructor and before this. Printing from BeginPlay instead left the instanced
	// components registered with no mesh, and a material assigned to them afterwards was
	// stored on the component but never reached the renderer: the felt drew WorldGridMaterial.
	BuildSurface();
}

void AHasardFelt::BeginPlay()
{
	Super::BeginPlay();

	if (!TableLayout)
	{
		UE_LOG(LogHasard, Error, TEXT("Felt: no table layout on BP_Felt - clicks cannot resolve"));
		return;
	}

	SyncBoxToLayout();

	// Surface before labels: both read the same positions, and printing the boxes first
	// means a numeral is never briefly visible over bare cloth.
	BuildSurface();

	BuildLabels();

	if (!PayoutTable)
	{
		// Not fatal - the ghost chip still shows where a bet would land. Loud anyway,
		// because a preview that cannot say what a bet pays is the half of this feature
		// that was worth building.
		UE_LOG(LogHasard, Error,
			TEXT("Felt: no payout table on BP_Felt - the readout cannot state a payout"));
	}

	const AHasardChip* Chip = GetChipDefault();

	const int32 Count = TableLayout->GetPositions().Num();
	UE_LOG(LogHasard, Warning, TEXT("Felt: %d positions, %d labels, %d per click"),
		Count, Labels.Num(), Chip ? Chip->GetChipValue() : 0);

	if (bDrawDebugLayout)
	{
		DrawDebugLayout();
	}
}

void AHasardFelt::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Unbind before the component goes. RemoveAll on a weak pointer that has already
	// gone is a no-op, which is the whole reason it is weak.
	if (UHasardBettingComponent* Betting = BoundBetting.Get())
	{
		Betting->OnBetsCleared.RemoveAll(this);
	}

	HidePreview();
	ClearChips();

	Super::EndPlay(EndPlayReason);
}

void AHasardFelt::SyncBoxToLayout()
{
	if (!TableLayout || !Surface)
	{
		return;
	}

	FVector2D Min, Max;
	TableLayout->GetFeltBounds(Min, Max);

	const FVector2D Size = Max - Min;
	const FVector2D Center = (Max + Min) * 0.5f;

	// Offset against Root, never against the actor. If Surface were the root component,
	// setting its relative location would move the actor and shift every coordinate.
	Surface->SetBoxExtent(FVector(Size.X * 0.5f, Size.Y * 0.5f, SurfaceThickness * 0.5f));
	Surface->SetRelativeLocation(FVector(Center.X, Center.Y, 0.0f));
}

FVector2D AHasardFelt::WorldToFeltLocal(const FVector& WorldPoint) const
{
	// Relative to the actor, not to the box: the box moves with GridOrigin, and the
	// layout's coordinates are measured from the actor's own origin.
	const FVector Local = GetActorTransform().InverseTransformPosition(WorldPoint);
	return FVector2D(Local.X, Local.Y);
}

FVector AHasardFelt::FeltLocalToWorld(const FVector2D& LocalPoint) const
{
	return GetActorTransform().TransformPosition(FVector(LocalPoint.X, LocalPoint.Y, 0.0f));
}

void AHasardFelt::OnPlayerInteract_Implementation(APawn* InstigatorPawn,
	const FVector& HitLocation)
{
	if (!InstigatorPawn || !TableLayout)
	{
		return;
	}

	UHasardBettingComponent* Betting =
		InstigatorPawn->FindComponentByClass<UHasardBettingComponent>();
	if (!Betting)
	{
		UE_LOG(LogHasard, Warning, TEXT("Felt: %s has no betting component"),
			*GetNameSafe(InstigatorPawn));
		return;
	}

	// Bound here rather than in BeginPlay: the component lives on the pawn, and the
	// pawn is not reliably around when the felt starts. The first bet is the first
	// moment it certainly is.
	if (BoundBetting.Get() != Betting)
	{
		if (UHasardBettingComponent* Previous = BoundBetting.Get())
		{
			Previous->OnBetsCleared.RemoveAll(this);
		}

		Betting->OnBetsCleared.AddUObject(this, &AHasardFelt::HandleBetsCleared);
		BoundBetting = Betting;
	}

	const FVector2D Local = WorldToFeltLocal(HitLocation);
	const int32 PositionId = TableLayout->ResolvePosition(Local);

	if (PositionId == INDEX_NONE)
	{
		// A miss is not an error, but it is not silent either: the coordinate is the
		// only way to tell "clicked the margin" from "the resolver is wrong".
		UE_LOG(LogHasard, Warning, TEXT("Felt: (%.1f, %.1f) is not a bet position"),
			Local.X, Local.Y);
		return;
	}

	const FHasardBetPosition* Position = TableLayout->GetPositionById(PositionId);
	if (!Position)
	{
		UE_LOG(LogHasard, Error, TEXT("Felt: resolved id %d has no position"), PositionId);
		return;
	}

	UE_LOG(LogHasard, Warning, TEXT("Felt: (%.1f, %.1f) -> id %d, %s"),
		Local.X, Local.Y, PositionId, *Position->DisplayName.ToString());

	const AHasardChip* Chip = GetChipDefault();
	if (!Chip)
	{
		// No chip class means no stake, because the chip is where the stake lives.
		// Refusing here is the only way to avoid inventing a number.
		UE_LOG(LogHasard, Error, TEXT("Felt: no chip class on BP_Felt, so a click has no stake"));
		return;
	}

	if (!Betting->PlaceBet(PositionId, Chip->GetChipValue()))
	{
		// Refused, so no stake was taken. Nothing may be drawn as though one was.
		return;
	}

	// The debug sphere that used to mark this spot is gone: a real chip lands here now,
	// and two markers for one event is one of them lying eventually.
	SpawnChip(*Position);
}

const AHasardChip* AHasardFelt::GetChipDefault() const
{
	return ChipClass ? ChipClass->GetDefaultObject<AHasardChip>() : nullptr;
}

void AHasardFelt::OnPlayerHover_Implementation(APawn* InstigatorPawn,
	const FVector& HitLocation)
{
	if (!TableLayout)
	{
		return;
	}

	// The same two calls OnPlayerInteract makes, on the same point, one frame earlier.
	// That is the guarantee: the preview and the bet cannot resolve differently because
	// they are the same question asked of the same resolver.
	const FVector2D Local = WorldToFeltLocal(HitLocation);
	const int32 PositionId = TableLayout->ResolvePosition(Local);
	const FHasardBetPosition* Position = PositionId != INDEX_NONE
		? TableLayout->GetPositionById(PositionId) : nullptr;

	if (!Position)
	{
		// Aiming at the margin is not a bet, so it must not look like one.
		HidePreview();
		return;
	}

	ShowPreview(InstigatorPawn, *Position);
}

void AHasardFelt::OnPlayerEndHover_Implementation(APawn* InstigatorPawn)
{
	HidePreview();
}

void AHasardFelt::ShowPreview(APawn* InstigatorPawn, const FHasardBetPosition& Position)
{
	const AHasardChip* Default = GetChipDefault();
	UWorld* World = GetWorld();

	if (!World || !Default || !PreviewMaterial)
	{
		HidePreview();
		return;
	}

	if (!PreviewChip)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;

		PreviewChip = World->SpawnActor<AHasardChip>(
			ChipClass, FVector::ZeroVector, GetActorRotation(), Params);

		if (!PreviewChip)
		{
			return;
		}

		// The material is the only thing separating this from a placed bet, so it is
		// applied before the chip is ever made visible.
		PreviewChip->SetPreviewMaterial(PreviewMaterial);
	}

	// On top of the stack that is already there, so the preview shows where this chip
	// would land rather than where the first one did.
	const int32* Count = StackCounts.Find(Position.PositionId);
	const float Height = ((Count ? *Count : 0) + 0.5f) * Default->GetChipHeight();

	PreviewChip->SetActorLocation(
		FeltLocalToWorld(Position.ChipLocation) + GetActorUpVector() * Height);
	PreviewChip->SetActorHiddenInGame(false);

	BoundController = InstigatorPawn
		? Cast<AHasardPlayerController>(InstigatorPawn->GetController()) : nullptr;

	// Rebuilt only when the aim moves to a different position. This runs every frame,
	// and formatting text sixty times a second to produce the same string is waste.
	if (PreviewPositionId != Position.PositionId)
	{
		PreviewPositionId = Position.PositionId;

		if (AHasardPlayerController* PC = BoundController.Get())
		{
			PC->SetBetPreview(ReadoutTextFor(Position, *Default));
		}
	}
}

void AHasardFelt::HidePreview()
{
	if (PreviewChip)
	{
		PreviewChip->SetActorHiddenInGame(true);
	}

	if (AHasardPlayerController* PC = BoundController.Get())
	{
		PC->ClearBetPreview();
	}

	PreviewPositionId = INDEX_NONE;
}

FText AHasardFelt::ReadoutTextFor(const FHasardBetPosition& Position,
	const AHasardChip& Chip) const
{
	const int32 Stake = Chip.GetChipValue();

	const FHasardPayoutRule* Rule = PayoutTable
		? PayoutTable->FindRule(Position.BetType) : nullptr;

	if (!Rule)
	{
		// Say so plainly. A bet with no rule is one settlement will take the stake for
		// and never pay, so the player is owed the warning while they can still not make
		// it - this is the DA_AmericanPayouts gap, made visible instead of silent.
		return FText::Format(
			NSLOCTEXT("Hasard", "ReadoutNoRule", "{0}  -  this table cannot pay this bet"),
			Position.DisplayName);
	}

	// The same arithmetic SettleRound uses. TryStake already took the stake, so a 35:1
	// winner returns 36 times it.
	const int32 Returns = Stake * (Rule->PayoutRatio + 1);

	return FText::Format(
		NSLOCTEXT("Hasard", "ReadoutFmt", "{0}  -  stake {1}, returns {2}   ({3}:1)"),
		Position.DisplayName, FText::AsNumber(Stake),
		FText::AsNumber(Returns), FText::AsNumber(Rule->PayoutRatio));
}

void AHasardFelt::SpawnChip(const FHasardBetPosition& Position)
{
	UWorld* World = GetWorld();
	const AHasardChip* Default = GetChipDefault();

	if (!World || !Default)
	{
		return;
	}

	int32& Count = StackCounts.FindOrAdd(Position.PositionId);

	// Height from the class default, not from the mesh: the felt asks the chip how tall
	// a chip is and does not look at how it was built.
	const float ChipHeight = Default->GetChipHeight();

	// Half a chip up for the first one, so the disc rests on the felt rather than half
	// sunk into it. Up vector rather than +Z, so a tilted table still stacks upwards.
	const FVector Base = FeltLocalToWorld(Position.ChipLocation);
	const FVector Lift = GetActorUpVector() * ((Count + 0.5f) * ChipHeight);

	FActorSpawnParameters Params;
	Params.Owner = this;

	if (AHasardChip* Chip = World->SpawnActor<AHasardChip>(
		ChipClass, Base + Lift, GetActorRotation(), Params))
	{
		Chips.Add(Chip);
		++Count;
	}
}

void AHasardFelt::ClearChips()
{
	for (const TObjectPtr<AHasardChip>& Chip : Chips)
	{
		if (Chip)
		{
			Chip->Destroy();
		}
	}

	Chips.Reset();

	// The counts go too. Keeping them would stack the next round's first chip on top of
	// a pile that is no longer there.
	StackCounts.Reset();
}

void AHasardFelt::HandleBetsCleared()
{
	const int32 Removed = Chips.Num();
	ClearChips();

	UE_LOG(LogHasard, Warning, TEXT("Felt: cleared %d chips"), Removed);
}

void AHasardFelt::DrawFelt()
{
	if (!TableLayout)
	{
		UE_LOG(LogHasard, Error, TEXT("Felt: no table layout on BP_Felt"));
		return;
	}

	DrawDebugLayout();
}

void AHasardFelt::RebuildLabels()
{
	if (!TableLayout)
	{
		UE_LOG(LogHasard, Error, TEXT("Felt: no table layout on BP_Felt"));
		return;
	}

	BuildLabels();
	UE_LOG(LogHasard, Warning, TEXT("Felt: built %d labels"), Labels.Num());
}

void AHasardFelt::RebuildSurface()
{
	if (!TableLayout)
	{
		UE_LOG(LogHasard, Error, TEXT("Felt: no table layout on BP_Felt"));
		return;
	}

	SyncBoxToLayout();
	BuildSurface();
}

void AHasardFelt::BuildLabels()
{
	// Destroy first, unconditionally. BeginPlay and the button can both land here, and a
	// pass that only added would print every number twice at the same spot.
	for (const TObjectPtr<UTextRenderComponent>& Existing : Labels)
	{
		if (Existing)
		{
			Existing->DestroyComponent();
		}
	}
	Labels.Reset();

	if (!TableLayout || !Root || !bShowLabels)
	{
		return;
	}

	// One pass over the generated positions, so a label cannot sit anywhere the resolver
	// disagrees with. There is no second list of numbers and no second set of coordinates.
	for (const FHasardBetPosition& Position : TableLayout->GetPositions())
	{
		if (!IsNamedOnFelt(Position.BetType))
		{
			continue;
		}

		UTextRenderComponent* Label = NewObject<UTextRenderComponent>(this);

		// Movable before registering, or setting the transform afterwards warns and is
		// ignored: a component created at runtime defaults to static.
		Label->SetMobility(EComponentMobility::Movable);
		Label->RegisterComponent();
		Label->AttachToComponent(Root, FAttachmentTransformRules::KeepRelativeTransform);
		Label->SetRelativeLocation(
			FVector(Position.ChipLocation.X, Position.ChipLocation.Y, LabelZOffset));
		Label->SetRelativeRotation(LabelRotationFor(Position));
		Label->SetText(LabelTextFor(Position));
		Label->SetWorldSize(LabelTextSize);
		Label->SetHorizontalAlignment(EHTA_Center);
		Label->SetVerticalAlignment(EVRTA_TextCenter);
		Label->SetTextRenderColor(NumeralColor);
		Labels.Add(Label);
	}
}

FTransform AHasardFelt::BoxTransform(const FVector2D& Center, const FVector2D& Size,
	float Height) const
{
	// BoxMesh is a unit quad measured in centimeters, so the scale is the size in
	// centimeters over the mesh's own. Read from the asset rather than assuming 100:
	// swapping in a differently sized plane should change nothing the player sees.
	const FVector MeshSize = BoxMesh ? BoxMesh->GetBounds().BoxExtent * 2.0f : FVector::OneVector;
	const float MeshX = FMath::IsNearlyZero(MeshSize.X) ? 1.0f : MeshSize.X;
	const float MeshY = FMath::IsNearlyZero(MeshSize.Y) ? 1.0f : MeshSize.Y;

	return FTransform(
		FRotator::ZeroRotator,
		FVector(Center.X, Center.Y, Height),
		FVector(Size.X / MeshX, Size.Y / MeshY, 1.0f));
}

UMaterialInstanceDynamic* AHasardFelt::PrepareSurface(UStaticMeshComponent* Component,
	const FColor& Color) const
{
	if (!Component || !BoxMesh || !SurfaceMaterial)
	{
		return nullptr;
	}

	Component->SetStaticMesh(BoxMesh);

	UMaterialInstanceDynamic* Instance = Component->CreateDynamicMaterialInstance(0, SurfaceMaterial);
	if (Instance && !ColorParameterName.IsNone())
	{
		// FromColor, not the FLinearColor constructor: these are authored as sRGB in the
		// details panel, and treating those bytes as linear washes every one of them out.
		Instance->SetVectorParameterValue(ColorParameterName, FLinearColor::FromSRGBColor(Color));
	}

	return Instance;
}

void AHasardFelt::BuildSurface()
{
	// Clear first, unconditionally. BeginPlay and the button can both land here, and a
	// pass that only added would stack a second felt on the first.
	Lines->ClearInstances();
	RedBoxes->ClearInstances();
	BlackBoxes->ClearInstances();
	GreenBoxes->ClearInstances();

	UStaticMeshComponent* const Surfaces[] = {
		Cloth, ZeroBox, Lines, RedBoxes, BlackBoxes, GreenBoxes };

	for (UStaticMeshComponent* Component : Surfaces)
	{
		Component->SetVisibility(bShowSurface);
	}

	if (!TableLayout || !bShowSurface)
	{
		return;
	}

	if (!BoxMesh || !SurfaceMaterial)
	{
		// Loud, because the alternative is an invisible felt that still takes bets - the
		// table would look broken while the money kept moving.
		UE_LOG(LogHasard, Error,
			TEXT("Felt: BP_Felt needs both Box Mesh and Surface Material to print itself"));
		return;
	}

	PrepareSurface(Cloth, ClothColor);
	PrepareSurface(Lines, LineColor);
	PrepareSurface(RedBoxes, RedBoxColor);
	PrepareSurface(BlackBoxes, BlackBoxColor);
	PrepareSurface(GreenBoxes, OutsideBoxColor);
	PrepareSurface(ZeroBox, ZeroBoxColor);

	FVector2D Min, Max;
	TableLayout->GetFeltBounds(Min, Max);

	const FVector2D ClothSize = (Max - Min) + FVector2D(ClothMargin * 2.0f, ClothMargin * 2.0f);
	Cloth->SetRelativeTransform(BoxTransform((Max + Min) * 0.5f, ClothSize, 0.0f));

	int32 Printed = 0;

	for (const FHasardBetPosition& Position : TableLayout->GetPositions())
	{
		// The 83 line bets sit on boundaries and own no area. This is the same test the
		// layout answered when it generated them, asked once rather than restated here.
		if (Position.BoxSize.IsNearlyZero())
		{
			continue;
		}

		// Full size on the plate, inset on the color. The difference is the printed line,
		// so a box smaller than two borders would invert - clamp rather than let it.
		const FVector2D Inset(
			FMath::Max(Position.BoxSize.X - BorderWidth * 2.0f, 0.0f),
			FMath::Max(Position.BoxSize.Y - BorderWidth * 2.0f, 0.0f));

		Lines->AddInstance(
			BoxTransform(Position.ChipLocation, Position.BoxSize, SurfaceZStep));

		const FTransform Face =
			BoxTransform(Position.ChipLocation, Inset, SurfaceZStep * 2.0f);

		if (IsZeroPosition(Position))
		{
			ZeroBox->SetRelativeTransform(Face);
		}
		else
		{
			BoxesFor(Position, *TableLayout)->AddInstance(Face);
		}

		++Printed;
	}

	UE_LOG(LogHasard, Warning, TEXT("Felt: printed %d boxes - %d red, %d black, %d outside"),
		Printed, RedBoxes->GetInstanceCount(), BlackBoxes->GetInstanceCount(),
		GreenBoxes->GetInstanceCount());
}

UInstancedStaticMeshComponent* AHasardFelt::BoxesFor(const FHasardBetPosition& Position,
	const UHasardTableLayout& Layout) const
{
	// An outside bet covers many pockets of both colors, so it has no color of its own
	// and takes the cloth's. Only a straight up is one pocket, and only one pocket has
	// a color to state. Zero never reaches here - BuildSurface routes it out first.
	if (Position.BetType != EHasardBetType::StraightUp)
	{
		return GreenBoxes;
	}

	const int32 Number = Position.CoveredNumbers.Num() == 1
		? Position.CoveredNumbers[0] : INDEX_NONE;

	return Layout.IsRedNumber(Number) ? RedBoxes : BlackBoxes;
}

FText AHasardFelt::LabelTextFor(const FHasardBetPosition& Position) const
{
	if (Position.BetType == EHasardBetType::Column)
	{
		// All three boxes print the same thing on a real layout, which is why the name
		// stays on the position: the log and the readout still have to say which column.
		return NSLOCTEXT("Hasard", "ColumnPays", "2 to 1");
	}

	return Position.DisplayName;
}

FRotator AHasardFelt::LabelRotationFor(const FHasardBetPosition& Position) const
{
	// Only a straight up sits in a square box. Every outside bet - dozens, even money and
	// the column boxes - is wider than it is deep, so its name runs along the long axis or
	// it is printed across the narrow dimension and crowds both rules.
	return Position.BetType == EHasardBetType::StraightUp ? LabelRotation : LabelRotationTurned;
}

void AHasardFelt::DrawDebugLayout() const
{
	const UWorld* World = GetWorld();
	if (!World || !TableLayout)
	{
		return;
	}

	FlushPersistentDebugLines(World);

	FVector2D Min, Max;
	TableLayout->GetFeltBounds(Min, Max);

	const int32 Columns = TableLayout->GetNumberColumns();
	const int32 Rows = TableLayout->GetNumberRows();

	// Outline of the whole felt, including the zero box and both outside bands.
	const FVector2D Corners[4] = {
		FVector2D(Min.X, Min.Y), FVector2D(Max.X, Min.Y),
		FVector2D(Max.X, Max.Y), FVector2D(Min.X, Max.Y) };

	for (int32 Index = 0; Index < 4; ++Index)
	{
		DrawDebugLine(World, FeltLocalToWorld(Corners[Index]),
			FeltLocalToWorld(Corners[(Index + 1) % 4]),
			FColor::White, true, -1.0f, 0, 0.6f);
	}

	// Cell boundaries, drawn from the layout's own origin and cell size. The grid and the
	// resolver therefore read the same two numbers - a grid drawn from a second copy of
	// the arithmetic would look correct and still send clicks to the wrong cell.
	const FVector2D Origin = TableLayout->GetGridOrigin();
	const FVector2D Cell = TableLayout->GetCellSize();
	const float GridFarX = Origin.X + Columns * Cell.X;
	const float GridFarY = Origin.Y + Rows * Cell.Y;

	for (int32 Column = 0; Column <= Columns; ++Column)
	{
		const float LineX = Origin.X + Column * Cell.X;
		DrawDebugLine(World, FeltLocalToWorld(FVector2D(LineX, Origin.Y)),
			FeltLocalToWorld(FVector2D(LineX, GridFarY)),
			FColor(90, 90, 90), true, -1.0f, 0, 0.3f);
	}

	for (int32 Row = 0; Row <= Rows; ++Row)
	{
		const float LineY = Origin.Y + Row * Cell.Y;
		DrawDebugLine(World, FeltLocalToWorld(FVector2D(Origin.X, LineY)),
			FeltLocalToWorld(FVector2D(GridFarX, LineY)),
			FColor(90, 90, 90), true, -1.0f, 0, 0.3f);
	}

	// The zero box, which is the one cell that is not part of the grid.
	DrawDebugLine(World, FeltLocalToWorld(FVector2D(Min.X, Origin.Y)),
		FeltLocalToWorld(FVector2D(Min.X, GridFarY)), FColor(90, 90, 90), true, -1.0f, 0, 0.3f);
	DrawDebugLine(World, FeltLocalToWorld(FVector2D(Min.X, GridFarY)),
		FeltLocalToWorld(FVector2D(Origin.X, GridFarY)), FColor(90, 90, 90), true, -1.0f, 0, 0.3f);
	DrawDebugLine(World, FeltLocalToWorld(FVector2D(Min.X, Origin.Y)),
		FeltLocalToWorld(FVector2D(Origin.X, Origin.Y)), FColor(90, 90, 90), true, -1.0f, 0, 0.3f);

	// Every chip location. Color carries the type only as a hint; the log is the record.
	for (const FHasardBetPosition& Position : TableLayout->GetPositions())
	{
		const bool bInside = Position.CoveredNumbers.Num() <= 6;
		DrawDebugPoint(World, FeltLocalToWorld(Position.ChipLocation), 6.0f,
			bInside ? FColor::Cyan : FColor::Orange, true, -1.0f);
	}

	UE_LOG(LogHasard, Warning, TEXT("Felt: drew %d chip locations, felt is %.0f x %.0f cm"),
		TableLayout->GetPositions().Num(), Max.X - Min.X, Max.Y - Min.Y);
}