// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DeliveryDetonation_PlayerState.generated.h"

/**
 * 
 */
UCLASS()
class DELIVERYDETONATION_API ADeliveryDetonation_PlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
	ADeliveryDetonation_PlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "PostMatch")
	bool IsWinner() const { return bIsWinner; }

	UFUNCTION(BlueprintPure, Category = "PostMatch")
	bool IsReady() const { return bIsReady; }

	// GameMode calls these on the server
	void SetWinner(bool bNewWinner) { bIsWinner = bNewWinner; }
	void SetReady(bool bNewReady) { bIsReady = bNewReady; }

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PostMatch")
	bool bIsWinner = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "PostMatch")
	bool bIsReady = false;
};
