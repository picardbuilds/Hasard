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

void UHasardHUDWidget::NotifyRealityCheckDismissed()
{
	OnRealityCheckDismissed.Broadcast();
}