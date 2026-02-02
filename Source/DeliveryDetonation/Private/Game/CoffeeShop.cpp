// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/CoffeeShop.h"
#include "Components/SphereComponent.h"
#include "Player/PlayerCharacter.h"
#include "ActorComponents/CaffeineComponent.h"

ACoffeeShop::ACoffeeShop()
{
    bReplicates = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    InteractTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("InteractTrigger"));
    InteractTrigger->SetupAttachment(Root);
    InteractTrigger->InitSphereRadius(180.f);
    InteractTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    InteractTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
    InteractTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    InteractTrigger->SetGenerateOverlapEvents(true);

    InteractTrigger->OnComponentBeginOverlap.AddDynamic(this, &ACoffeeShop::OnTriggerBegin);
    InteractTrigger->OnComponentEndOverlap.AddDynamic(this, &ACoffeeShop::OnTriggerEnd);
}

void ACoffeeShop::BeginPlay()
{
    Super::BeginPlay();
}

void ACoffeeShop::OnTriggerBegin(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
    {
        Player->SetOverlappingInteractable(this);
    }
}

void ACoffeeShop::OnTriggerEnd(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32)
{
    if (APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor))
    {
        Player->ClearOverlappingInteractable(this);
    }
}

void ACoffeeShop::Server_TryPurchase_Implementation(APlayerCharacter* Player)
{
    if (!Player) return;

    // TODO: check currency here (coins, score, etc). For now just grant caffeine.
    if (UCaffeineComponent* Caf = Player->FindComponentByClass<UCaffeineComponent>())
    {
        Caf->AddCaffeine(CaffeineAmount);
    }
}

