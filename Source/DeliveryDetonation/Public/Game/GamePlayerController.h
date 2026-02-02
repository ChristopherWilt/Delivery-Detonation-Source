// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "GamePlayerController.generated.h"

class UPauseMenuWidget;
class UHUDWidget;
class APlayerCharacter;
/**
 * 
 */
UCLASS()
class DELIVERYDETONATION_API AGamePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetGameOnlyInput();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetUIOnlyInput(UUserWidget* FocusWidget);

	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();
	void HidePauseMenu();

	UFUNCTION(Server, Reliable)
	void Server_RestartLevel();

	UFUNCTION(BlueprintCallable, Category = "Match")
	void SetReadyUp(bool bReady);

	UFUNCTION(Server, Reliable)
	void Server_SetReadyUp(bool bReady);

	UFUNCTION(BlueprintCallable, Category = "Match")
	void QuitMatchToMenu();

	UFUNCTION(Client, Reliable)
	void Client_EnterPostMatchUI();

	UFUNCTION(Client, Reliable)
	void Client_ShowPostMatch();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPauseMenuWidget> PauseWidgetClass;

	UPROPERTY()
	UPauseMenuWidget* PauseWidgetInstance = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UPostMatchWidget> PostMatchWidgetClass;

	UPROPERTY()
	UPostMatchWidget* PostMatchWidgetInstance = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UHUDWidget> HUDWidgetClass;

	UPROPERTY()
	UHUDWidget* HUDWidgetInstance = nullptr;

	void EnsureHUD();

private:
	void ShowPauseMenu();
	virtual void SetupInputComponent() override;

	UFUNCTION(BlueprintCallable, Category = "Match")
	void ShowPostMatch();
};
