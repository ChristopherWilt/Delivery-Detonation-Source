#include "Widgets/ModeSelectWidget.h"
#include "Components/Button.h"
#include "Game/BaseGameInstance.h" // Needed for LoadFirstLevel

void UModeSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (LanButton) LanButton->OnClicked.AddDynamic(this, &UModeSelectWidget::OnLanClicked);

	if (OnlineButton)
	{
		OnlineButton->OnClicked.AddDynamic(this, &UModeSelectWidget::OnOnlineClicked);
		OnlineButton->SetIsEnabled(true);
		OnlineButton->SetToolTipText(FText::FromString("Play Online via EOS"));
	}

	if (BackButton) BackButton->OnClicked.AddDynamic(this, &UModeSelectWidget::OnBackClicked);
}

void UModeSelectWidget::OnLanClicked()
{
	if (LanLobbyClass)
	{
		UUserWidget* Lobby = CreateWidget<UUserWidget>(this, LanLobbyClass);
		if (Lobby)
		{
			Lobby->AddToViewport();
		}
	}
}

void UModeSelectWidget::OnOnlineClicked()
{
	if (OnlineLobbyClass)
	{
		UUserWidget* Lobby = CreateWidget<UUserWidget>(this, OnlineLobbyClass);
		if (Lobby)
		{
			Lobby->AddToViewport();
		}
	}
	else
	{
		// Fallback if you forgot to assign the widget
		UE_LOG(LogTemp, Error, TEXT("OnlineLobbyClass not set in WBP_ModeSelect!"));
	}
}

void UModeSelectWidget::OnBackClicked()
{
	// 1. Remove Self
	RemoveFromParent();

	// 2. Re-create Main Menu (You need a reference to the Main Menu Class)
	if (MainMenuClass)
	{
		CreateWidget<UUserWidget>(this, MainMenuClass)->AddToViewport();
	}
}