// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animations/PlayerAnimation.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/HUDWidget.h"
#include "ActorComponents/HealthComponent.h"
#include "Game/GamePlayerController.h"
#include "Game/DeliveryDetonation_GameMode.h"
#include "ActorComponents/CaffeineComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "Pickups/BasePackage.h"
#include "Pickups/ExplosivePackage.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Game/DeliveryDetonation_GameState.h"
#include "Game/CoffeeShop.h"


// Sets default values
APlayerCharacter::APlayerCharacter()
{
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetRelativeLocation(FVector(0.0f, 40.0f, 70.0f));
	SpringArm->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	CaffeineComp = CreateDefaultSubobject<UCaffeineComponent>("CaffeineComp");

	HandConstraintComp = CreateDefaultSubobject<UPhysicsConstraintComponent>(TEXT("HandConstraintComp"));
	HandConstraintComp->SetupAttachment(GetMesh(), FName("hand_r")); // Attach to holding hand bone

	// Configure default constraint settings (Locked = stiff hold, Limited = floppy hold)
	HandConstraintComp->SetAngularSwing1Limit(ACM_Locked, 0.f);
	HandConstraintComp->SetAngularSwing2Limit(ACM_Locked, 0.f);
	HandConstraintComp->SetAngularTwistLimit(ACM_Locked, 0.f);
	HandConstraintComp->SetLinearXLimit(LCM_Locked, 0.f);
	HandConstraintComp->SetLinearYLimit(LCM_Locked, 0.f);
	HandConstraintComp->SetLinearZLimit(LCM_Locked, 0.f);

	// Disable collision between the hand and the object held
	HandConstraintComp->SetDisableCollision(true);

	WaypointSystem = CreateDefaultSubobject<UWaypointComponent>(TEXT("WaypointSystem"));
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Input mapping (local only)
	if (IsLocallyControlled())
	{
		if (APlayerController* PC = Cast<APlayerController>(Controller))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	// Cache anim instance
	if (GetMesh())
	{
		CharacterAnim = Cast<UPlayerAnimation>(GetMesh()->GetAnimInstance());
		AnimInstance = CharacterAnim;

		if (!AnimInstance)
		{
			UE_LOG(LogTemp, Warning, TEXT("AnimInstance is null in APlayerCharacter."));
		}

		// Bind Death Event (make sure we don't double-bind on respawn edge cases)
		if (UPlayerAnimation* Anim = Cast<UPlayerAnimation>(GetMesh()->GetAnimInstance()))
		{
			Anim->OnDeathEnded.RemoveAll(this);
			Anim->OnDeathEnded.AddDynamic(this, &APlayerCharacter::RequestRespawn);
		}
	}

	// Store default movement values
	DefaultGroundFriction = GetCharacterMovement()->GroundFriction;
	DefaultBrakingDeceleration = GetCharacterMovement()->BrakingDecelerationWalking;
	DefaultJumpZVelocity = GetCharacterMovement()->JumpZVelocity;

	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

void APlayerCharacter::InitHUD(UHUDWidget* InHUD)
{
	if (!IsLocallyControlled()) return;

	PlayerHUD = InHUD;
	if (!PlayerHUD) return;

	// --- Reset HUD display to a clean state ---
	PlayerHUD->SetPickupPromptVisible(false);
	PlayerHUD->SetInteractPrompt(false, TEXT(""));

	// --- HEALTH ---
	if (HealthComponent)
	{
		// Bind HUD updates
		HealthComponent->OnHealthChanged.RemoveAll(PlayerHUD);
		HealthComponent->OnHealthChanged.AddDynamic(PlayerHUD, &UHUDWidget::SetHealth);

		// Push initial values immediately after respawn
		PlayerHUD->SetHealth(HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth());
	}

	// --- CAFFEINE ---
	if (CaffeineComp)
	{
		// Push initial values
		PlayerHUD->SetCaffeine(CaffeineComp->GetCurrent(), CaffeineComp->GetMax());

		// Bind HUD + player responses
		CaffeineComp->OnCaffeineChanged.RemoveAll(PlayerHUD);
		CaffeineComp->OnCaffeineChanged.AddDynamic(PlayerHUD, &UHUDWidget::SetCaffeine);

		CaffeineComp->OnCaffeineChanged.RemoveAll(this);
		CaffeineComp->OnCaffeineChanged.AddDynamic(this, &APlayerCharacter::OnCaffeineChanged);
	}

	// --- GAMESTATE (timer + cargo) ---
	if (UWorld* World = GetWorld())
	{
		// next tick helps when GS isn't ready on the same frame of respawn/possess
		GetWorldTimerManager().SetTimerForNextTick([this]()
			{
				if (!PlayerHUD) return;

				if (ADeliveryDetonation_GameState* GS = GetWorld()->GetGameState<ADeliveryDetonation_GameState>())
				{
					GS->OnTimeUpdated.RemoveAll(PlayerHUD);
					GS->OnTimeUpdated.AddDynamic(PlayerHUD, &UHUDWidget::SetTimeSeconds);

					GS->OnCargoUpdated.RemoveAll(PlayerHUD);
					GS->OnCargoUpdated.AddDynamic(PlayerHUD, &UHUDWidget::SetDeliveries);

					// initial push
					PlayerHUD->SetTimeSeconds(GS->TimeRemaining);
					PlayerHUD->SetDeliveries(GS->DeliveredCargo, GS->TotalCargo);
				}
			});
	}
	if (IsLocallyControlled())
	{
		GetWorldTimerManager().SetTimerForNextTick([this]()
			{
				if (PlayerHUD && HealthComponent)
				{
					PlayerHUD->SetHealth(HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth());
				}
			});
	}
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!CaffeineComp) return;

	CaffeineComp->Consume(DeltaTime);

	// Only update speed if we are not sliding.
	// Sliding has its own physics + speed rules.
	if (!bIsSliding)
	{
		UpdateMovementSpeedFromState();
	}

}

// Called to bind functionality to input
void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// ---MOVEMENT ---
		EIC->BindAction(RunAction, ETriggerEvent::Started, this, &APlayerCharacter::StartRunning);
		EIC->BindAction(RunAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopRunning);
		EIC->BindAction(CrouchAction, ETriggerEvent::Started, this, &APlayerCharacter::StartCrouching);
		EIC->BindAction(CrouchAction, ETriggerEvent::Completed, this, &APlayerCharacter::StopCrouching);
		EIC->BindAction(SlideAction, ETriggerEvent::Started, this, &APlayerCharacter::StartSliding);
		EIC->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Look);
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Move);
		EIC->BindAction(PauseAction, ETriggerEvent::Started, this, &APlayerCharacter::HandlePauseAction);
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &APlayerCharacter::Server_Interact);
	}
}

void APlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// --- ALL REPLICATED VARIABLES ---
	DOREPLIFETIME(APlayerCharacter, bIsCrouching);
	DOREPLIFETIME(APlayerCharacter, bActionHappening);
	DOREPLIFETIME(APlayerCharacter, bIsSliding);
	DOREPLIFETIME(APlayerCharacter, CarriedPackage);
}

void APlayerCharacter::PlayerWin()
{
}

void APlayerCharacter::PlayerLost()
{
	if (!IsLocallyControlled())
		return;

	if (AGamePlayerController* GPC = Cast<AGamePlayerController>(GetController()))
	{
		if (GPC->IsPaused())
		{
			GPC->HidePauseMenu();
		}
		else
		{
			GPC->SetGameOnlyInput();
		}

		GPC->Server_RestartLevel();
		return;
	}

	if (AGamePlayerController* GPC2 = Cast<AGamePlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		if (GPC2->IsPaused())
		{
			GPC2->HidePauseMenu();
		}
		else
		{
			GPC2->SetGameOnlyInput();
		}

		GPC2->Server_RestartLevel();
	}
}

void APlayerCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D (X = Side/Side, Y = Forward/Back)
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// Get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// Get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Add movement (Y component of input is Forward/Back)
		AddMovementInput(ForwardDirection, MovementVector.Y);

		// Add movement (X component of input is Right/Left)
		AddMovementInput(RightDirection, MovementVector.X);
	}
}


void APlayerCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y * -1.0f);
}

void APlayerCharacter::StartRunning()
{
	bWantsToRun = true;
	if (!bIsSliding) UpdateMovementSpeedFromState();
	Server_StartRunning();
}

void APlayerCharacter::StopRunning()
{
	bWantsToRun = false;
	if (!bIsSliding) UpdateMovementSpeedFromState();
	Server_StopRunning();
}

void APlayerCharacter::StartCrouching()
{
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	Crouch();
	UpdateMovementSpeedFromState();
	Server_StartCrouching();
}

void APlayerCharacter::StopCrouching()
{
	UnCrouch();
	UpdateMovementSpeedFromState();
	Server_StopCrouching();
}

void APlayerCharacter::StartSliding()
{
	bool bCanSlide = !bIsSliding &&
		GetCharacterMovement()->IsMovingOnGround() &&
		GetCharacterMovement()->Velocity.Size() > (RunSpeed - 50.f);

	if (bCanSlide)
	{
		// A. Apply Locally immediately (Prediction)
		ApplySlidePhysics();

		// B. Tell Server to apply physics too (Fixes Jitter)
		Server_StartSliding();

		// C. Apply Impulse (Velocity)
		FVector SlideDir = GetLastMovementInputVector().GetSafeNormal();
		if (SlideDir.IsNearlyZero()) SlideDir = GetActorForwardVector();
		GetCharacterMovement()->Launch(SlideDir * (SlideImpulseAmount * GetCaffeineMultiplier()));

		// D. Timer
		GetWorldTimerManager().SetTimer(SlideTimerHandle, this, &APlayerCharacter::StopSliding, SlideDuration, false);
	}
}

void APlayerCharacter::StopSliding()
{
	if (bIsSliding)
	{
		// A. Reset Locally
		ResetSlidePhysics();

		// B. Reset on Server
		Server_StopSliding();

		GetWorldTimerManager().ClearTimer(SlideTimerHandle);
	}
}

void APlayerCharacter::CancelSlide()
{
	if (bIsSliding) StopSliding();
}

void APlayerCharacter::Server_StopCrouching_Implementation()
{
	if (bIsCrouching) { 
		bIsCrouching = false;
		UnCrouch();
		UpdateMovementSpeedFromState();
	}
}

void APlayerCharacter::Server_StartCrouching_Implementation()
{
	if (!bIsCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
		bIsCrouching = true;
		Crouch();
		UpdateMovementSpeedFromState();
	}
}

void APlayerCharacter::Server_StopRunning_Implementation()
{
	bWantsToRun = false;
	if (!bIsSliding && !bIsCrouching) UpdateMovementSpeedFromState();
}

void APlayerCharacter::Server_StartRunning_Implementation()
{
	bWantsToRun = true;
	UpdateMovementSpeedFromState();
}

void APlayerCharacter::Server_StartSliding_Implementation()
{
	ApplySlidePhysics();
}

void APlayerCharacter::Server_StopSliding_Implementation()
{
	ResetSlidePhysics();
}

void APlayerCharacter::ApplySlidePhysics()
{
	bIsSliding = true;

	float SlideCarryMult = 1.0f;
	if (CarriedPackage.IsValid())
	{
		SlideCarryMult = CarriedPackage->GetSlideFrictionMultiplier();
	}

	GetCharacterMovement()->GroundFriction = SlideFriction * SlideCarryMult;
	GetCharacterMovement()->BrakingDecelerationWalking = SlideBrakingDeceleration * SlideCarryMult;

	// ALLOW FAST MOVEMENT IN CROUCH HITBOX
	GetCharacterMovement()->MaxWalkSpeedCrouched = SlideSpeed;

	Crouch(); // Shrink Hitbox

	// Trigger Camera Shake (Client Only check inside)
	if (IsLocallyControlled() && SlideCameraShakeClass)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			PC->ClientStartCameraShake(SlideCameraShakeClass, 1.0f);
		}
	}
}

