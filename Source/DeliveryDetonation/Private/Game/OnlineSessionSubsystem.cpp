#include "Game/OnlineSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"

UOnlineSessionSubsystem::UOnlineSessionSubsystem()
    : CreateSessionDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete))
    , FindSessionsDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete))
    , JoinSessionDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete))
{
    // Constructor
}

void UOnlineSessionSubsystem::LoginWithDeviceID()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(TEXT("EIK"));

    if (Subsystem)
    {
        IOnlineIdentityPtr Identity = Subsystem->GetIdentityInterface();
        if (Identity.IsValid())
        {
            if (Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
            {
                OnLoginCompleteEvent.Broadcast(true);
                return;
            }

            FOnLoginCompleteDelegate LoginDelegate;
            LoginDelegate.BindLambda([this](int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
                {
                    if (!bWasSuccessful)
                    {
                        UE_LOG(LogTemp, Error, TEXT("[EOS] Login Failed: %s"), *Error);
                    }
                    OnLoginCompleteEvent.Broadcast(bWasSuccessful);
                });

            Identity->AddOnLoginCompleteDelegate_Handle(0, LoginDelegate);

            // Check if AutoLogin started successfully. If not, fail immediately.
            if (!Identity->AutoLogin(0))
            {
                UE_LOG(LogTemp, Error, TEXT("[EOS] AutoLogin call failed immediately."));
                OnLoginCompleteEvent.Broadcast(false);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[EOS] Identity Interface Invalid"));
            OnLoginCompleteEvent.Broadcast(false);
        }
    }
    else
    {
        // Log if the subsystem is missing so we know why it's silent
        UE_LOG(LogTemp, Error, TEXT("[EOS] EIK Subsystem NOT FOUND! Check plugin enablement."));
        OnLoginCompleteEvent.Broadcast(false);
    }
}

void UOnlineSessionSubsystem::HostDeliverySession(FString ServerName, FString MapName, int32 MaxPlayers, bool bIsLAN)
{
    // 1. CHOOSE THE RIGHT SUBSYSTEM
    // If LAN, force "NULL". If Online, use Default (EIK).
    FName SubsystemName = bIsLAN ? FName("NULL") : FName("EIK");
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(SubsystemName);

    if (!Subsystem) return;

    SessionInterface = Subsystem->GetSessionInterface();
    if (SessionInterface.IsValid())
    {
        DesiredMapName = MapName;

        FOnlineSessionSettings SessionSettings;
        SessionSettings.bIsLANMatch = bIsLAN;
        SessionSettings.NumPublicConnections = MaxPlayers;

        SessionSettings.bUsesPresence = false;
        SessionSettings.bShouldAdvertise = true;

        // Standard Lobby settings
        SessionSettings.bAllowJoinInProgress = true;
        SessionSettings.bAllowJoinViaPresence = true;
        SessionSettings.bUseLobbiesIfAvailable = true;
        SessionSettings.Set(FName("SERVER_NAME"), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

        CreateSessionDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegate);

        if (!SessionInterface->CreateSession(0, FName("DeliverySession"), SessionSettings))
        {
            SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
            OnCreateSessionCompleteEvent.Broadcast(false);
        }
    }
}

void UOnlineSessionSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionDelegateHandle);
    }

    if (bWasSuccessful)
    {
        // START THE SESSION
       // SessionInterface->StartSession(SessionName);

        // REGISTER THE HOST
        // We need to grab the identity from the SAME subsystem we used to create the session
        // However, only EOS needs explicit registration usually, but it's safe to check.
        IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
        IOnlineIdentityPtr Identity = Subsystem ? Subsystem->GetIdentityInterface() : nullptr;

        if (Identity.IsValid())
        {
            FUniqueNetIdPtr NetId = Identity->GetUniquePlayerId(0);
            if (NetId.IsValid())
            {
                SessionInterface->RegisterPlayer(SessionName, *NetId, false);
            }
        }

        // SERVER TRAVEL
        UWorld* World = GetWorld();
        if (World)
        {
            FString Path = FString::Printf(TEXT("/Game/Maps/%s?listen"), *DesiredMapName);
            World->ServerTravel(Path);
        }

        // --- DEBUG BLOCK (Session ID + User ID) ---
        if (IOnlineSubsystem* DebugSubsystem = IOnlineSubsystem::Get())
        {
            // Get Session Interface to grab Lobby ID
            IOnlineSessionPtr DebugSessionInterface = DebugSubsystem->GetSessionInterface();
            if (DebugSessionInterface.IsValid())
            {
                FNamedOnlineSession* Session = DebugSessionInterface->GetNamedSession(SessionName);
                if (Session)
                {
                    FString SessionIdStr = Session->GetSessionIdStr();

                    // Log it
                    UE_LOG(LogTemp, Warning, TEXT("HOST SUCCESS! Lobby ID: %s"), *SessionIdStr);

                    // Print to Screen (Cyan Color)
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Cyan,
                            FString::Printf(TEXT("Lobby ID: %s"), *SessionIdStr));
                    }
                }
            }

            // Get Identity Interface to grab User ID
            IOnlineIdentityPtr DebugIdentityInterface = DebugSubsystem->GetIdentityInterface();
            if (DebugIdentityInterface.IsValid())
            {
                // Get the Unique ID of the local player (Controller Index 0)
                TSharedPtr<const FUniqueNetId> UserId = DebugIdentityInterface->GetUniquePlayerId(0);

                if (UserId.IsValid())
                {
                    FString UserIdStr = UserId->ToString();

                    // Log it
                    UE_LOG(LogTemp, Warning, TEXT("HOST USER ID: %s"), *UserIdStr);

                    // Print to Screen (Green Color)
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 60.f, FColor::Green,
                            FString::Printf(TEXT("My User ID: %s"), *UserIdStr));
                    }
                }
            }
        }
    }

    OnCreateSessionCompleteEvent.Broadcast(bWasSuccessful);
}

