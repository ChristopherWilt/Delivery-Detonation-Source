#include "Pickups/BasePackage.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "ActorComponents/HealthComponent.h"
#include "Game/DeliveryDetonation_GameMode.h"
#include "Player/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Sound/SoundBase.h"
#include "NiagaraSystem.h"
#include "Game/PackageSpawnPoint.h" // <--- CRITICAL INCLUDE

ABasePackage::ABasePackage()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	Mesh->SetSimulatePhysics(true);
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	CollisionTrigger = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionTrigger"));
	CollisionTrigger->SetupAttachment(Mesh);

	CollisionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionTrigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionTrigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionTrigger->SetGenerateOverlapEvents(true);

	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
}

void ABasePackage::BeginPlay()
{
	Super::BeginPlay();

	if (!CargoTag.IsNone())
	{
		Tags.AddUnique(CargoTag);
	}

	if (CollisionTrigger)
	{
		CollisionTrigger->OnComponentBeginOverlap.AddDynamic(this, &ABasePackage::OnTriggerBegin);
		CollisionTrigger->OnComponentEndOverlap.AddDynamic(this, &ABasePackage::OnTriggerEnd);
	}

	if (Mesh)
	{
		Mesh->SetMassOverrideInKg(NAME_None, PackageWeightInKg, true);
	}

	if (ADeliveryDetonation_GameMode* GM = GetWorld()->GetAuthGameMode<ADeliveryDetonation_GameMode>())
	{
		GM->RegisterCargo(this);
	}

	if (HealthComp)
	{
		HealthComp->OnDeath.AddDynamic(this, &ABasePackage::HandlePackageDeath);
	}

	if (bIsCarriedVisual)
	{
		ApplyCarriedModeSettings();
	}
	else
	{
		ApplyWorldModeSettings();
	}
}

void ABasePackage::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// --- NEW FUNCTIONS FOR SPAWN SYSTEM ---
void ABasePackage::SetOwnerSpawnPoint(APackageSpawnPoint* Point)
{
	OwnerSpawnPoint = Point;
	if (Point)
	{
		Point->SetOccupied(true);
	}
}

void ABasePackage::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Release the spawn point when this package is destroyed (Picked Up)
	if (OwnerSpawnPoint.IsValid())
	{
		OwnerSpawnPoint->SetOccupied(false);
		OwnerSpawnPoint = nullptr;
	}
}
// --------------------------------------

void ABasePackage::SetAsCarriedVisual(bool bCarried)
{
	bIsCarriedVisual = bCarried;
	if (bIsCarriedVisual) ApplyCarriedModeSettings();
	else ApplyWorldModeSettings();
}

void ABasePackage::ApplyWorldModeSettings()
{
	if (Mesh)
	{
		Mesh->SetSimulatePhysics(true);
		Mesh->SetEnableGravity(true);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
		Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}

	if (CollisionTrigger)
	{
		CollisionTrigger->SetSphereRadius(TriggerRadius);
		CollisionTrigger->SetRelativeLocation(FVector::ZeroVector);
		CollisionTrigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionTrigger->SetGenerateOverlapEvents(true);
	}
}

void ABasePackage::ApplyCarriedModeSettings()
{
	if (Mesh)
	{
		Mesh->SetSimulatePhysics(true);
		Mesh->SetEnableGravity(true);
		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
		Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}

	if (CollisionTrigger)
	{
		CollisionTrigger->SetGenerateOverlapEvents(false);
		CollisionTrigger->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ABasePackage::Deliver()
{
	if (bDelivered || bFailed) return;
	bDelivered = true;

	// NOTE: Updated method signature to include Instigator (if known)
	// If unknown, GameMode handles it.
	if (ADeliveryDetonation_GameMode* GM = GetWorld()->GetAuthGameMode<ADeliveryDetonation_GameMode>())
	{
		// We pass 'GetInstigatorController()' if we have it, otherwise nullptr.
		GM->NotifyCargoDelivered(this, GetInstigatorController());
	}

	if (Mesh)
	{
		Mesh->SetSimulatePhysics(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	SetLifeSpan(0.2f);
}

void ABasePackage::Fail(FName Reason)
{
	if (bFailed || bDelivered) return;
	bFailed = true;

	if (ADeliveryDetonation_GameMode* GM = GetWorld()->GetAuthGameMode<ADeliveryDetonation_GameMode>())
	{
		GM->NotifyCargoFailed(this);
	}

	OnFailFX(Reason);
	SetLifeSpan(0.2f);
}

void ABasePackage::OnFailFX(FName Reason)
{
	if (Sfx_OnFail)
		UGameplayStatics::PlaySoundAtLocation(this, Sfx_OnFail, GetActorLocation());
	if (Vfx_OnFail)
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), Vfx_OnFail, GetActorLocation());
}

void ABasePackage::HandlePackageDeath()
{
	Fail("Destroyed");
}

void ABasePackage::OnTriggerBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bIsCarriedVisual || bDelivered || bFailed) return;

	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player) return;

	Player->SetOverlappingPackage(this);
}

void ABasePackage::OnTriggerEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (bIsCarriedVisual) return;

	APlayerCharacter* Player = Cast<APlayerCharacter>(OtherActor);
	if (!Player) return;

	Player->ClearOverlappingPackage(this);
}