#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Game/OnlineSessionSubsystem.h" 
#include "ServerRowWidget.generated.h"

class UTextBlock;
class UButton;

UCLASS()
class DELIVERYDETONATION_API UServerRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Called by the Lobby Widget to fill in data
	void Setup(const FServerInfo& InServerInfo, int32 InRowIndex);

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnJoinClicked();

	// --- BIND WIDGETS ---
	// In your WBP, name these exactly as written here:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ServerNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerCountText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PingText;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

private:
	int32 RowIndex;
};