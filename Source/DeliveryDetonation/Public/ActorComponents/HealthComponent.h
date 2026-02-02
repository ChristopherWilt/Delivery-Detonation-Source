// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, Current, float, Max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHurt, float, HealthRatio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeal, float, HealthRatio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DELIVERYDETONATION_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UHealthComponent();

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHurt OnHurt;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHeal OnHeal;

	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;

	float GetCurrentHealth() const { return CurrentHealth; }
	float GetMaxHealth() const { return MaxHealth; }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// REPLICATION OVERRIDE
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	float MaxHealth = 5.0f;

private:

	UPROPERTY(VisibleAnywhere, Category = "Health", ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth = 0.0f;

	UFUNCTION()
	void OnRep_CurrentHealth();

public:

	UFUNCTION()
	void HandleDamage(AActor* DamagedActor, float Damage,
		const class UDamageType* DamageType,
		class AController* InstigatedBy, AActor* DamageCauser);
		
};