void APlayerCharacter::ResetSlidePhysics()
{
	bIsSliding = false;

	// Restore Grip
	GetCharacterMovement()->GroundFriction = DefaultGroundFriction;
	GetCharacterMovement()->BrakingDecelerationWalking = DefaultBrakingDeceleration;

	// RESTORE SLOW CROUCH SPEED (Crucial Fix)
	GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;

	// Reset Hitbox
	UnCrouch();

	// Determine Speed based on synced bool
	if (bWantsToRun)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}

	UpdateMovementSpeedFromState();
}

void APlayerCharacter::HandlePauseAction()
{
	if (AGamePlayerController* GPC = Cast<AGamePlayerController>(GetController()))
	{
		GPC->TogglePauseMenu(); // Reuse the logic you already wrote in the controller
	}
}

void APlayerCharacter::OnDeath()
{
	Super::OnDeath();

	// Disable input immediately on death (local feel)
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}

	if (PlayerHUD && HealthComponent)
	{
		PlayerHUD->SetHealth(0, HealthComponent->GetMaxHealth());
	}

	// IMPORTANT: Only the server should actually respawn
	// If we’re client, ask the server.
	if (HasAuthority())
	{
		// Server can directly restart after a delay
		FTimerHandle RespawnTimer;
		GetWorldTimerManager().SetTimer(RespawnTimer, [this]()
			{
				if (!IsValid(this)) return;

				AController* C = GetController();
				if (!C) return;

				// Let GameMode handle it
				if (UWorld* World = GetWorld())
				{
					if (AGameModeBase* GM = World->GetAuthGameMode())
					{
						GM->RestartPlayer(C);
					}
				}
			}, DeathRespawnDelay, false);
	}
	else
	{
		// Client asks server to do it
		FTimerHandle RespawnTimer;
		GetWorldTimerManager().SetTimer(RespawnTimer, this, &APlayerCharacter::Server_RequestRespawn, DeathRespawnDelay, false);
	}
}

void APlayerCharacter::OnHurt(float NewHealth)
{
	Super::OnHurt(NewHealth);
	if (PlayerHUD)
	{
		PlayerHUD->SetHealth(NewHealth, HealthComponent->GetMaxHealth());
	}
}

void APlayerCharacter::UpdateMovementSpeedFromState()
{
	if (!CaffeineComp) return;

	const float Ratio = (CaffeineComp->GetMax() > 0.f)
		? (CaffeineComp->GetCurrent() / CaffeineComp->GetMax())
		: 0.f;

	const float Mult = FMath::Lerp(CaffeineMinMultiplier, CaffeineMaxMultiplier, Ratio);

	// Choose a base speed depending on your current movement intent/state.
	float Base = WalkSpeed;

	// If you're crouching and not sliding, use crouch speed.
	if (bIsCrouching && !bIsSliding)
	{
		Base = CrouchSpeed;
	}
	// If you're holding run and not crouching/sliding, use run speed.
	else if (bWantsToRun && !bIsSliding)
	{
		Base = RunSpeed;
	}

	float CarryMult = 1.0f;
	if (CarriedPackage.IsValid())
	{
		CarryMult = CarriedPackage->GetCarrySpeedMultiplier();
	}

	GetCharacterMovement()->MaxWalkSpeed = Base * Mult * CarryMult;
}

