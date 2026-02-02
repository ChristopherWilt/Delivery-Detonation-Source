// Fill out your copyright notice in the Description page of Project Settings.


#include "WaypointSystem/WaypointMarker.h"
#include "Components/StaticMeshComponent.h"

AWaypointMarker::AWaypointMarker()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create a mesh component (User should assign a Cylinder in BP)
	BeamMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BeamMesh"));
	SetRootComponent(BeamMesh);

	// Default settings to make it look like a beam
	BeamMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BeamMesh->SetCastShadow(false);
}

void AWaypointMarker::UpdateBeamVisuals(float Height, FLinearColor Color)
{
	// Scale Z to make it tall
	FVector CurrentScale = GetActorScale3D();
	SetActorScale3D(FVector(CurrentScale.X, CurrentScale.Y, Height));

	// Create a dynamic material to change color (assuming material has "Color" param)
	if (BeamMesh)
	{
		UMaterialInstanceDynamic* DynMat = BeamMesh->CreateAndSetMaterialInstanceDynamic(0);
		if (DynMat)
		{
			DynMat->SetVectorParameterValue(FName("Color"), Color);
		}
	}
}

