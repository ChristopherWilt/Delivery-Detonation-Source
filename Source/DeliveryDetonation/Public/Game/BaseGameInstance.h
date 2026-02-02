// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Misc/Optional.h"
#include "BaseGameInstance.generated.h"

class USoundMix;
class USoundClass;

UCLASS()
class DELIVERYDETONATION_API UBaseGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UBaseGameInstance();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Levels")
	TArray<FName> GameLevels;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maps")
	FName MainMenuMapName = "MainMenuMap";

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Maps")
	FName FirstLevelMapName = "Level1";

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Levels")
	FName CurrentLevelName;

	// UPDATED: Now accepts bIsLAN
	UFUNCTION(BlueprintCallable, Category = "Levels")
	void LoadFirstLevel(bool bIsLAN);

	// NEW: Helper for UI to check if we can skip login
	UFUNCTION(BlueprintPure, Category = "Online")
	bool IsPlayerLoggedIn() const;

	UFUNCTION(BlueprintCallable, Category = "System")
	void QuitTheGame();

	UFUNCTION(BlueprintCallable)
	void LoadMenuLevel();

	virtual void Init() override;

	// --- Audio API ---
	float GetMasterVolume() const { return MasterVolume; }
	void SetMasterVolume(float NewValue);

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundMix* MasterMix = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	USoundClass* MasterClass = nullptr;

protected:
	UFUNCTION()
	void LoadLevelSafe(FName LevelName);

	UFUNCTION()
	void HandleLoginBeforeHost(bool bWasSuccessful);

private:
	// 0..1
	UPROPERTY()
	float MasterVolume = 1.0f;

	// Save helpers
	void LoadAudioSettings();
	void SaveAudioSettings();
	void ApplyAudioSettings();

	// Slot
	static constexpr const TCHAR* AudioSaveSlot = TEXT("AudioSettings");
	static constexpr int32 AudioSaveUserIndex = 0;
};