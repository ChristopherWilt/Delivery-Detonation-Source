// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoffeePickup.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;

UCLASS()
class DELIVERYDETONATION_API ACoffeePickup : public AActor
{
	GENERATED_BODY()
	
public:
	ACoffeePickup();

protected:
	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* Trigger;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Cup;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Lid;

	UPROPERTY(EditAnywhere, Category = "Caffeine")
	float CaffeineAmount = 25.f;

	UPROPERTY(EditAnywhere, Category = "Audio")
	class USoundBase* PickupSound = nullptr;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
