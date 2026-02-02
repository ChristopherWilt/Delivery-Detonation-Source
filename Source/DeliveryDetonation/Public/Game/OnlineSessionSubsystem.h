#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSessionSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FServerInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ServerName = "Unknown";

    UPROPERTY(BlueprintReadOnly)
    FString HostUsername = "Unknown";

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentPlayers = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 MaxPlayers = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 Ping = 999; // <--- (Default to high ping)

    UPROPERTY(BlueprintReadOnly)
    int32 SearchResultIndex = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnServerListUpdated, const TArray<FServerInfo>&, ServerList);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSessionActionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSubsystemLoginResult, bool, bWasSuccessful);

UCLASS()
class DELIVERYDETONATION_API UOnlineSessionSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UOnlineSessionSubsystem();

    UFUNCTION(BlueprintCallable, Category = "Session")
    void LoginWithDeviceID();

    UFUNCTION(BlueprintCallable, Category = "Session")
    void HostDeliverySession(FString ServerName, FString MapName, int32 MaxPlayers, bool bIsLAN);

    UFUNCTION(BlueprintCallable, Category = "Session")
    void FindSessions(bool bIsLAN);

    UFUNCTION(BlueprintCallable, Category = "Session")
    void JoinFoundSession(int32 SearchResultIndex);

    UFUNCTION(BlueprintCallable, Category = "Session")
    void DestroySession();

    UFUNCTION(BlueprintCallable, Category = "Session")
    void LeaveSession();

    UPROPERTY(BlueprintAssignable)
    FOnSessionActionComplete OnCreateSessionCompleteEvent;

    UPROPERTY(BlueprintAssignable)
    FOnServerListUpdated OnServerListUpdatedEvent;

    UPROPERTY(BlueprintAssignable)
    FSubsystemLoginResult OnLoginCompleteEvent;

protected:
    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:
    IOnlineSessionPtr SessionInterface;
    TSharedPtr<FOnlineSessionSearch> SessionSearch;
    FString DesiredMapName;

    // --- THESE WERE MISSING ---
    FOnCreateSessionCompleteDelegate CreateSessionDelegate;
    FOnFindSessionsCompleteDelegate FindSessionsDelegate;
    FOnJoinSessionCompleteDelegate JoinSessionDelegate;
    // --------------------------

    FDelegateHandle CreateSessionDelegateHandle;
    FDelegateHandle FindSessionsDelegateHandle;
    FDelegateHandle JoinSessionDelegateHandle;
};