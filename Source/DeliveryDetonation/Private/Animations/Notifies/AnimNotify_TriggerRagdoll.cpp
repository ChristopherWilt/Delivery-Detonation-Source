// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/Notifies/AnimNotify_TriggerRagdoll.h"

#include "Character/BaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_TriggerRagdoll::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp) return;

	// 1. Find the Character that owns this mesh
	ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner());

	// 2. Trigger the "Crash Out"
	if (Character)
	{
		Character->TriggerFullRagdoll();
	}
}