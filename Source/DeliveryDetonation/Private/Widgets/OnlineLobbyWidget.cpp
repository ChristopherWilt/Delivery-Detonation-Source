#include "Widgets/OnlineLobbyWidget.h"
#include "Widgets/ServerRowWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/WidgetSwitcher.h"
#include "Game/BaseGameInstance.h"

void UOnlineLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind Navigation
	if (HostTabButton) HostTabButton->OnClicked.AddDynamic(this, &UOnlineLobbyWidget::OnHostTabClicked);
	if (JoinTabButton) JoinTabButton->OnClicked.AddDynamic(this, &UOnlineLobbyWidget::OnJoinTabClicked);
	if (BackButton) BackButton->OnClicked.AddDynamic(this, &UOnlineLobbyWidget::OnBackClicked);

	// Bind Actions
	if (HostGameButton) HostGameButton->OnClicked.AddDynamic(this, &UOnlineLobbyWidget::OnStartHostClicked);
	if (RefreshButton) RefreshButton->OnClicked.AddDynamic(this, &UOnlineLobbyWidget::OnRefreshClicked);

	if (MaxPlayersSlider)
	{
		MaxPlayersSlider->OnValueChanged.AddDynamic(this, &UOnlineLobbyWidget::OnSliderValueChanged);
		OnSliderValueChanged(MaxPlayersSlider->GetValue());
	}

	// --- Bind Search Input ---
	if (SearchServerInput)
	{
		SearchServerInput->OnTextChanged.AddDynamic(this, &UOnlineLobbyWidget::OnSearchTextChanged);
	}

	// --- Subscribe to Subsystem Updates ---
	if (UOnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UOnlineSessionSubsystem>())
	{
		OSS->OnServerListUpdatedEvent.AddDynamic(this, &UOnlineLobbyWidget::UpdateServerList);
	}
}

void UOnlineLobbyWidget::NativeDestruct()
{
	if (UOnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UOnlineSessionSubsystem>())
	{
		OSS->OnServerListUpdatedEvent.RemoveDynamic(this, &UOnlineLobbyWidget::UpdateServerList);
	}
	Super::NativeDestruct();
}

void UOnlineLobbyWidget::OnHostTabClicked()
{
	if (MenuSwitcher) MenuSwitcher->SetActiveWidgetIndex(0);
}

void UOnlineLobbyWidget::OnJoinTabClicked()
{
	if (MenuSwitcher) MenuSwitcher->SetActiveWidgetIndex(1);
	OnRefreshClicked(); // Auto refresh
}

void UOnlineLobbyWidget::OnBackClicked()
{
	RemoveFromParent();
}

void UOnlineLobbyWidget::OnStartHostClicked()
{
	if (UOnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UOnlineSessionSubsystem>())
	{
		FString ServerName = ServerNameInput ? ServerNameInput->GetText().ToString() : "My EOS Server";
		if (ServerName.IsEmpty()) ServerName = "Online Game";

		int32 MaxPlayers = MaxPlayersSlider ? (int32)MaxPlayersSlider->GetValue() : 4;

		// --- CRITICAL CHANGE: bIsLAN = false ---
		OSS->HostDeliverySession(ServerName, SelectedMapName.ToString(), MaxPlayers, false);
	}
}

void UOnlineLobbyWidget::OnRefreshClicked()
{
	if (ServerListScrollBox) ServerListScrollBox->ClearChildren();

	if (UOnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UOnlineSessionSubsystem>())
	{
		// --- CRITICAL CHANGE: bIsLAN = false ---
		OSS->FindSessions(false);
	}
}

// Store the data, then update the UI
void UOnlineLobbyWidget::UpdateServerList(const TArray<FServerInfo>& ServerInfos)
{
	// 1. Cache the raw list from EOS
	CachedServerList = ServerInfos;

	// 2. Refresh the visual list (this will apply any current filter)
	RefreshServerListUI();
}

void UOnlineLobbyWidget::OnSearchTextChanged(const FText& Text)
{
	RefreshServerListUI();
}

// The actual filtering logic
void UOnlineLobbyWidget::RefreshServerListUI()
{
	if (!ServerListScrollBox || !ServerRowClass) return;

	ServerListScrollBox->ClearChildren();

	// Get search text (lowercase for case-insensitive search)
	FString FilterText = SearchServerInput ? SearchServerInput->GetText().ToString().ToLower() : "";

	for (const FServerInfo& Info : CachedServerList)
	{
		// Filter Logic: If filter is not empty, check if Server Name contains it
		if (!FilterText.IsEmpty() && !Info.ServerName.ToLower().Contains(FilterText))
		{
			continue; // Skip this row
		}

		// Create Widget
		if (UServerRowWidget* Row = CreateWidget<UServerRowWidget>(this, ServerRowClass))
		{
			Row->Setup(Info, Info.SearchResultIndex);
			ServerListScrollBox->AddChild(Row);
		}
	}
}

void UOnlineLobbyWidget::OnSliderValueChanged(float Value)
{
	if (MaxPlayersValueText)
	{
		MaxPlayersValueText->SetText(FText::AsNumber((int32)Value));
	}
}