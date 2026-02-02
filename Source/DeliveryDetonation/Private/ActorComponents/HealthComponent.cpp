// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/HealthComponent.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true); // Enable component replication
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
}

// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleDamage);
	}
}

void UHealthComponent::OnRep_CurrentHealth()
{
	// This function runs on Clients when CurrentHealth updates.
	// We broadcast so the UI can update.
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		OnDeath.Broadcast();
	}
}

void UHealthComponent::HandleDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage == 0.0f)
		return;

	if (Damage > 0.0f && CurrentHealth <= 0.0f)
		return;

	const float OldHealth = CurrentHealth;

	if (Damage > 0.0f)
	{
		CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
	}
	else
	{
		const float HealAmount = -Damage;
		CurrentHealth = FMath::Clamp(CurrentHealth + HealAmount, 0.0f, MaxHealth);
	}

	OnRep_CurrentHealth();

	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	const float Ratio = (MaxHealth > 0.f) ? (CurrentHealth / MaxHealth) : 0.f;

	if (Damage > 0.0f)
	{
		if (CurrentHealth <= 0.0f)
		{
			if (AActor* Owner = GetOwner())
			{
				Owner->OnTakeAnyDamage.RemoveDynamic(this, &UHealthComponent::HandleDamage);
			}
			OnDeath.Broadcast();
		}
		else
		{
			OnHurt.Broadcast(Ratio);
		}
	}
	else
	{
		OnHeal.Broadcast(Ratio);
	}
}
