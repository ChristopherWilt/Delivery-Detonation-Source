// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CoffeeShop.generated.h"

class USphereComponent;
class APlayerCharacter;

UCLASS()
class DELIVERYDETONATION_API ACoffeeShop : public AActor
{
	GENERATED_BODY()
	
public:
    ACoffeeShop();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere)
    USceneComponent* Root;

    UPROPERTY(VisibleAnywhere)
    USphereComponent* InteractTrigger;

    UPROPERTY(EditDefaultsOnly, Category = "Coffee")
    float CaffeineAmount = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "Coffee")
    int32 Cost = 0;

    UFUNCTION()
    void OnTriggerBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep);

    UFUNCTION()
    void OnTriggerEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    UFUNCTION(Server, Reliable)
    void Server_TryPurchase(APlayerCharacter* Player);
};