void APlayerCharacter::OnCaffeineChanged(float Current, float Max)
{
	
	if (PhysicalAnimationComponent && CharacterAnim)
	{
		float CaffeinePercent = (Max > 0.f) ? (Current / Max) : 0.f;
		float SpeedFactor = FMath::Clamp(GetVelocity().Size() / 1200.0f, 0.0f, 1.0f);

		FPhysicalAnimationData NewData;
		NewData.bIsLocalSimulation = true;

		// At 1200 speed, we boost the minimum strength to 15,000 to "lock" the arms
		float MinStrength = FMath::Lerp(3000.0f, 15000.0f, SpeedFactor);
		NewData.OrientationStrength = FMath::Lerp(MinStrength, 60000.0f, CaffeinePercent);

		// Boost PositionStrength to act as "glue" against the wind at 1200 speed
		float MinPosition = FMath::Lerp(1000.0f, 10000.0f, SpeedFactor);
		NewData.PositionStrength = FMath::Lerp(MinPosition, 40000.0f, CaffeinePercent);

		PhysicalAnimationComponent->ApplyPhysicalAnimationSettingsBelow(FName("spine_03"), NewData);
	}

	if (CharacterAnim)
	{
		float CaffeinePercent = (Max > 0.f) ? (Current / Max) : 0.f;

		// MAP CAFFEINE TO PHYSICS INTENSITY:
		// 0% Caffeine -> 0.8 Alpha (75% flimsiness feel)
		// 100% Caffeine -> 0.05 Alpha (Very rigid / 5% feel)
		CharacterAnim->PhysicsAlpha = FMath::Lerp(0.80f, 0.05f, CaffeinePercent);
	}


	if (bIsSliding)
	{
		// Update slide speed cap live while sliding
		GetCharacterMovement()->MaxWalkSpeedCrouched = SlideSpeed * GetCaffeineMultiplier();
	}
	else
	{
		UpdateMovementSpeedFromState();
	}
}

float APlayerCharacter::GetCaffeineMultiplier() const
{
	if (!CaffeineComp) return 1.f;
	const float Ratio = (CaffeineComp->GetMax() > 0.f) ? (CaffeineComp->GetCurrent() / CaffeineComp->GetMax()) : 0.f;
	return FMath::Lerp(CaffeineMinMultiplier, CaffeineMaxMultiplier, Ratio);
}

bool APlayerCharacter::IsCarrying() const
{
	return CarriedPackage.IsValid();
}

bool APlayerCharacter::CanPickup() const
{
	return !IsCarrying() && OverlappingPackage.IsValid();
}

void APlayerCharacter::SetPickupPromptVisible(bool bVisible)
{
	if (!IsLocallyControlled() || !PlayerHUD) return;
	PlayerHUD->SetPickupPromptVisible(bVisible);
}

void APlayerCharacter::SetOverlappingPackage(ABasePackage* Package)
{
	if (!IsValid(Package)) return;

	if (OverlappingInteractable.IsValid())
		return;
	// If we’re carrying something, ignore prompts entirely
	if (IsCarrying())
		return;

	OverlappingPackage = Package;
	SetPickupPromptVisible(true);
}

void APlayerCharacter::ClearOverlappingPackage(ABasePackage* Package)
{
	if (!Package) return;

	// Only clear if it's the same package
	if (OverlappingPackage.Get() == Package)
	{
		OverlappingPackage = nullptr;
		SetPickupPromptVisible(false);
	}
}

