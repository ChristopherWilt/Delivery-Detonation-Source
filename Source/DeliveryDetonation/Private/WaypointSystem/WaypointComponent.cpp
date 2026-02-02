// Fill out your copyright notice in the Description page of Project Settings.


#include "WaypointSystem/WaypointComponent.h"
#include "WaypointSystem/WaypointMarker.h"
#include "Net/UnrealNetwork.h"

UWaypointComponent::UWaypointComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UWaypointComponent::SetObjectiveLocation(FVector NewLocation)
{
	// Logic runs on Server, sends to Owner Clientt
	Client_UpdateObjective(NewLocation);
}

void UWaypointComponent::Client_UpdateObjective_Implementation(FVector NewLocation)
{
	// 1. Spawn Marker if it doesn't exist
	if (!CurrentMarker && WaypointMarkerClass)
	{
		FActorSpawnParameters Params;
		Params.Owner = GetOwner();
		CurrentMarker = GetWorld()->SpawnActor<AWaypointMarker>(WaypointMarkerClass, NewLocation, FRotator::ZeroRotator, Params);

		if (CurrentMarker)
		{
			CurrentMarker->UpdateBeamVisuals(BeamHeightScale, FLinearColor::Green);
		}
	}

	// 2. Move Marker
	if (CurrentMarker)
	{
		CurrentMarker->SetActorLocation(NewLocation);
	}

	// 3. Notify UI
	OnWaypointUpdated.Broadcast(NewLocation);
}

void UWaypointComponent::ClearObjective()
{
	// Server tells Client to destroy it
	Client_ClearObjective();
}

void UWaypointComponent::Client_ClearObjective_Implementation()
{
	if (CurrentMarker)
	{
		CurrentMarker->Destroy();
		CurrentMarker = nullptr;
	}
}

