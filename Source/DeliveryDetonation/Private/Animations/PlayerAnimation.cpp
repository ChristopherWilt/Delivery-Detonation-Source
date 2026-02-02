// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/PlayerAnimation.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/NamedSlot.h"
#include "TimerManager.h" 
#include "Player/PlayerCharacter.h"
#include "KismetAnimationLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Pickups/BasePackage.h"


UPlayerAnimation::UPlayerAnimation()
{
	SlotName = FName("ActionSlot");

	static ConstructorHelpers::FObjectFinder<UAnimSequence> Death1AnimAssetFinder(TEXT("/Game/Animations/Death/A_Death1.A_Death1"));
	static ConstructorHelpers::FObjectFinder<UAnimSequence> Death2AnimAssetFinder(TEXT("/Game/Animations/Death/A_Death2.A_Death2"));

	DeathAnimationAssets.Empty();
	if (Death1AnimAssetFinder.Succeeded())
	{
		DeathAnimationAssets.Add(Death1AnimAssetFinder.Object);
	}
	if (Death2AnimAssetFinder.Succeeded())
	{
		DeathAnimationAssets.Add(Death2AnimAssetFinder.Object);
	}

	CurrentDeathAnimationAsset = nullptr;

}

void UPlayerAnimation::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
}

void UPlayerAnimation::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	PreviewWindowUpdate();

	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());
	}

	APawn* Pawn = TryGetPawnOwner();
	if (Pawn)
	{
		// is Valid
		Velocity = Pawn->GetVelocity().Length();
		FRotator PlayerRotation = Pawn->GetActorRotation();
		Direction = UKismetAnimationLibrary::CalculateDirection(Pawn->GetVelocity(), PlayerRotation);
	}
	
	if (PlayerCharacter && PlayerCharacter->GetCharacterMovement())
	{
		bIsAccelerating = PlayerCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0;
		bIsInAir = PlayerCharacter->GetCharacterMovement()->IsFalling();
		bIsCrouching = PlayerCharacter->IsCrouching();
		bIsSliding = PlayerCharacter->bIsSliding;
		bIsCarrying = PlayerCharacter->IsCarrying();

		// --- LEFT HAND IK LOGIC ---
		ABasePackage* Package = PlayerCharacter->GetCarriedPackage();

		if (Package && bIsCarrying)
		{
			// Smoothly blend IK on
			IKAlpha = FMath::FInterpTo(IKAlpha, 1.0f, DeltaSeconds, 10.0f);

			// FIX: Check for Socket existence safely
			if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(Package->GetRootComponent()))
			{
				if (Mesh->DoesSocketExist(FName("LeftHandGrip")))
				{
					FVector TargetLoc = Mesh->GetSocketLocation(FName("LeftHandGrip"));
					FRotator TargetRot = Mesh->GetSocketRotation(FName("LeftHandGrip"));

					LeftHandIKLocation = TargetLoc;
					LeftHandIKRotation = TargetRot;

				}
				else
				{
					// Fallback: Math-based offset if you forgot the socket
					FVector PkgLoc = Package->GetActorLocation();
					FRotator PkgRot = Package->GetActorRotation();
					FVector GripOffset = -PkgRot.RotateVector(FVector::RightVector) * 25.0f; // Approx left side
					LeftHandIKLocation = PkgLoc + GripOffset;
				}
			}
		}
		else
		{
			// Smoothly blend IK off
			IKAlpha = FMath::FInterpTo(IKAlpha, 0.0f, DeltaSeconds, 10.0f);
		}
	}
}

void UPlayerAnimation::SelectDeathAnimation_Implementation()
{
	// Select a random death animation if the parent hasn't already
	if (DeathAnimationAssets.Num() > 0 && CurrentDeathAnimationAsset == nullptr)
	{
		const int32 RandomIndex = FMath::RandRange(0, DeathAnimationAssets.Num() - 1);
		CurrentDeathAnimationAsset = DeathAnimationAssets[RandomIndex];
	}

	// If we have a valid animation, set a timer for its duration
	if (CurrentDeathAnimationAsset)
	{
		// Get the length of the animation
		float AnimLength = CurrentDeathAnimationAsset->GetPlayLength();

		UWorld* World = GetWorld();
		if (World)
		{
			// Clear any existing timer before setting a new one
			World->GetTimerManager().ClearTimer(DeathAnimationTimerHandle);

			// Set a timer to call OnDeathAnimationFinished after the animation length
			World->GetTimerManager().SetTimer(
				DeathAnimationTimerHandle,
				this,
				&UPlayerAnimation::OnDeathAnimationFinished,
				AnimLength,
				false // Not looping
			);
		}
	}
}

void UPlayerAnimation::PreviewWindowUpdate_Implementation()
{
	if (DebugDeath)
	{
		SelectDeathAnimation();
		DebugDeath = false;
	}
}

void UPlayerAnimation::CallOnDeathEnded()
{
	if (OnDeathEnded.IsBound())
	{
		OnDeathEnded.Broadcast();
	}
	else {
	}
}

void UPlayerAnimation::OnDeathAnimationFinished()
{
	CallOnDeathEnded();
}