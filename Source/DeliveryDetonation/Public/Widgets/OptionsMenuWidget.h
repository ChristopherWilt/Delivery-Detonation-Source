// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OptionsMenuWidget.generated.h"

class USlider;
class UCodeButtonWithText;
class USoundMix;
class USoundClass;
class UPauseMenuWidget;
/**
 * 
 */
UCLASS()
class DELIVERYDETONATION_API UOptionsMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetOwningPauseMenu(UPauseMenuWidget* InPauseMenu);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	USlider* MasterVolumeSlider;

	UPROPERTY(meta = (BindWidget))
	UCodeButtonWithText* Back_Button;

	// Assign these in the widget defaults
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundMix* MasterMix = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* MasterClass = nullptr;

	// default if nothing is saved yet
	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	float DefaultVolume = 1.0f;

	UFUNCTION()
	void OnMasterVolumeChanged(float Value);

	UFUNCTION()
	void OnBackClicked();

private:
	void ApplyMasterVolume(float Value);

	UPROPERTY()
	UPauseMenuWidget* OwningPauseMenu = nullptr;
	
};
