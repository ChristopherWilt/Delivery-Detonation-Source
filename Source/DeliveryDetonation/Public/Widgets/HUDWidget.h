// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDWidget.generated.h"

class UTextBlock;
class UProgressBar;
/**
 * 
 */
UCLASS()
class DELIVERYDETONATION_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Called by GameMode
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetDeliveries(int32 Delivered, int32 Total);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetTimeSeconds(float SecondsRemaining);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetHealth(float Current, float Max);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetCaffeine(float Current, float Max);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetPickupPromptVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "HUD")
	void SetInteractPrompt(bool bVisible, const FString& PromptText);

protected:
	// These names MUST match the widget names in WBP
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TxtDeliveries;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TxtTime;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_Health_Back;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_Health_Front;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_Caffeine_Back;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PB_Caffeine_Front;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TxtPickupPrompt;

private:
	static FString FormatMMSS(int32 TotalSeconds);
	
};
