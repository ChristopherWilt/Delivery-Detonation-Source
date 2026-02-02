#include "Game/PackageSpawnPoint.h"
#include "Components/BillboardComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/World.h"

APackageSpawnPoint::APackageSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;

    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    Sprite = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
    Sprite->SetupAttachment(Root);

    Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
    Arrow->SetupAttachment(Root);
}

bool APackageSpawnPoint::IsObstructed() const
{
    FHitResult Hit;
    FVector Start = GetActorLocation() + FVector(0, 0, 20);
    FVector End = GetActorLocation() + FVector(0, 0, 100);

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);
}

void APackageSpawnPoint::SetOccupied(bool bOccupied)
{
    bIsOccupied = bOccupied;
    // Optional: Toggle sprite visibility for debug
     if (Sprite) Sprite->SetVisibility(!bIsOccupied);
}