// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/MenuPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Game/OnlineSessionSubsystem.h"

void AMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();

    if (IsLocalController())
    {
        if (UOnlineSessionSubsystem* SessionSubsystem = GetGameInstance()->GetSubsystem<UOnlineSessionSubsystem>())
        {
            SessionSubsystem->LoginWithDeviceID();
        }

        if (MenuWidgetClass)
        {
            MenuWidgetInstance = CreateWidget<UUserWidget>(this, MenuWidgetClass);
            if (MenuWidgetInstance)
            {
                MenuWidgetInstance->AddToViewport();
            }
        }

        bShowMouseCursor = true;
        FInputModeUIOnly UIOnly;
        UIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(UIOnly);
    }
}