void UOnlineSessionSubsystem::FindSessions(bool bIsLAN)
{
    // 1. CHOOSE THE RIGHT SUBSYSTEM
    FName SubsystemName = bIsLAN ? FName("NULL") : FName("EIK");
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get(SubsystemName);

    if (!Subsystem) return;

    SessionInterface = Subsystem->GetSessionInterface();
    if (SessionInterface.IsValid())
    {
        SessionSearch = MakeShareable(new FOnlineSessionSearch());
        SessionSearch->bIsLanQuery = bIsLAN;
        SessionSearch->MaxSearchResults = 100;

        if (!bIsLAN)
        {
            //SessionSearch->QuerySettings.Set(FName("SEARCH_PRESENCE"), true, EOnlineComparisonOp::Equals);

            SessionSearch->QuerySettings.Set(FName("SEARCH_LOBBIES"), true, EOnlineComparisonOp::Equals);
			// Debug log
			UE_LOG(LogTemp, Warning, TEXT("Finding Online Sessions with Lobbies..."));
        }

        FindSessionsDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegate);

        SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
    }
}

void UOnlineSessionSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
    // --- START DEBUG BLOCK ---
    if (GEngine)
    {
        int32 Count = (SessionSearch.IsValid()) ? SessionSearch->SearchResults.Num() : 0;

        // Print total count (Green if found, Red if 0 or failed)
        FColor Color = (bWasSuccessful && Count > 0) ? FColor::Green : FColor::Red;
        FString Msg = FString::Printf(TEXT("[DEBUG] Search State: %s | Found: %d Lobbies"),
            bWasSuccessful ? TEXT("SUCCESS") : TEXT("FAILED"),
            Count);

        GEngine->AddOnScreenDebugMessage(-1, 20.0f, Color, Msg);
        UE_LOG(LogTemp, Warning, TEXT("======================================="));
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Msg);

        // Loop through results and print details
        if (bWasSuccessful && SessionSearch.IsValid())
        {
            for (int32 i = 0; i < SessionSearch->SearchResults.Num(); i++)
            {
                const auto& Result = SessionSearch->SearchResults[i];

                // Get critical debug info
                FString LobbyId = Result.GetSessionIdStr();
                FString OwnerName = Result.Session.OwningUserName;
                int32 OpenSlots = Result.Session.NumOpenPublicConnections;

                FString LobbyMsg = FString::Printf(TEXT("   [%d] Lobby ID: %s | Owner: %s | Open Slots: %d"),
                    i, *LobbyId, *OwnerName, OpenSlots);

                // Print Blue debug line for each lobby found
                GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, LobbyMsg);
                UE_LOG(LogTemp, Warning, TEXT("%s"), *LobbyMsg);
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("======================================="));
    }

    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsDelegateHandle);
    }

    if (bWasSuccessful && SessionSearch.IsValid())
    {
        TArray<FServerInfo> ServerList;

        IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
        IOnlineIdentityPtr Identity = Subsystem ? Subsystem->GetIdentityInterface() : nullptr;

        int32 Index = 0;
        for (const FOnlineSessionSearchResult& Result : SessionSearch->SearchResults)
        {
            bool bIsMyOwnSession = false;
            if (Identity.IsValid())
            {
                FUniqueNetIdPtr MyId = Identity->GetUniquePlayerId(0);
                if (MyId.IsValid() && Result.Session.OwningUserId == MyId)
                {
                    bIsMyOwnSession = true;
                }
            }

            if (!bIsMyOwnSession)
            {
                FServerInfo Info;
                FString ServerName = "Unknown";

                Result.Session.SessionSettings.Get(FName("SERVER_NAME"), ServerName);

                Info.ServerName = ServerName;
                Info.HostUsername = Result.Session.OwningUserName;
                Info.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
                Info.CurrentPlayers = Info.MaxPlayers - Result.Session.NumOpenPublicConnections;
                Info.Ping = Result.PingInMs;
                Info.SearchResultIndex = Index;

                ServerList.Add(Info);
            }
            Index++;
        }

        OnServerListUpdatedEvent.Broadcast(ServerList);
    }
}

