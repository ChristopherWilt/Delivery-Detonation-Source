// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_TriggerRagdoll.generated.h"

/**
 * 
 */
UCLASS()
class DELIVERYDETONATION_API UAnimNotify_TriggerRagdoll : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	// The function that runs when the animation hits the notify
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
