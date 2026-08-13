// Copyright Picardbuilds. All Rights Reserved.

#include "HasardFelt.h"
#include "HasardBettingComponent.h"
#include "HasardTableLayout.h"
#include "HasardTypes.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

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
}

void AHasardFelt::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Resized here as well as in BeginPlay, so assigning the asset in the editor
	// shows the right box immediately rather than after the next Play.
	SyncBoxToLayout();
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

	BuildLabels();

	const int32 Count = TableLayout->GetPositions().Num();
	UE_LOG(LogHasard, Warning, TEXT("Felt: %d positions, %d labels, stake %d per click"),
		Count, Labels.Num(), StakePerClick);

	if (bDrawDebugLayout)
	{
		DrawDebugLayout();
	}
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

	if (!Betting->PlaceBet(PositionId, StakePerClick))
	{
		// Refused, so no stake was taken. Nothing may be drawn as though one was.
		return;
	}

	// Where the chip actually went, not where the player clicked. If these two are ever
	// far apart the resolver picked a neighbor, and seeing it is how you notice.
	if (const UWorld* World = GetWorld())
	{
		DrawDebugSphere(World, FeltLocalToWorld(Position->ChipLocation),
			2.0f, 8, FColor::Yellow, false, 4.0f);
	}
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
		Label->SetRelativeRotation(LabelRotation);
		Label->SetText(Position.DisplayName);
		Label->SetWorldSize(LabelTextSize);
		Label->SetHorizontalAlignment(EHTA_Center);
		Label->SetVerticalAlignment(EVRTA_TextCenter);
		Label->SetTextRenderColor(LabelColorFor(Position, *TableLayout));
		Labels.Add(Label);
	}
}

FColor AHasardFelt::LabelColorFor(const FHasardBetPosition& Position,
	const UHasardTableLayout& Layout) const
{
	if (Position.BetType != EHasardBetType::StraightUp)
	{
		return OutsideLabelColor;
	}

	// A straight up covers exactly one pocket. Reading past that would be reading a
	// position this function was not given.
	const int32 Number = Position.CoveredNumbers.Num() == 1
		? Position.CoveredNumbers[0] : INDEX_NONE;

	if (Number == 0)
	{
		return ZeroLabelColor;
	}

	return Layout.IsRedNumber(Number) ? RedNumberColor : BlackNumberColor;
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