// Copyright Picardbuilds. All Rights Reserved.

#include "HasardHUDWidget.h"
#include "Components/TextBlock.h"

void UHasardHUDWidget::SetBankroll(int32 Balance, int32 TotalStaked, int32 SessionNetChange)
{
	// No null checks. BindWidget already made these compile-time guarantees, and a
	// defensive check here would only hide the Blueprint error you want to see.
	BalanceText->SetText(FText::AsNumber(Balance));
	StakedText->SetText(FText::AsNumber(TotalStaked));

	// Signed, always. "%+d" prints -60 as -60 and 40 as +40, so a losing session
	// can never read as a balance that merely happens to be smaller.
	NetText->SetText(FText::FromString(FString::Printf(TEXT("%+d"), SessionNetChange)));
}

void UHasardHUDWidget::SetSessionTime(float ElapsedSeconds)
{
	const int32 TotalSeconds = FMath::FloorToInt(ElapsedSeconds);
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;

	TimeText->SetText(FText::FromString(
		FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
}

void UHasardHUDWidget::SetHouseEdge(float EdgePercent)
{
	FNumberFormattingOptions Opts;
	Opts.MinimumFractionalDigits = 2;
	Opts.MaximumFractionalDigits = 2;

	// FText::Format, not Printf: this is the one line on the HUD a player has to
	// read as a sentence, and a sentence has to survive translation.
	OddsText->SetText(FText::Format(
		NSLOCTEXT("Hasard", "HouseEdge", "Every bet here costs {0}% of what you stake"),
		FText::AsNumber(EdgePercent, &Opts)));
}

void UHasardHUDWidget::SetHouseEdgeUnknown()
{
	OddsText->SetText(NSLOCTEXT("Hasard", "HouseEdgeUnknown",
		"House edge unavailable - the payout table is missing or inconsistent"));
}

void UHasardHUDWidget::SetBetPreview(const FText& InPreviewText)
{
	PreviewText->SetText(InPreviewText);
}

void UHasardHUDWidget::ClearBetPreview()
{
	// Blank rather than the last bet. A price left standing after the aim moved is a
	// price for a bet the player is not making.
	PreviewText->SetText(FText::GetEmpty());
}

void UHasardHUDWidget::NotifyRealityCheckDismissed()
{
	OnRealityCheckDismissed.Broadcast();
}