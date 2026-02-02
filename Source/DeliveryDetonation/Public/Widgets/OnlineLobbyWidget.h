#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/OnlineSessionSubsystem.h" 
#include "OnlineLobbyWidget.generated.h"

class UButton;
class UEditableTextBox;
class USlider;
class UTextBlock;
class UScrollBox;
class UWidgetSwitcher;

UCLASS()
class DELIVERYDETONATION_API UOnlineLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Config")
	TSubclassOf<UUserWidget> ServerRowClass;


protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION() void OnHostTabClicked();
	UFUNCTION() void OnJoinTabClicked();
	UFUNCTION() void OnStartHostClicked();
	UFUNCTION() void OnRefreshClicked();
	UFUNCTION() void OnBackClicked();
	UFUNCTION() void OnSliderValueChanged(float Value);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maps")
	FName SelectedMapName = "Level1";

	// --- Search Functions ---
	UFUNCTION()
	void OnSearchTextChanged(const FText& Text);

	UFUNCTION()
	void UpdateServerList(const TArray<FServerInfo>& ServerInfos);

	// Helper to redraw the list based on the cached data + filter
	void RefreshServerListUI();

	// --- Local Cache ---
	TArray<FServerInfo> CachedServerList;

	// --- BIND WIDGETS ---
	// Navigation
	UPROPERTY(meta = (BindWidget)) UButton* HostTabButton;
	UPROPERTY(meta = (BindWidget)) UButton* JoinTabButton;
	UPROPERTY(meta = (BindWidget)) UButton* BackButton;
	UPROPERTY(meta = (BindWidget)) UWidgetSwitcher* MenuSwitcher;

	// HOST Page
	UPROPERTY(meta = (BindWidget)) UEditableTextBox* ServerNameInput;
	UPROPERTY(meta = (BindWidget)) USlider* MaxPlayersSlider;
	UPROPERTY(meta = (BindWidget)) UTextBlock* MaxPlayersValueText;
	UPROPERTY(meta = (BindWidget)) UButton* HostGameButton;

	// JOIN Page
	UPROPERTY(meta = (BindWidget)) UButton* RefreshButton;
	UPROPERTY(meta = (BindWidget)) UScrollBox* ServerListScrollBox;

	// --- Search Box Widget ---
	UPROPERTY(meta = (BindWidget)) UEditableTextBox* SearchServerInput;
};