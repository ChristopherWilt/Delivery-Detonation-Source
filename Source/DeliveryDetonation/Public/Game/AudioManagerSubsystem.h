// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AudioManagerSubsystem.generated.h"


class USoundBase;

UENUM(BlueprintType)
enum class EDD_Sound : uint8
{
	UI_Click,
	UI_Hover,
	SFX_Pickup,
	SFX_Drop,
	SFX_Explode,
	Music_Menu,
	Music_Game
};
/**
 * 
 */
UCLASS()
class DELIVERYDETONATION_API UAudioManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// Volumes (easy to expose to settings menu later)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float UIVolume = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float SFXVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MusicVolume = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	bool bMuteAll = false;

	// Sound library
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
	TMap<EDD_Sound, TObjectPtr<USoundBase>> Sounds;

	// Play helpers
	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayUI(EDD_Sound SoundId);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySFX2D(EDD_Sound SoundId);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlaySFXAtLocation(EDD_Sound SoundId, FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayMusic(EDD_Sound SoundId);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void StopMusic();

private:
	UPROPERTY()
	TObjectPtr<class UAudioComponent> MusicComponent = nullptr;

	USoundBase* GetSound(EDD_Sound SoundId) const;
};
