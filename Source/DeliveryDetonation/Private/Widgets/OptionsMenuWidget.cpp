// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/OptionsMenuWidget.h"
#include "Components/Slider.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Widgets/CodeButtonWithText.h"
#include "Widgets/PauseMenuWidget.h"
//For Saving
#include "Game/BaseGameInstance.h"

void UOptionsMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UBaseGameInstance* GI = GetGameInstance<UBaseGameInstance>())
	{
		DefaultVolume = GI->GetMasterVolume();
	}

	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->SetValue(DefaultVolume);
		MasterVolumeSlider->OnValueChanged.AddDynamic(this, &UOptionsMenuWidget::OnMasterVolumeChanged);
	}

	if (Back_Button)
	{
		Back_Button->SetLabel(FText::FromString("Back"));
		Back_Button->OnClicked.AddDynamic(this, &UOptionsMenuWidget::OnBackClicked);
	}
}

void UOptionsMenuWidget::OnMasterVolumeChanged(float Value)
{
	ApplyMasterVolume(Value);

	if (UBaseGameInstance* GI = GetGameInstance<UBaseGameInstance>())
	{
	    GI->SetMasterVolume(Value);
	}
}

void UOptionsMenuWidget::ApplyMasterVolume(float Value)
{
	if (!MasterMix || !MasterClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[OptionsMenu] MasterMix or MasterClass not set."));
		return;
	}

	UGameplayStatics::SetSoundMixClassOverride(
		this,
		MasterMix,
		MasterClass,
		Value,   // Volume
		1.0f,    // Pitch
		0.0f,    // Fade in time
		true     // Apply to children
	);

	UGameplayStatics::PushSoundMixModifier(this, MasterMix);
}

void UOptionsMenuWidget::SetOwningPauseMenu(UPauseMenuWidget* InPauseMenu)
{
	OwningPauseMenu = InPauseMenu;
}

void UOptionsMenuWidget::OnBackClicked()
{
	// Let pause menu restore itself
	if (OwningPauseMenu)
	{
		OwningPauseMenu->ReturnFromOptions();
		return; // pause menu will remove us
	}

	// Fallback if not set
	RemoveFromParent();
}
