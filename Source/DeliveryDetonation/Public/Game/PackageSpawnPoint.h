#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PackageSpawnPoint.generated.h"

class UBillboardComponent;
class UArrowComponent;

UCLASS()
class DELIVERYDETONATION_API APackageSpawnPoint : public AActor
{
    GENERATED_BODY()

public:
    APackageSpawnPoint();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBillboardComponent* Sprite;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UArrowComponent* Arrow;

    // Is a package currently sitting here?
    UPROPERTY(VisibleInstanceOnly, Category = "State")
    bool bIsOccupied = false;

public:
    UFUNCTION(BlueprintCallable, Category = "Spawn")
    bool IsObstructed() const;

    UFUNCTION(BlueprintPure, Category = "Spawn")
    bool IsOccupied() const { return bIsOccupied; }

    void SetOccupied(bool bOccupied);
};