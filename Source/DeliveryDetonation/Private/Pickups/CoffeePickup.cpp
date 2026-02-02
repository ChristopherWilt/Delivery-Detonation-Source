// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/CoffeePickup.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/PlayerCharacter.h"
#include "ActorComponents/CaffeineComponent.h"

ACoffeePickup::ACoffeePickup()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Trigger = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Trigger"));
	SetRootComponent(Trigger);
	Trigger->InitCapsuleSize(80.f, 120.f);
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	Cup = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CupMesh"));
	Cup->SetupAttachment(Trigger);
	Cup->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Lid = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LidMesh"));
	Lid->SetupAttachment(Cup);
	Lid->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ACoffeePickup::OnOverlapBegin);
}

void ACoffeePickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player) return;

	if (UCaffeineComponent* Caf = Player->FindComponentByClass<UCaffeineComponent>())
	{
		Caf->AddCaffeine(CaffeineAmount);

		if (PickupSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
		}

		Destroy();
	}
}