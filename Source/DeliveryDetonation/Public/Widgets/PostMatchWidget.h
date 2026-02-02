// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PostMatchWidget.generated.h"

class ADeliveryDetonation_GameState;
class ADeliveryDetonation_PlayerState;
class UTextBlock;
class UCodeButtonWithText;

/**
 * 
 */
UCLASS()
class DELIVERYDETONATION_API UPostMatchWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// --- WBP Bindings (names MUST match in WBP_PostMatch) ---
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TxtResult = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TxtStatus = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TxtReady = nullptr;	

	UPROPERTY(meta = (BindWidget))
	UCodeButtonWithText* Ready_Button = nullptr;

	UPROPERTY(meta = (BindWidget))
	UCodeButtonWithText* Quit_Button = nullptr;

	// --- Button handlers ---
	UFUNCTION()
	void OnReadyClicked();

	UFUNCTION()
	void OnQuitClicked();

	// --- UI update ---
	void Refresh();

	void UpdateResultText(bool bIsWinner);
	void UpdateReadyText(bool bReady);
	void UpdateStatusText(int32 CountdownSeconds);

private:

	bool bLocalReady = false;

	FTimerHandle RefreshTimerHandle;
};
