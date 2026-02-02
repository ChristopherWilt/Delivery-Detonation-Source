// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/PostMatchWidget.h"
#include "Components/TextBlock.h"
#include "Widgets/CodeButtonWithText.h"
#include "Game/GamePlayerController.h"
#include "Game/DeliveryDetonation_GameState.h"
#include "Player/DeliveryDetonation_PlayerState.h"
#include "TimerManager.h"
#include "Engine/World.h"

void UPostMatchWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Wire buttons like your pause menu
	if (Ready_Button)
	{
		Ready_Button->SetLabel(FText::FromString("Ready Up"));
		Ready_Button->OnClicked.AddDynamic(this, &UPostMatchWidget::OnReadyClicked);
	}

	if (Quit_Button)
	{
		Quit_Button->SetLabel(FText::FromString("Quit To Menu"));
		Quit_Button->OnClicked.AddDynamic(this, &UPostMatchWidget::OnQuitClicked);
	}

	// Initialize from replicated state
	Refresh();

	// Keep UI synced as RestartCountdown / PlayerState replicates
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			RefreshTimerHandle,
			this,
			&UPostMatchWidget::Refresh,
			0.25f,   // 4 times/sec is plenty
			true
		);
	}
}

void UPostMatchWidget::NativeDestruct()
{
	// Clear timer
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}

	Super::NativeDestruct();
}

void UPostMatchWidget::OnReadyClicked()
{
	// Toggle local and send to server through your PC function
	bLocalReady = !bLocalReady;

	if (AGamePlayerController* PC = Cast<AGamePlayerController>(GetOwningPlayer()))
	{
		PC->SetReadyUp(bLocalReady);
	}

	// Update immediately so it feels responsive
	UpdateReadyText(bLocalReady);

	if (Ready_Button)
	{
		Ready_Button->SetLabel(bLocalReady
			? FText::FromString("Unready")
			: FText::FromString("Ready Up"));
	}
}

void UPostMatchWidget::OnQuitClicked()
{
	if (AGamePlayerController* PC = Cast<AGamePlayerController>(GetOwningPlayer()))
	{
		PC->QuitMatchToMenu();
	}
}

void UPostMatchWidget::Refresh()
{
	// Read PlayerState (winner + ready)
	if (ADeliveryDetonation_PlayerState* PS = GetOwningPlayerState<ADeliveryDetonation_PlayerState>())
	{
		UpdateResultText(PS->IsWinner());

		// Keep local cached value aligned with replicated value
		const bool bRepReady = PS->IsReady();
		bLocalReady = bRepReady;

		UpdateReadyText(bRepReady);

		if (Ready_Button)
		{
			Ready_Button->SetLabel(bRepReady
				? FText::FromString("Unready")
				: FText::FromString("Ready Up"));
		}
	}

	// Read GameState (countdown / waiting)
	if (UWorld* World = GetWorld())
	{
		if (ADeliveryDetonation_GameState* GS = World->GetGameState<ADeliveryDetonation_GameState>())
		{
			UpdateStatusText(GS->RestartCountdown);
		}
	}
}

void UPostMatchWidget::UpdateResultText(bool bIsWinner)
{
	if (!TxtResult) return;

	TxtResult->SetText(bIsWinner
		? FText::FromString("YOU WIN")
		: FText::FromString("YOU LOSE"));
}

void UPostMatchWidget::UpdateReadyText(bool bReady)
{
	if (!TxtReady) return;

	TxtReady->SetText(bReady
		? FText::FromString("Ready")
		: FText::FromString("Not Ready"));
}

void UPostMatchWidget::UpdateStatusText(int32 CountdownSeconds)
{
	if (!TxtStatus) return;

	if (CountdownSeconds > 0)
	{
		TxtStatus->SetText(FText::FromString(
			FString::Printf(TEXT("Restarting in %d..."), CountdownSeconds)
		));
	}
	else
	{
		TxtStatus->SetText(FText::FromString("Waiting for players..."));
	}
}