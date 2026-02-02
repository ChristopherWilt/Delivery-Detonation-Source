// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/AudioManagerSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"

USoundBase* UAudioManagerSubsystem::GetSound(EDD_Sound SoundId) const
{
	if (const TObjectPtr<USoundBase>* Found = Sounds.Find(SoundId))
	{
		return Found->Get();
	}
	return nullptr;
}

void UAudioManagerSubsystem::PlayUI(EDD_Sound SoundId)
{
	if (bMuteAll) return;
	if (USoundBase* S = GetSound(SoundId))
	{
		UGameplayStatics::PlaySound2D(GetWorld(), S, UIVolume);
	}
}

void UAudioManagerSubsystem::PlaySFX2D(EDD_Sound SoundId)
{
	if (bMuteAll) return;
	if (USoundBase* S = GetSound(SoundId))
	{
		UGameplayStatics::PlaySound2D(GetWorld(), S, SFXVolume);
	}
}

void UAudioManagerSubsystem::PlaySFXAtLocation(EDD_Sound SoundId, FVector Location)
{
	if (bMuteAll) return;
	if (USoundBase* S = GetSound(SoundId))
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), S, Location, SFXVolume);
	}
}

void UAudioManagerSubsystem::PlayMusic(EDD_Sound SoundId)
{
	if (bMuteAll) return;

	USoundBase* S = GetSound(SoundId);
	if (!S) return;

	// Stop old music if any
	StopMusic();

	MusicComponent = UGameplayStatics::SpawnSound2D(GetWorld(), S, MusicVolume, 1.0f, 0.0f, nullptr, true);
	if (MusicComponent)
	{
		MusicComponent->bIsUISound = true;
	}
}

void UAudioManagerSubsystem::StopMusic()
{
	if (MusicComponent)
	{
		MusicComponent->Stop();
		MusicComponent = nullptr;
	}
}
