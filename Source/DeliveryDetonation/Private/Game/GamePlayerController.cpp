// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GamePlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/PauseMenuWidget.h"
#include "Engine/World.h"
#include "Game/DeliveryDetonation_GameMode.h"
#include "Game/OnlineSessionSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/PostMatchWidget.h"
#include "Game/DeliveryDetonation_GameState.h"
#include "Widgets/HUDWidget.h"
#include "Player/PlayerCharacter.h"

void AGamePlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetGameOnlyInput();
	EnsureHUD();

	if (APlayerCharacter* PC = Cast<APlayerCharacter>(GetPawn()))
	{
		PC->InitHUD(HUDWidgetInstance);
	}
}

void AGamePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!IsLocalController()) return;

	EnsureHUD();

	if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(InPawn))
	{
		// every respawn, rebind the NEW pawn’s components to the SAME HUD
		PlayerChar->InitHUD(HUDWidgetInstance);
	}
}

void AGamePlayerController::SetReadyUp(bool bReady)
{
	Server_SetReadyUp(bReady);
}

void AGamePlayerController::Server_SetReadyUp_Implementation(bool bReady)
{
	if (ADeliveryDetonation_GameMode* GM = GetWorld()->GetAuthGameMode<ADeliveryDetonation_GameMode>())
	{
		GM->Server_SetPlayerReady(this, bReady);
	}
}

void AGamePlayerController::QuitMatchToMenu()
{

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UOnlineSessionSubsystem* OSS = GI->GetSubsystem<UOnlineSessionSubsystem>())
		{
			if (HasAuthority())
			{
				// Host
				OSS->DestroySession();
			}
			else
			{
				// Client
				OSS->LeaveSession();
			}
		}
	}

	UGameplayStatics::OpenLevel(this, FName("MainMenuMap"));
}

void AGamePlayerController::SetGameOnlyInput()
{
	bShowMouseCursor = false;

	FInputModeGameOnly Mode;
	SetInputMode(Mode);

	SetIgnoreLookInput(false);
	SetIgnoreMoveInput(false);
}

void AGamePlayerController::SetUIOnlyInput(UUserWidget* FocusWidget)
{
	bShowMouseCursor = true;

	// Use GameAndUI so the Character can still "hear" the Pause action
	FInputModeGameAndUI Mode;
	if (FocusWidget)
	{
		Mode.SetWidgetToFocus(FocusWidget->TakeWidget());
	}
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	// This allows the UI to have focus but doesn't block "Trigger When Paused" actions
	SetInputMode(Mode);

	SetIgnoreLookInput(true);
	SetIgnoreMoveInput(true);
}

void AGamePlayerController::TogglePauseMenu()
{
	if (IsPaused())
	{
		HidePauseMenu();
	}
	else
	{
		ShowPauseMenu();
	}
}

void AGamePlayerController::ShowPauseMenu()
{
	if (!PauseWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] PauseWidgetClass not set"));
		return;
	}

	SetPause(true);

	if (!PauseWidgetInstance)
	{
		PauseWidgetInstance = CreateWidget<UPauseMenuWidget>(this, PauseWidgetClass);
	}

	if (PauseWidgetInstance)
	{
		PauseWidgetInstance->AddToViewport(200);
		SetUIOnlyInput(PauseWidgetInstance);
	}
}

void AGamePlayerController::HidePauseMenu()
{
	SetPause(false);

	if (PauseWidgetInstance)
	{
		PauseWidgetInstance->RemoveFromParent();
	}

	SetGameOnlyInput();
}

void AGamePlayerController::Server_RestartLevel_Implementation()
{
	// Only the server runs this
	if (UWorld* World = GetWorld())
	{
		// Restart the game for everyone
		World->ServerTravel("?Restart");
	}
}

void AGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAction("Pause", IE_Pressed, this, &AGamePlayerController::TogglePauseMenu);
}

void AGamePlayerController::Client_EnterPostMatchUI_Implementation()
{
	if (!PostMatchWidgetClass) return;

	if (!PostMatchWidgetInstance)
	{
		PostMatchWidgetInstance = CreateWidget<UPostMatchWidget>(
			this, PostMatchWidgetClass);
	}

	if (PostMatchWidgetInstance && !PostMatchWidgetInstance->IsInViewport())
	{
		PostMatchWidgetInstance->AddToViewport(300);
	}

	SetUIOnlyInput(PostMatchWidgetInstance);
}

void AGamePlayerController::ShowPostMatch()
{
	if (!PostMatchWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PC] PostMatchWidgetClass not set"));
		return;
	}

	if (!PostMatchWidgetInstance)
	{
		PostMatchWidgetInstance = CreateWidget<UPostMatchWidget>(this, PostMatchWidgetClass);
	}

	if (PostMatchWidgetInstance && !PostMatchWidgetInstance->IsInViewport())
	{
		PostMatchWidgetInstance->AddToViewport(300);
	}

	// UI input mode like pause menu
	bShowMouseCursor = true;

	FInputModeUIOnly Mode;
	Mode.SetWidgetToFocus(PostMatchWidgetInstance->TakeWidget());
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);

	SetIgnoreLookInput(true);
	SetIgnoreMoveInput(true);
}

void AGamePlayerController::Client_ShowPostMatch_Implementation()
{
	ShowPostMatch();
}

void AGamePlayerController::EnsureHUD()
{
	if (!IsLocalController()) return;
	if (!HUDWidgetClass) return;

	if (!HUDWidgetInstance)
	{
		HUDWidgetInstance = CreateWidget<UHUDWidget>(this, HUDWidgetClass);
		if (HUDWidgetInstance)
		{
			HUDWidgetInstance->AddToViewport(50); // lower than pause/postmatch
		}
	}
}