#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaypointMarker.generated.h"

UCLASS()
class DELIVERYDETONATION_API AWaypointMarker : public AActor
{
	GENERATED_BODY()

public:
	AWaypointMarker();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* BeamMesh;

public:
	// Call this to update the beam's look
	UFUNCTION(BlueprintCallable, Category = "Waypoint")
	void UpdateBeamVisuals(float Height, FLinearColor Color);
};