void APlayerCharacter::Server_Interact_Implementation()
{
	if (OverlappingInteractable.IsValid())
	{
		if (ACoffeeShop* Shop = Cast<ACoffeeShop>(OverlappingInteractable.Get()))
		{
			Shop->Server_TryPurchase(this); // server-side purchase
			return;
		}
	}
	// Drop if carrying
	if (CarriedPackage.IsValid())
	{
		DropCarriedPackage();
		SetPickupPromptVisible(false);
		return;
	}
	else
	{
		CarriedPackage = nullptr;
	}

	// Pickup if in range
	if (!CanPickup())
	{
		SetPickupPromptVisible(false);
		return;
	}

	ABasePackage* WorldPkg = OverlappingPackage.Get();
	if (!IsValid(WorldPkg) || WorldPkg->IsActorBeingDestroyed() || WorldPkg->IsPendingKillPending())
	{
		OverlappingPackage = nullptr;
		SetPickupPromptVisible(false);
		return;
	}

	// Ask the package what carried visual to spawn
	TSubclassOf<ABasePackage> CarryClass = WorldPkg->GetCarriedVisualClass();
	if (!CarryClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pickup] WorldPkg has no CarriedVisualClass set: %s"), *GetNameSafe(WorldPkg));
		return;
	}

	const FVector WorldScale = WorldPkg->GetActorScale3D();

	OverlappingPackage = nullptr;
	SetPickupPromptVisible(false);

	if (AExplosivePackage* Bomb = Cast<AExplosivePackage>(WorldPkg))
	{
		float Remaining = Bomb->GetRemainingFuseTime();
		CachedFuseTime = Remaining; // store locally
	}

	WorldPkg->Destroy();

	ABasePackage* NewCarried = SpawnCarriedPackage(CarryClass, WorldScale);
	if (!IsValid(NewCarried))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pickup] Failed to spawn carried visual"));
		return;
	}

	CarriedPackage = NewCarried;
	ApplyCarryMovementModifiers();

	// --- TRIGGER OBJECTIVE ---
	if (ADeliveryDetonation_GameMode* GM = GetWorld()->GetAuthGameMode<ADeliveryDetonation_GameMode>())
	{
		// NEW: Assign the destination!
		GM->AssignDeliveryObjective(GetController());
	}
}

ABasePackage* APlayerCharacter::SpawnCarriedPackage(TSubclassOf<ABasePackage> ClassToSpawn, const FVector& WorldScale)
{
	if (!ClassToSpawn) return nullptr;

	FTransform HandTM = GetMesh()->GetSocketTransform(FName("CarrySocket"), RTS_World);

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 1. Spawn Deferred
	ABasePackage* NewPkg = GetWorld()->SpawnActorDeferred<ABasePackage>(
		ClassToSpawn, HandTM, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!NewPkg) return nullptr;

	NewPkg->SetAsCarriedVisual(true);
	NewPkg->FinishSpawning(HandTM);
	NewPkg->SetReplicateMovement(false); // <--- Stop syncing position, let the constraint handle it.
	NewPkg->SetActorScale3D(WorldScale);

	// Bomb Logic
	if (AExplosivePackage* Bomb = Cast<AExplosivePackage>(NewPkg))
	{
		if (CachedFuseTime > 0.f) {
			Bomb->FuseSeconds = CachedFuseTime;
			Bomb->StartFuseFromPickup();
			CachedFuseTime = -1.f;
		}
		else {
			Bomb->StartFuseFromPickup();
		}
	}

	// THE PHYSICS FIX (Overrides Blueprint Settings)
	if (UPrimitiveComponent* PkgMesh = Cast<UPrimitiveComponent>(NewPkg->GetRootComponent()))
	{
		PkgMesh->SetMassOverrideInKg(NAME_None, NewPkg->GetPackageWeight(), true);
		// FORCE NO COLLISION: This overrides the "Block" checkbox you have in the Blueprint
		PkgMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		PkgMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

		// FORCE IGNORE CAPSULE: Double safety
		IgnorePackageCollision(PkgMesh, true);

		// 4. ALIGNMENT (Math to match Socket to Hand)
		FTransform TargetHandTM = GetMesh()->GetSocketTransform(FName("CarrySocket"), RTS_World);

		// Default final location if socket is missing
		FVector FinalLoc = TargetHandTM.GetLocation();
		FQuat FinalRot = TargetHandTM.GetRotation();

		if (PkgMesh->DoesSocketExist(FName("RightHandGrip")))
		{
			FTransform GripRelativeTM = PkgMesh->GetSocketTransform(FName("RightHandGrip"), RTS_Component);
			FTransform NewPackageTM = GripRelativeTM.Inverse() * TargetHandTM;
			FinalLoc = NewPackageTM.GetLocation();
			FinalRot = NewPackageTM.GetRotation();
		}

		// 5. TELEPORT PHYSICS (Prevents "Sweeping" into the body)
		NewPkg->SetActorLocationAndRotation(FinalLoc, FinalRot, false, nullptr, ETeleportType::TeleportPhysics);

		// 6. LOCK IT
		HandConstraintComp->SetConstrainedComponents(GetMesh(), FName("hand_r"), PkgMesh, NAME_None);
	}

	return NewPkg;
}

