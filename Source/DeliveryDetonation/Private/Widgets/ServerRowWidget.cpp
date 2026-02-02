#include "Widgets/ServerRowWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Game/OnlineSessionSubsystem.h"

void UServerRowWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UServerRowWidget::OnJoinClicked);
	}
}

void UServerRowWidget::Setup(const FServerInfo& InServerInfo, int32 InRowIndex)
{
	RowIndex = InRowIndex;

	if (ServerNameText)
	{
		ServerNameText->SetText(FText::FromString(InServerInfo.ServerName));
	}

	if (PingText)
	{
		PingText->SetText(FText::AsNumber(InServerInfo.Ping));
	}

	if (PlayerCountText)
	{
		FString CountStr = FString::Printf(TEXT("%d/%d"), InServerInfo.CurrentPlayers, InServerInfo.MaxPlayers);
		PlayerCountText->SetText(FText::FromString(CountStr));
	}
}

void UServerRowWidget::OnJoinClicked()
{
	if (UOnlineSessionSubsystem* OSS = GetGameInstance()->GetSubsystem<UOnlineSessionSubsystem>())
	{
		// Join the session at this specific index
		OSS->JoinFoundSession(RowIndex);
	}
}