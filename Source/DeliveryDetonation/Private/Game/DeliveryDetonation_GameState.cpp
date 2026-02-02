// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/DeliveryDetonation_GameState.h"
#include "Net/UnrealNetwork.h"

ADeliveryDetonation_GameState::ADeliveryDetonation_GameState()
{
    // GameStates must tick to replicate frequently if needed
    PrimaryActorTick.bCanEverTick = true;
}

void ADeliveryDetonation_GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ADeliveryDetonation_GameState, TimeRemaining);
    DOREPLIFETIME(ADeliveryDetonation_GameState, TotalCargo);
    DOREPLIFETIME(ADeliveryDetonation_GameState, DeliveredCargo);

    DOREPLIFETIME(ADeliveryDetonation_GameState, MatchPhase);
    DOREPLIFETIME(ADeliveryDetonation_GameState, RestartCountdown);
}

void ADeliveryDetonation_GameState::OnRep_TimeRemaining()
{
    OnTimeUpdated.Broadcast(TimeRemaining);
}

void ADeliveryDetonation_GameState::OnRep_CargoStats()
{
    OnCargoUpdated.Broadcast(DeliveredCargo, TotalCargo);
}

void ADeliveryDetonation_GameState::OnRep_MatchPhase() 
{
    /* UI listens in BP if needed */ 
}

void ADeliveryDetonation_GameState::OnRep_PostMatchState() 
{
    /* UI reads PostMatchStates */ 
}

void ADeliveryDetonation_GameState::OnRep_RestartCountdown() 
{
    /* UI reads RestartCountdown */ 
}