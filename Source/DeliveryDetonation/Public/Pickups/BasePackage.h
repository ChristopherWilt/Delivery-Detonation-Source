// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/PackageSpawnPoint.h"
#include "BasePackage.generated.h"

class UStaticMeshComponent;
class UHealthComponent;
class USceneComponent;
class USphereComponent;
class APlayerCharacter;

UCLASS()
class DELIVERYDETONATION_API ABasePackage : public AActor
{
	GENERATED_BODY()

public:
	ABasePackage();

	virtual void Tick(float DeltaTime) override;

	// --- STATE ---
	UFUNCTION(BlueprintPure, Category = "Package")
	bool IsDelivered() const { return bDelivered; }

	UFUNCTION(BlueprintPure, Category = "Package")
	bool IsFailed() const { return bFailed; }

	UFUNCTION(BlueprintPure, Category = "Package")
	bool IsCarriedVisual() const { return bIsCarriedVisual; }

	// Called by DeliveryZone / GameMode when delivered
	UFUNCTION(BlueprintCallable, Category = "Package")
	virtual void Deliver();

	// Called when package is destroyed / exploded / failed
	UFUNCTION(BlueprintCallable, Category = "Package")
	virtual void Fail(FName Reason = "Unknown");

	// Optional speed impact
	UFUNCTION(BlueprintPure, Category = "Package")
	float GetCarrySpeedMultiplier() const { return CarrySpeedMultiplier; }

	// Optional movement impacts while carrying
	UFUNCTION(BlueprintPure, Category = "Package|Carry")
	virtual float GetCarryJumpMultiplier() const { return 1.0f; }

	UFUNCTION(BlueprintPure, Category = "Package|Carry")
	virtual float GetSlideFrictionMultiplier() const { return 1.0f; }

	// Switch modes at runtime (optional)
	UFUNCTION(BlueprintCallable, Category = "Package")
	void SetAsCarriedVisual(bool bCarried);

	UFUNCTION()
	float GetPackageWeight() { return this->PackageWeightInKg; }

	USphereComponent* GetCollisionTrigger() const { return CollisionTrigger; }

	// ---------- NEW: pairing (world <-> carried) ----------
	UFUNCTION(BlueprintPure, Category = "Package|Pairing")
	TSubclassOf<ABasePackage> GetCarriedVisualClass() const { return CarriedVisualClass; }

	UFUNCTION(BlueprintPure, Category = "Package|Pairing")
	TSubclassOf<ABasePackage> GetWorldDropClass() const { return WorldDropClass; }

	void SetOwnerSpawnPoint(APackageSpawnPoint* Point);

protected:
	virtual void BeginPlay() override;

	// --- COMPONENTS ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollisionTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UHealthComponent* HealthComp;

	// --- CONFIG ---
	UPROPERTY(EditDefaultsOnly, Category = "Package")
	FName CargoTag = "Cargo";

	UPROPERTY(EditDefaultsOnly, Category = "Package|Carry")
	float CarryJumpMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Package|Carry")
	float SlideFrictionMultiplier = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	float PackageWeightInKg;

	// If TRUE: this actor is the "carried visual" version (no trigger, no physics)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Package|Mode")
	bool bIsCarriedVisual = false;

	// Trigger radius for the world version
	UPROPERTY(EditDefaultsOnly, Category = "Package|Trigger")
	float TriggerRadius = 120.f;

	// Optional movement slowdown while carrying
	UPROPERTY(EditDefaultsOnly, Category = "Package|Carry")
	float CarrySpeedMultiplier = 1.0f;

	// ---------- NEW: pairing data ----------
	// Set this on the WORLD package BP (what it spawns when picked up)
	UPROPERTY(EditDefaultsOnly, Category = "Package|Pairing")
	TSubclassOf<ABasePackage> CarriedVisualClass;

	// Set this on the CARRIED visual BP (what it spawns when dropped)
	UPROPERTY(EditDefaultsOnly, Category = "Package|Pairing")
	TSubclassOf<ABasePackage> WorldDropClass;

	// --- INTERNAL ---
	UPROPERTY(VisibleAnywhere, Category = "Package")
	bool bDelivered = false;

	UPROPERTY(VisibleAnywhere, Category = "Package")
	bool bFailed = false;

	UFUNCTION()
	void HandlePackageDeath();

	UFUNCTION()
	void OnTriggerBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnTriggerEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// Optional generic FX for any package type
	UPROPERTY(EditDefaultsOnly, Category = "Package|FX")
	class USoundBase* Sfx_OnFail = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Package|FX")
	class UNiagaraSystem* Vfx_OnFail = nullptr;

	// Called exactly once when Fail() is triggered (explosive package will override)
	virtual void OnFailFX(FName Reason);

	// Reference to the point that spawned us
	UPROPERTY()
	TWeakObjectPtr<APackageSpawnPoint> OwnerSpawnPoint;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


private:
	void ApplyWorldModeSettings();
	void ApplyCarriedModeSettings();
};