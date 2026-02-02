#include "Game/DeliveryZone.h"
#include "Components/BoxComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Player/PlayerCharacter.h"
#include "Game/DeliveryDetonation_GameMode.h"
#include "Kismet/GameplayStatics.h"

ADeliveryZone::ADeliveryZone()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true; // Just in case, though usually static

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(Root);
	TriggerBox->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	TriggerBox->SetCollisionProfileName(TEXT("Trigger"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Root);
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));

	Sprite = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	Sprite->SetupAttachment(Root);
}

void ADeliveryZone::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ADeliveryZone::OnOverlapBegin);
	}
}

void ADeliveryZone::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (Player && Player->IsCarrying())
	{
		// Ask GameMode to verify and complete the transaction
		if (ADeliveryDetonation_GameMode* GM = GetWorld()->GetAuthGameMode<ADeliveryDetonation_GameMode>())
		{
			GM->TryCompleteDelivery(Player, this);
		}
	}
}