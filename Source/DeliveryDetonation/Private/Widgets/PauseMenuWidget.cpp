// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/PauseMenuWidget.h"
#include "Widgets/CodeButtonWithText.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Game/BaseGameInstance.h"
#include "Game/GamePlayerController.h"
#include "Widgets/OptionsMenuWidget.h"

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Resume_Button)
	{
		Resume_Button->SetLabel(FText::FromString("Resume"));
		Resume_Button->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnResumeClicked);
	}

	if (Restart_Button)
	{
		Restart_Button->SetLabel(FText::FromString("Restart"));
		Restart_Button->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnRestartClicked);
	}

	if (Menu_Button)
	{
		Menu_Button->SetLabel(FText::FromString("Main Menu"));
		Menu_Button->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnMenuClicked);
	}

	if (Quit_Button)
	{
		Quit_Button->SetLabel(FText::FromString("Quit"));
		Quit_Button->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnQuitClicked);
	}

	if (Options_Button)
	{
		Options_Button->SetLabel(FText::FromString("Options"));
		Options_Button->OnClicked.AddDynamic(this, &UPauseMenuWidget::OnOptionsClicked);
	}

	// Optional: default focus
	if (Resume_Button)
	{
		Resume_Button->Focus();
	}
}

void UPauseMenuWidget::OnResumeClicked()
{
	if (AGamePlayerController* GPC = Cast<AGamePlayerController>(GetOwningPlayer()))
	{
		GPC->HidePauseMenu(); // This handles unpausing AND restoring input
	}
}

void UPauseMenuWidget::OnRestartClicked()
{
	if (AGamePlayerController* GPC = Cast<AGamePlayerController>(GetOwningPlayer()))
	{
		// Call the server function
		GPC->Server_RestartLevel();
	}
}

void UPauseMenuWidget::OnMenuClicked()
{
	if (UBaseGameInstance* GI = GetGameInstance<UBaseGameInstance>())
	{
		//UE_LOG(LogTemp, Warning, TEXT("[PauseMenu] Main Menu clicked"));
		GI->LoadMenuLevel();
		return;
	}

	// Fallback if GI isn't set up
	//UGameplayStatics::OpenLevel(this, FName(TEXT("CodeMenuMap")));
}

void UPauseMenuWidget::OnQuitClicked()
{
	if (UBaseGameInstance* GI = GetGameInstance<UBaseGameInstance>())
	{
		GI->QuitTheGame();
	}
}

void UPauseMenuWidget::OnOptionsClicked()
{
	if (!OptionsWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("OptionsWidgetClass not set in WBP_PauseMenu!"));
		return;
	}

	// Create once
	if (!OptionsWidgetInstance)
	{
		OptionsWidgetInstance = CreateWidget<UUserWidget>(GetOwningPlayer(), OptionsWidgetClass);

		if (UOptionsMenuWidget* OptionsTyped = Cast<UOptionsMenuWidget>(OptionsWidgetInstance))
		{
			OptionsTyped->SetOwningPauseMenu(this);
		}
	}

	if (!OptionsWidgetInstance)
		return;

	if (!OptionsWidgetInstance->IsInViewport())
	{
		OptionsWidgetInstance->AddToViewport(/*ZOrder=*/200);
	}

	SetVisibility(ESlateVisibility::Hidden);

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(OptionsWidgetInstance->TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = true;
	}
}

void UPauseMenuWidget::ReturnFromOptions()
{
	// Show pause menu again
	SetVisibility(ESlateVisibility::Visible);

	if (OptionsWidgetInstance && OptionsWidgetInstance->IsInViewport())
	{
		OptionsWidgetInstance->RemoveFromParent();
	}

	// Focus pause menu again
	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(Mode);
		PC->bShowMouseCursor = true;
	}
}