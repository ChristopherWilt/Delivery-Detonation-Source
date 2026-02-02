// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacter.h"
#include "Animations/PlayerAnimation.h"
#include "ActorComponents/HealthComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"




// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	PhysicalAnimationComponent = CreateDefaultSubobject<UPhysicalAnimationComponent>(TEXT("PhysicalAnimationComponent"));

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();


	if (PhysicalAnimationComponent && GetMesh())
	{
		// Link the component to your Character's Mesh
		PhysicalAnimationComponent->SetSkeletalMeshComponent(GetMesh());

		// Define the settings (The "Flimsiness")
		FPhysicalAnimationData PhysData;
		PhysData.bIsLocalSimulation = true;
		PhysData.OrientationStrength = 500.0f; // Lower = more flimsy
		PhysData.AngularVelocityStrength = 100.0f;
		PhysData.PositionStrength = 0.0f;
		PhysData.VelocityStrength = 0.0f;

		PhysicalAnimationComponent->ApplyPhysicalAnimationSettingsBelow(FName("spine_03"), PhysData);
	}

	if (HealthComponent)
	{

		//HealthComponent->OnHealthChanged.AddDynamic(this, &ABaseCharacter::OnHurt);
		HealthComponent->OnDeath.AddDynamic(this, &ABaseCharacter::OnDeath);

		// DEBUG LOG 1: Confirm the delegate was bound
		//UE_LOG(Game, Warning, TEXT("BaseCharacter: Successfully bound to HealthComponent delegates."));
	}
	else
	{
		// DEBUG LOG: HealthComponent is missing
		//UE_LOG(Game, Error, TEXT("BaseCharacter: HealthComponent is NULL in BeginPlay!"));
	}
	
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HealthComponent->GetCurrentHealth() <= 0.0f)
	{
		OnDeath();
	}

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseCharacter::TriggerFullRagdoll()
{
	if (PhysicalAnimationComponent && GetMesh())
	{
		// 1. Completely remove the "muscle" strength
		FPhysicalAnimationData RagdollData;
		RagdollData.bIsLocalSimulation = false; // Let them tumble in world space
		RagdollData.OrientationStrength = 0.0f;
		RagdollData.PositionStrength = 0.0f;

		PhysicalAnimationComponent->ApplyPhysicalAnimationSettingsBelow(FName("pelvis"), RagdollData);

		// 2. Enable physics on the ENTIRE body
		GetMesh()->SetAllBodiesBelowSimulatePhysics(FName("pelvis"), true, false);

		// 3. Set the Mesh to a Ragdoll collision profile so it hits the floor correctly
		GetMesh()->SetCollisionProfileName(FName("Ragdoll"));

		if (CharacterAnim)
		{
			CharacterAnim->PhysicsAlpha = 1.0f; // Force max physics voice
			CharacterAnim->bIsDead = true; // Trigger the AnimGraph switch
		}
	}
}

void ABaseCharacter::OnHurt(float NewHealth)
{
	UPlayerAnimation* PlayerAnimInstance = Cast<UPlayerAnimation>(AnimInstance);
	if (PlayerAnimInstance)
	{
		//PlayerAnimInstance->HitAnimation(NewHealth);
	}
}

void ABaseCharacter::OnDeath()
{
	UPlayerAnimation* PlayerAnimInstance = Cast<UPlayerAnimation>(AnimInstance);
	if (PlayerAnimInstance)
	{
		PlayerAnimInstance->CurrentDeathAnimationAsset = nullptr;
		PlayerAnimInstance->SelectDeathAnimation();


	}
	SetActorEnableCollision(false);
}


