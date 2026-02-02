// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "KismetAnimationLibrary.h"
#include "PlayerAnimation.generated.h"

class APlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathEndedSignature);

/**
 * 
 */
UCLASS()
class DELIVERYDETONATION_API UPlayerAnimation : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPlayerAnimation();
	virtual void NativeInitializeAnimation() override;

	//virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(BlueprintAssignable, Category = "Animation Events")
	FOnDeathEndedSignature OnDeathEnded;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|Playback")
	UAnimSequence* CurrentIdleAnimation;


	// --- MOVEMENT DATA (For AnimGraph) ---
	UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
	bool bIsCrouching;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
	bool bIsSliding;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|State")
	bool bIsCarrying = false;

	UPROPERTY(BlueprintReadOnly, Category = "Physics")
	float PhysicsAlpha = 0.5f; // Default starting point

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	FVector LeftHandIKLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
	FRotator LeftHandIKRotation;

	UPROPERTY(BlueprintReadOnly, Category = "Animation|IK")
	float IKAlpha = 0.0f;

protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Default)
	float Velocity;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Default)
	float Direction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TArray<TObjectPtr<UAnimSequenceBase>> DeathAnimationAssets;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	FName SlotName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Default)
	bool DebugDeath = false;

	FTimerHandle DeathAnimationTimerHandle;


public:
	UFUNCTION(BlueprintNativeEvent)
	void PreviewWindowUpdate();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SelectDeathAnimation();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimSequenceBase> CurrentDeathAnimationAsset = nullptr;

	UFUNCTION(BlueprintCallable)
	void CallOnDeathEnded();

protected:
	UFUNCTION()
	void OnDeathAnimationFinished();

private:
	UPROPERTY()
	APlayerCharacter* PlayerCharacter;

	FVector CurrentIKLoc;
	FRotator CurrentIKRot;
};
