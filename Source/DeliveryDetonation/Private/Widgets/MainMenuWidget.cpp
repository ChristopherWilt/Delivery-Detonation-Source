#include "Widgets/MainMenuWidget.h"
#include "Widgets/CodeButtonWithText.h"
#include "Game/BaseGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/Engine.h"
#include "Game/OnlineSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h" 

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // --- Force clean any old sessions when we load the menu ---
    if (UOnlineSessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UOnlineSessionSubsystem>())
    {
        SessionSubsystem->DestroySession();
    }

    if (Play_Button)
    {
        Play_Button->SetLabel(FText::FromString("Play"));
        Play_Button->SetIsEnabled(true);
        Play_Button->OnClicked.AddDynamic(this, &UMainMenuWidget::OnPlayClicked);
    }

    if (Quit_Button)
    {
        Quit_Button->SetLabel(FText::FromString("Quit"));
        Quit_Button->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
    }

    if (Options_Button)
    {
        Options_Button->SetLabel(FText::FromString("Options"));
        Options_Button->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsClicked);
    }
}

void UMainMenuWidget::OnPlayClicked()
{
    if (ModeSelectWidgetClass)
    {
        UUserWidget* ModeSelect = CreateWidget<UUserWidget>(this, ModeSelectWidgetClass);
        if (ModeSelect)
        {
            ModeSelect->AddToViewport();
            //RemoveFromParent(); // Close Main Menu
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("ModeSelectWidgetClass not set in WBP_MainMenu!"));
    }
}

void UMainMenuWidget::OnQuitClicked()
{
    if (UBaseGameInstance* GI = GetGameInstance<UBaseGameInstance>())
    {
        GI->QuitTheGame();
    }
}

void UMainMenuWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void UMainMenuWidget::OnOptionsClicked()
{
    if (!OptionsWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("OptionsWidgetClass not set in WBP_MainMenu!"));
        return;
    }

    UUserWidget* Options = CreateWidget<UUserWidget>(this, OptionsWidgetClass);
    if (Options)
    {
        Options->AddToViewport();
        // We can choose whether main menu stays or hides:
        // RemoveFromParent();
    }
}