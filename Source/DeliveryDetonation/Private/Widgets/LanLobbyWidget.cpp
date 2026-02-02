#include "Widgets/LanLobbyWidget.h"
#include "Widgets/ServerRowWidget.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/WidgetSwitcher.h"

void ULanLobbyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind Navigation
	if (HostTabButton) HostTabButton->OnClicked.AddDynamic(this, &ULanLobbyWidget::OnHostTabClicked);
	if (JoinTabButton) JoinTabButton->OnClicked.AddDynamic(this, &ULanLobbyWidget::OnJoinTabClicked);
	if (BackButton) BackButton->OnClicked.AddDynamic(this, &ULanLobbyWidget::OnBackClicked);

	// Bind Actions
	if (HostGameButton) HostGameButton->OnClicked.AddDynamic(this, &ULanLobbyWidget::OnStartHostClicked);
	if (RefreshButton) RefreshButton->OnClicked.AddDynamic(this, &ULanLobbyWidget::OnRefreshClicked);

	if (MaxPlayersSlider)
	{
		MaxPlayersSlider->OnValueChanged.AddDynamic(this, &ULanLobbyWidget::OnSliderValueChanged);
		OnSliderValueChanged(MaxPlayersSlider->GetValue());
	}

	// Subscribe to Subsystem Updates
	if (UOnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UOnlineSessionSubsystem>())
	{
		OSS->OnServerListUpdatedEvent.AddDynamic(this, &ULanLobbyWidget::UpdateServerList);
	}
}

void ULanLobbyWidget::NativeDestruct()
{
	if (UOnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UOnlineSessionSubsystem>())
	{
		OSS->OnServerListUpdatedEvent.RemoveDynamic(this, &ULanLobbyWidget::UpdateServerList);
	}
	Super::NativeDestruct();
}

void ULanLobbyWidget::OnHostTabClicked()
{
	if (MenuSwitcher) MenuSwitcher->SetActiveWidgetIndex(0);
}

void ULanLobbyWidget::OnJoinTabClicked()
{
	if (MenuSwitcher) MenuSwitcher->SetActiveWidgetIndex(1);
	OnRefreshClicked(); // Auto refresh
}

void ULanLobbyWidget::OnBackClicked()
{
	RemoveFromParent();
}

void ULanLobbyWidget::OnStartHostClicked()
{
	if (UOnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UOnlineSessionSubsystem>())
	{
		FString ServerName = ServerNameInput ? ServerNameInput->GetText().ToString() : "My Server";
		if (ServerName.IsEmpty()) ServerName = "LAN Game";

		int32 MaxPlayers = MaxPlayersSlider ? (int32)MaxPlayersSlider->GetValue() : 4;

		// HOST LAN SESSION (bIsLAN = true)
		OSS->HostDeliverySession(ServerName, SelectedMapName.ToString(), MaxPlayers, true);
	}
}

void ULanLobbyWidget::OnRefreshClicked()
{
	if (ServerListScrollBox) ServerListScrollBox->ClearChildren();

	if (UOnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UOnlineSessionSubsystem>())
	{
		// FIND LAN SESSIONS (bIsLAN = true)
		OSS->FindSessions(true);
	}
}

void ULanLobbyWidget::UpdateServerList(const TArray<FServerInfo>& ServerInfos)
{
	if (!ServerListScrollBox || !ServerRowClass) return;

	ServerListScrollBox->ClearChildren();

	for (const FServerInfo& Info : ServerInfos)
	{
		// Create the Widget
		if (UServerRowWidget* Row = CreateWidget<UServerRowWidget>(this, ServerRowClass))
		{
			Row->Setup(Info, Info.SearchResultIndex);
			ServerListScrollBox->AddChild(Row);
		}
	}
}

void ULanLobbyWidget::OnSliderValueChanged(float Value)
{
	if (MaxPlayersValueText)
	{
		MaxPlayersValueText->SetText(FText::AsNumber((int32)Value));
	}
}