// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "AudioSettingsSave.generated.h"

/**
 * 
 */
UCLASS()
class DELIVERYDETONATION_API UAudioSettingsSave : public USaveGame
{
	GENERATED_BODY()
	
public:
	// 0..1 range
	UPROPERTY(BlueprintReadWrite)
	float MasterVolume = 1.0f;

};
