#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WaypointComponent.generated.h"

class AWaypointMarker;

// Delegate for UI
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaypointUpdatedSignature, FVector, TargetLocation);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DELIVERYDETONATION_API UWaypointComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWaypointComponent();

	// --- SERVER API ---
	// GameMode calls this to set a new target
	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	void SetObjectiveLocation(FVector NewLocation);

	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	void ClearObjective();

	// --- CONFIGURATION ---
	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
	TSubclassOf<AWaypointMarker> WaypointMarkerClass;

	UPROPERTY(EditDefaultsOnly, Category = "Visuals")
	float BeamHeightScale = 50.0f; // Multiplier for Z scale

	// --- UI EVENTS ---
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWaypointUpdatedSignature OnWaypointUpdated;

protected:
	// The Client-Side RPC
	UFUNCTION(Client, Reliable)
	void Client_UpdateObjective(FVector NewLocation);

	UFUNCTION(Client, Reliable)
	void Client_ClearObjective();

	// The actual marker actor instance
	UPROPERTY()
	AWaypointMarker* CurrentMarker;
};

