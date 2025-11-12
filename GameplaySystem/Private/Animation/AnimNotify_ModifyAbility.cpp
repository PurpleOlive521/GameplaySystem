// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "Animation/AnimNotify_ModifyAbility.h"

#include "GameplaySystemComponent.h"
#include "GameplaySystemOwnerInterface.h"
#include "DevelopmentTypes.h"

void UAnimNotify_ModifyAbility::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp->GetOwner();

	if (Owner)
	{
		UGameplaySystemComponent* GameplaySystem = IGameplaySystemOwnerInterface::Execute_GetGameplaySystemComponent(Owner);
		if(!GameplaySystem)
		{
			GS_LOG(Error, TEXT("No GameplaySystem found when trying to modify ability properties!"));
			return;
		}

		// Ensure that the ability we are modifying is the one that started the montage that triggered this AnimNotify.
		UGameplayAbility* AnimatingAbility = GameplaySystem->GetAnimatingAbility();
		if (!GameplaySystem->GetAnimMontageInfo()->IsActiveMontage(Cast<UAnimMontage>(Animation)))
		{
		
			GS_LOG(Warning, TEXT("AnimNotify_ModifyAbility was triggered by an animation that is not responsible for animating in AnimMontageInfo. No properties were modified."));
			return;
		}
		
		// Ensure we are not trying to modify the CDO
		if (AnimatingAbility->GetInstancingPolicy() == EInstancingPolicy::EIP_NoLifetime)
		{
			GS_LOG(Warning, TEXT("AnimNotify_ModifyAbility was triggered on a non-instanced Ability, and can not modify it. Ensure that %s is using the correct InstancingPolicy."), *AnimatingAbility->GetDisplayName());
			return;
		}

		// --- Modify Ability

		if (bEnableAbilityCancelling)
		{
			AnimatingAbility->SetIsCancellable(true);
		}
		

	}
}

#if WITH_EDITOR
bool UAnimNotify_ModifyAbility::ShouldFireInEditor()
{
	return false;
}
#endif