void APlayerCharacter::OnRep_CarriedPackage()
{
	// Case 1: We just picked up the package
	if (CarriedPackage.IsValid())
	{
		ABasePackage* Pkg = CarriedPackage.Get();
		if (UPrimitiveComponent* PkgMesh = Cast<UPrimitiveComponent>(Pkg->GetRootComponent()))
		{
			// Ignore Collision (So we don't trip over it)
			IgnorePackageCollision(PkgMesh, true);
			PkgMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			PkgMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

			// DISABLE MOVEMENT REPLICATION
			// We want the constraint to move it locally. If we leave this on, 
			// the Server will fight the Client's local physics, causing jitter.
			Pkg->SetReplicateMovement(false);

			// LOCK The Constraint
			HandConstraintComp->SetConstrainedComponents(GetMesh(), FName("hand_r"), PkgMesh, NAME_None);
		}
	}
	// Case 2: We dropped it (or it was destroyed)
	else
	{
		// Break the link
		HandConstraintComp->BreakConstraint();
	}
	ApplyCarryMovementModifiers();
}

void APlayerCharacter::DropCarriedPackage()
{
	if (!CarriedPackage.IsValid()) return;

	ABasePackage* CarryPkg = CarriedPackage.Get();
	if (!IsValid(CarryPkg)) { CarriedPackage = nullptr; return; }

	if (AExplosivePackage* Bomb = Cast<AExplosivePackage>(CarryPkg))
	{
		CachedFuseTime = Bomb->GetRemainingFuseTime();
	}

	// Break the constraint immediately
	HandConstraintComp->BreakConstraint();

	// Re-enable collision between player and this specific package mesh if needed
	if (CarriedPackage.IsValid())
	{
		if (UPrimitiveComponent* PkgRoot = Cast<UPrimitiveComponent>(CarriedPackage->GetRootComponent()))
		{
			IgnorePackageCollision(PkgRoot, false);
		}
	}

	// Ask carried visual what world class to spawn
	TSubclassOf<ABasePackage> DropClass = CarryPkg->GetWorldDropClass();
	if (!DropClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Drop] Carried package has no WorldDropClass set: %s"), *GetNameSafe(CarryPkg));
		return;
	}

	OverlappingPackage = nullptr;

	const FVector DropLoc = GetActorLocation() + GetActorForwardVector() * 120.f + FVector(0, 0, 40);
	const FRotator DropRot = GetActorRotation();
	FTransform DropTM(DropRot, DropLoc);

	const FVector CarryScale = CarryPkg->GetActorScale3D();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ABasePackage* WorldPkg = GetWorld()->SpawnActorDeferred<ABasePackage>(
		DropClass,
		DropTM,
		this,
		this,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
	);

	if (!WorldPkg)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Drop] Failed to spawn WorldDropClass"));
		return;
	}

	WorldPkg->SetAsCarriedVisual(false);
	WorldPkg->FinishSpawning(DropTM);

	WorldPkg->SetActorScale3D(CarryScale);

	if (AExplosivePackage* Bomb = Cast<AExplosivePackage>(WorldPkg))
	{
		if (CachedFuseTime > 0.f)
		{
			Bomb->FuseSeconds = CachedFuseTime;
			Bomb->StartFuseFromPickup();  // or StartFuseFromPickup() even when dropped
			CachedFuseTime = -1.f;
		}
	}

	// Force overlaps
	if (USphereComponent* Trigger = WorldPkg->GetCollisionTrigger())
	{
		Trigger->UpdateOverlaps();
	}

	CarryPkg->Destroy();
	CarriedPackage = nullptr;
	ApplyCarryMovementModifiers();

	SetPickupPromptVisible(false);
}