void UOnlineSessionSubsystem::JoinFoundSession(int32 SearchResultIndex)
{
    if (!SessionInterface.IsValid() || !SessionSearch.IsValid()) return;

    if (SessionSearch->SearchResults.IsValidIndex(SearchResultIndex))
    {
        JoinSessionDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegate);
        const FOnlineSessionSearchResult& Result = SessionSearch->SearchResults[SearchResultIndex];
        SessionInterface->JoinSession(0, FName("DeliverySession"), Result);
    }
}

void UOnlineSessionSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (SessionInterface.IsValid())
    {
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionDelegateHandle);
    }

    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        FString ConnectInfo;
        if (SessionInterface->GetResolvedConnectString(SessionName, ConnectInfo))
        {
            if (APlayerController* PC = GetGameInstance()->GetFirstLocalPlayerController())
            {
                PC->ClientTravel(ConnectInfo, ETravelType::TRAVEL_Absolute);
            }
        }
    }
}

void UOnlineSessionSubsystem::DestroySession()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        IOnlineSessionPtr SessionInt = Subsystem->GetSessionInterface();
        if (SessionInt.IsValid())
        {
            // Clean up the named session so we can create a new one later
            SessionInt->DestroySession(FName("DeliverySession"));
        }
    }
}

void UOnlineSessionSubsystem::LeaveSession()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (!Subsystem) return;

    IOnlineSessionPtr SessionInt = Subsystem->GetSessionInterface();
    if (SessionInt.IsValid())
    {
        // For clients, DestroySession is effectively "leave"
        SessionInt->DestroySession(FName("DeliverySession"));
    }
}