#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UCodeButtonWithText;

UCLASS()
class DELIVERYDETONATION_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// --- NEW: Assign WBP_ModeSelect here in the Editor ---
	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<UUserWidget> ModeSelectWidgetClass;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UCodeButtonWithText* Play_Button;

	UPROPERTY(meta = (BindWidget))
	UCodeButtonWithText* Quit_Button;

	UPROPERTY(meta = (BindWidget))
	UCodeButtonWithText* Options_Button;

	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<UUserWidget> OptionsWidgetClass;

	UFUNCTION()
	void OnPlayClicked();

	UFUNCTION()
	void OnQuitClicked();

	UFUNCTION()
	void OnOptionsClicked();
};