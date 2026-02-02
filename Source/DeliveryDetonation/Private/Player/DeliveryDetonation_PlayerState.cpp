// Fill out your copyright notice in the Description page of Project Settings.
#include "Player/DeliveryDetonation_PlayerState.h"
#include "Net/UnrealNetwork.h"

ADeliveryDetonation_PlayerState::ADeliveryDetonation_PlayerState()
{
}

void ADeliveryDetonation_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADeliveryDetonation_PlayerState, bIsWinner);
	DOREPLIFETIME(ADeliveryDetonation_PlayerState, bIsReady);
}