// Helper function implementation
void APlayerCharacter::IgnorePackageCollision(UPrimitiveComponent* PackageMesh, bool bIgnore)
{
	// Safety checks
	if (!PackageMesh || !GetMesh() || !GetCapsuleComponent()) return;

	// Ignore the character capsule (Collision Cylinder)
	GetCapsuleComponent()->IgnoreComponentWhenMoving(PackageMesh, bIgnore);
	PackageMesh->IgnoreComponentWhenMoving(GetCapsuleComponent(), bIgnore);

	// Ignore the character mesh (The Skeletal Mesh)
	GetMesh()->IgnoreComponentWhenMoving(PackageMesh, bIgnore);
	PackageMesh->IgnoreComponentWhenMoving(GetMesh(), bIgnore);
}

void APlayerCharacter::ApplyCarryMovementModifiers()
{
	UCharacterMovementComponent* Move = GetCharacterMovement();
	if (!Move) return;

	float JumpMult = 1.0f;

	if (CarriedPackage.IsValid())
	{
		JumpMult = CarriedPackage->GetCarryJumpMultiplier();
	}

	Move->JumpZVelocity = DefaultJumpZVelocity * JumpMult;

	// Update walking/running speed (includes caffeine + carry)
	if (!bIsSliding)
		UpdateMovementSpeedFromState();
}

void APlayerCharacter::SuccessDelivery()
{
	// Clean up hands without triggering a drop
	if (HandConstraintComp)
	{
		HandConstraintComp->BreakConstraint();
	}

	// The Package->Deliver() call destroys the actor, so we just clear the pointer
	CarriedPackage = nullptr;

	// Reset movement physics
	ApplyCarryMovementModifiers();
}

void APlayerCharacter::SetOverlappingInteractable(AActor* InActor)
{
	if (!IsValid(InActor)) return;

	OverlappingInteractable = InActor;

	if (InActor->IsA(ACoffeeShop::StaticClass()))
	{
		if (IsLocallyControlled() && PlayerHUD)
		{
			PlayerHUD->SetInteractPrompt(true, TEXT("Press E to Buy Coffee"));
		}
	}
}

void APlayerCharacter::ClearOverlappingInteractable(AActor* InActor)
{
	if (OverlappingInteractable.Get() == InActor)
	{
		OverlappingInteractable = nullptr;

		if (IsLocallyControlled() && PlayerHUD)
		{
			PlayerHUD->SetInteractPrompt(false, TEXT(""));
		}
	}
}

bool APlayerCharacter::CanInteract() const
{
	return OverlappingInteractable.IsValid();
}

void APlayerCharacter::RequestRespawn()
{
	// Only the owning client should request respawn
	if (!IsLocallyControlled())
		return;

	if (AGamePlayerController* GPC = Cast<AGamePlayerController>(GetController()))
	{
		// If your UI was open, restore game input before respawn
		if (GPC->IsPaused())
		{
			GPC->HidePauseMenu();
		}
		else
		{
			GPC->SetGameOnlyInput();
		}
	}

	Server_RequestRespawn();
}

void APlayerCharacter::Server_RequestRespawn_Implementation()
{
	if (!HasAuthority()) return;

	// GameMode does the respawn
	if (ADeliveryDetonation_GameMode* GM = GetWorld()->GetAuthGameMode<ADeliveryDetonation_GameMode>())
	{
		// Don't respawn if match is over (postmatch)
		if (ADeliveryDetonation_GameState* GS = GetWorld()->GetGameState<ADeliveryDetonation_GameState>())
		{
			if (GS->MatchPhase == EMatchPhase::PostMatch)
				return;
		}

		AController* C = GetController();
		if (!C) return;

		C->UnPossess();

		// Destroy old pawn so it doesn't block spawns / keep ragdoll around forever
		SetLifeSpan(2.0f);

		FTimerHandle Tmp;
		GetWorldTimerManager().SetTimer(Tmp, FTimerDelegate::CreateLambda([GM, C]()
			{
				if (IsValid(GM) && IsValid(C))
					GM->RestartPlayer(C);
			}), 2.0f, false);
	}
}