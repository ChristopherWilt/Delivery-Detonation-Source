// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UCodeButtonWithText;
/**
 * 
 */
UCLASS()
class DELIVERYDETONATION_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UCodeButtonWithText* Resume_Button;

	UPROPERTY(meta = (BindWidget))
	UCodeButtonWithText* Restart_Button;

	UPROPERTY(meta = (BindWidget))
	UCodeButtonWithText* Menu_Button;

	UPROPERTY(meta = (BindWidget))
	UCodeButtonWithText* Quit_Button;

	UPROPERTY(meta = (BindWidget))
	UCodeButtonWithText* Options_Button;

	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<UUserWidget> OptionsWidgetClass;

	UPROPERTY()
	UUserWidget* OptionsWidgetInstance = nullptr;

	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnRestartClicked();

	UFUNCTION()
	void OnMenuClicked();

	UFUNCTION()
	void OnQuitClicked();

	UFUNCTION()
	void OnOptionsClicked();

public:
	void ReturnFromOptions();
};
