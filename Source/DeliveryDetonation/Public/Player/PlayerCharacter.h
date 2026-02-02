// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/BaseCharacter.h"
#include "WaypointSystem/WaypointComponent.h"
#include "InputActionValue.h"
#include "PlayerCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UHUDWidget;
class UCaffeineComponent;
class ABasePackage;
class AExplosivePackage;
class UPhysicsConstraintComponent;
class ACoffeeShop;

UCLASS()
class DELIVERYDETONATION_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void PlayerWin();

	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void PlayerLost();

	// Called by ABasePackage trigger overlaps
	void SetOverlappingPackage(ABasePackage* Package);
	void ClearOverlappingPackage(ABasePackage* Package);

	// ---- Bomb fuse carry-over (used when we Destroy+Respawn on pickup/drop)
	UPROPERTY()
	float CachedFuseTime = -1.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UWaypointComponent* WaypointSystem;

	TWeakObjectPtr<AActor> OverlappingInteractable;

	UFUNCTION()
	void SetOverlappingInteractable(AActor* InActor);

	UFUNCTION()
	void ClearOverlappingInteractable(AActor* InActor);

	UFUNCTION()
	void InitHUD(UHUDWidget* InHUD);

	UFUNCTION()
	void RequestRespawn();

	UFUNCTION(Server, Reliable)
	void Server_RequestRespawn();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Category = Character, VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(Category = Character, VisibleAnywhere)
	UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UHUDWidget> PlayerHUDClass;

	UPROPERTY()
	UHUDWidget* PlayerHUD;

	UPROPERTY(VisibleAnywhere)
	UCaffeineComponent* CaffeineComp;

	// World package in range (set by package trigger)
	TWeakObjectPtr<ABasePackage> OverlappingPackage;

	// Carried package actor attached to player (spawned)
	UPROPERTY(ReplicatedUsing = OnRep_CarriedPackage)
	TWeakObjectPtr<ABasePackage> CarriedPackage;

	UPROPERTY(EditDefaultsOnly, Category = "Respawn")
	float DeathRespawnDelay = 2.0f;

	// Input handler
	UFUNCTION(Server, Reliable)
	void Server_Interact();

	// Spawn/Drop
	ABasePackage* SpawnCarriedPackage(TSubclassOf<ABasePackage> ClassToSpawn, const FVector& WorldScale);
	void DropCarriedPackage();

	// UI helper
	void SetPickupPromptVisible(bool bVisible);

	// Helpers
	bool CanPickup() const;

	bool CanInteract() const;

	void ApplyCarryMovementModifiers();

	UFUNCTION()
	void OnRep_CarriedPackage();

public:
	// --- MOVEMENT PROPERTIES ---
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Movement")
	bool bIsSliding = false;

	FTimerHandle SlideTimerHandle;

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsCrouching() const { return bIsCrouching; }

	UFUNCTION(BlueprintPure, Category = "Movement")
	bool IsCarrying() const;

	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	float SlideDuration = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float WalkSpeed = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunSpeed = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float SlideSpeed = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float RunningFOV = 95;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float DefaultFOV = 90;

	UFUNCTION(BlueprintPure, Category = "Action")
	bool IsActionHappening() const { return bActionHappening; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	UPhysicsConstraintComponent* HandConstraintComp;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	ABasePackage* GetCarriedPackage() const {
		return CarriedPackage.Get();
	}

	void SuccessDelivery();

protected:
	// --- INPUT SYSTEM ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* RunAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* SlideAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* PauseAction;

	// --- CROUCHING ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float CrouchSpeed = 300.f;

	// --- SLIDING CONFIG ---
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sliding")
	float SlideFriction = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sliding")
	float SlideBrakingDeceleration = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sliding")
	float SlideImpulseAmount = 900.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Sliding")
	TSubclassOf<class UCameraShakeBase> SlideCameraShakeClass;

	// ---- Caffeine Speed Scaling ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caffeine|Speed")
	float CaffeineMinMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caffeine|Speed")
	float CaffeineMaxMultiplier = 1.35f;

	float DefaultJumpZVelocity = 0.f;

private:
	// --- MOVEMENT FUNCTIONS ---
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartRunning();
	void StopRunning();
	void StartCrouching();
	void StopCrouching();
	void StartSliding();
	void StopSliding();
	void CancelSlide();
	float GetCaffeineMultiplier() const;

	void UpdateMovementSpeedFromState();

	UFUNCTION()
	void OnCaffeineChanged(float Current, float Max);

	UFUNCTION(Server, Reliable) void Server_StartRunning();
	UFUNCTION(Server, Reliable) void Server_StopRunning();
	UFUNCTION(Server, Reliable) void Server_StartCrouching();
	UFUNCTION(Server, Reliable) void Server_StopCrouching();
	UFUNCTION(Server, Reliable) void Server_StartSliding();
	UFUNCTION(Server, Reliable) void Server_StopSliding();

	void ApplySlidePhysics();
	void ResetSlidePhysics();

	UPROPERTY(Replicated)
	bool bIsCrouching = false;

	UPROPERTY(Replicated)
	bool bActionHappening = false;

	bool bWantsToRun = false;

	void HandlePauseAction();

	float DefaultGroundFriction;
	float DefaultBrakingDeceleration;

	// helper to configure collision ignoring
	void IgnorePackageCollision(class UPrimitiveComponent* PackageMesh, bool bIgnore);

	virtual void OnDeath() override;
	virtual void OnHurt(float NewHealth) override;
};