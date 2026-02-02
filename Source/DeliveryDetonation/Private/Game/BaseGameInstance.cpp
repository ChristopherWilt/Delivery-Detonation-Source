// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/BaseGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Game/OnlineSessionSubsystem.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Sound/SoundMix.h"
#include "Sound/SoundClass.h"
#include "Game/AudioSettingsSave.h"

UBaseGameInstance::UBaseGameInstance()
{
    CurrentLevelName = FirstLevelMapName;
}

// NEW: Helper to check if we are logged into EOS
bool UBaseGameInstance::IsPlayerLoggedIn() const
{
    // FIX: Change "EOS" to "EIK"
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("EIK"));
    if (OSS)
    {
        auto Identity = OSS->GetIdentityInterface();
        if (Identity.IsValid())
        {
            return Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn;
        }
    }
    return false;
}

// UPDATED: Handles both LAN and Online hosting paths
void UBaseGameInstance::LoadFirstLevel(bool bIsLAN)
{
    UOnlineSessionSubsystem* SessionSubsystem = GetSubsystem<UOnlineSessionSubsystem>();
    if (!SessionSubsystem) return;

    if (bIsLAN)
    {
        SessionSubsystem->HostDeliverySession("LAN_Host", FirstLevelMapName.ToString(), 4, true);
        return;
    }

    // FIX: Change "EOS" to "EIK"
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get(TEXT("EIK"));
    if (OSS)
    {
        auto Identity = OSS->GetIdentityInterface();

        if (Identity.IsValid() && Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn)
        {
            SessionSubsystem->HostDeliverySession("Online_Host", FirstLevelMapName.ToString(), 4, false);
            return;
        }
    }

    SessionSubsystem->OnLoginCompleteEvent.AddDynamic(this, &UBaseGameInstance::HandleLoginBeforeHost);
    SessionSubsystem->LoginWithDeviceID();
}

void UBaseGameInstance::HandleLoginBeforeHost(bool bWasSuccessful)
{
    if (UOnlineSessionSubsystem* SessionSubsystem = GetSubsystem<UOnlineSessionSubsystem>())
    {
        SessionSubsystem->OnLoginCompleteEvent.RemoveDynamic(this, &UBaseGameInstance::HandleLoginBeforeHost);

        if (bWasSuccessful)
        {
            // Login Success -> Host Online Session
            // Note: passing 'false' for bIsLAN
            SessionSubsystem->HostDeliverySession("Online_Host", FirstLevelMapName.ToString(), 4, false);
        }
        else
        {
            // Login Failed -> We cannot host online. 
            // Fallback to offline map or show error
            UE_LOG(LogTemp, Error, TEXT("[GameInstance] Failed to login to EOS. Cannot host online."));
            // Optional: fallback to local play
            // UGameplayStatics::OpenLevel(this, FirstLevelMapName); 
        }
    }
}

void UBaseGameInstance::QuitTheGame()
{
    if (UOnlineSessionSubsystem* OSS = GetSubsystem<UOnlineSessionSubsystem>())
    {
        // If we are the host, destroy. If client, leave. Hard to know here, so safest:
        OSS->DestroySession();
    }

    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PC->ConsoleCommand(TEXT("quit"));
        }
    }
}

void UBaseGameInstance::LoadMenuLevel()
{
    LoadLevelSafe(MainMenuMapName);
}

void UBaseGameInstance::LoadLevelSafe(FName LevelName)
{
    if (!LevelName.IsNone())
    {
        if (APlayerController* PC = GetFirstLocalPlayerController())
        {
            // This is the actual command that changes the level
            PC->ClientTravel(LevelName.ToString(), ETravelType::TRAVEL_Absolute);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("LoadLevelSafe: Attempted to load an empty Level Name!"));
    }
}

void UBaseGameInstance::SetMasterVolume(float NewValue)
{
    MasterVolume = FMath::Clamp(NewValue, 0.0f, 1.0f);

    ApplyAudioSettings();
    SaveAudioSettings();
}

void UBaseGameInstance::ApplyAudioSettings()
{
    if (!MasterMix || !MasterClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[GameInstance] MasterMix or MasterClass not set."));
        return;
    }

    UGameplayStatics::SetSoundMixClassOverride(
        this,
        MasterMix,
        MasterClass,
        MasterVolume,
        1.0f,
        0.0f,
        true
    );

    UGameplayStatics::PushSoundMixModifier(this, MasterMix);
}

void UBaseGameInstance::LoadAudioSettings()
{
    if (!UGameplayStatics::DoesSaveGameExist(AudioSaveSlot, AudioSaveUserIndex))
    {
        MasterVolume = 1.0f;
        return;
    }

    if (USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(AudioSaveSlot, AudioSaveUserIndex))
    {
        if (UAudioSettingsSave* Save = Cast<UAudioSettingsSave>(Loaded))
        {
            MasterVolume = FMath::Clamp(Save->MasterVolume, 0.0f, 1.0f);
        }
    }
}

void UBaseGameInstance::SaveAudioSettings()
{
    UAudioSettingsSave* SaveObj = nullptr;

    if (UGameplayStatics::DoesSaveGameExist(AudioSaveSlot, AudioSaveUserIndex))
    {
        if (USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(AudioSaveSlot, AudioSaveUserIndex))
        {
            SaveObj = Cast<UAudioSettingsSave>(Loaded);
        }
    }

    if (!SaveObj)
    {
        SaveObj = Cast<UAudioSettingsSave>(UGameplayStatics::CreateSaveGameObject(UAudioSettingsSave::StaticClass()));
    }

    if (!SaveObj) return;

    SaveObj->MasterVolume = MasterVolume;
    UGameplayStatics::SaveGameToSlot(SaveObj, AudioSaveSlot, AudioSaveUserIndex);
}

void UBaseGameInstance::Init()
{
    Super::Init();

    LoadAudioSettings();
    ApplyAudioSettings();
}