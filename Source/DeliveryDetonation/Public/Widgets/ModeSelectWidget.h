#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ModeSelectWidget.generated.h"

class UButton;

UCLASS()
class DELIVERYDETONATION_API UModeSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Assign WBP_LanLobby here
	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<UUserWidget> LanLobbyClass;

	// Assign WBP_OnlineLobby here
	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<UUserWidget> OnlineLobbyClass;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION() void OnLanClicked();
	UFUNCTION() void OnOnlineClicked();
	UFUNCTION() void OnBackClicked();

	UPROPERTY(meta = (BindWidget)) UButton* LanButton;
	UPROPERTY(meta = (BindWidget)) UButton* OnlineButton;
	UPROPERTY(meta = (BindWidget)) UButton* BackButton;

	UPROPERTY(EditAnywhere) TSubclassOf<UUserWidget> MainMenuClass;
};