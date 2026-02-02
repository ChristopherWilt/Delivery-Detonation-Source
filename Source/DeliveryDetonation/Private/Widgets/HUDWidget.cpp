// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUDWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UHUDWidget::SetDeliveries(int32 Delivered, int32 Total)
{
	if (!TxtDeliveries) return;

	Delivered = FMath::Max(0, Delivered);
	Total = FMath::Max(0, Total);

	const FString S = FString::Printf(TEXT("Delivered: %d / %d"), Delivered, Total);
	TxtDeliveries->SetText(FText::FromString(S));
}

void UHUDWidget::SetTimeSeconds(float SecondsRemaining)
{
	if (!TxtTime) return;

	const int32 TimeInt = FMath::Max(0, (int32)FMath::FloorToInt(SecondsRemaining));
	const FString S = FString::Printf(TEXT("Time: %s"), *FormatMMSS(TimeInt));
	TxtTime->SetText(FText::FromString(S));
}

void UHUDWidget::SetHealth(float Current, float Max)
{
	if (!PB_Health_Front || !PB_Health_Back) return;

	const float SafeMax = FMath::Max(0.0001f, Max);
	const float Ratio = FMath::Clamp(Current / SafeMax, 0.0f, 1.0f);

	PB_Health_Front->SetPercent(Ratio);

	PB_Health_Back->SetPercent(1.0f);
}

FString UHUDWidget::FormatMMSS(int32 TotalSeconds)
{
	const int32 Minutes = TotalSeconds / 60;
	const int32 Seconds = TotalSeconds % 60;
	return FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
}

void UHUDWidget::SetCaffeine(float Current, float Max)
{
	if (!PB_Caffeine_Front || !PB_Caffeine_Back) return;

	const float SafeMax = FMath::Max(0.0001f, Max);
	const float Ratio = FMath::Clamp(Current / SafeMax, 0.0f, 1.0f);

	PB_Caffeine_Front->SetPercent(Ratio);

	PB_Caffeine_Back->SetPercent(1.0f);
}

void UHUDWidget::SetPickupPromptVisible(bool bVisible)
{
	SetInteractPrompt(bVisible, TEXT("Press E to Pick Up"));
}

void UHUDWidget::SetInteractPrompt(bool bVisible, const FString& PromptText)
{
	if (!TxtPickupPrompt) return;

	TxtPickupPrompt->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	if (bVisible)
	{
		TxtPickupPrompt->SetText(FText::FromString(PromptText));
	}
}
