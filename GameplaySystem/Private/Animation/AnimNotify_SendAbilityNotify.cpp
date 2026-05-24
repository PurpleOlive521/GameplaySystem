// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "Animation/AnimNotify_SendAbilityNotify.h"
#include "GameplaySystemComponent.h"
#include "DevelopmentTypes.h"

UAnimNotify_SendAbilityNotify::UAnimNotify_SendAbilityNotify()
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif //WITH_EDITORONLY_DATA
}

void UAnimNotify_SendAbilityNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp->GetOwner();

	if (Owner)
	{
		if (UGameplaySystemComponent* GameplaySystem = UGameplaySystemComponent::GetGameplaySystemFromActor(Owner))
		{
			// Ensure that the ability we are modifying is the one that started the montage that triggered this AnimNotify.
			const FName Group = UGameplaySystemComponent::GetGroupForAnimation(Animation);
			UGameplayAbility* AnimatingAbility = GameplaySystem->GetAnimatingAbility(Group);

			if (!AnimatingAbility)
			{
				GS_LOG(Warning, TEXT("UAnimNotify_SendAbilityNotify was triggered while no animating ability is active!"));
				return;
			}

			if (!GameplaySystem->GetAnimMontageInfo()->IsActiveMontage(Group, Animation))
			{

				GS_LOG(Warning, TEXT("UAnimNotify_SendAbilityNotify was triggered by an animation that is not responsible for animating in AnimMontageInfo."));
				return;
			}

			// Ensure that we do not send a notify to CDO
			if (AnimatingAbility->GetInstancingPolicy() == EInstancingPolicy::EIP_NoLifetime)
			{
				GS_LOG(Warning, TEXT("UAnimNotify_SendAbilityNotify was triggered on a non-instanced Ability. Ensure that %s is using the correct InstancingPolicy."), *AnimatingAbility->GetDisplayName());
				return;
			}

			AnimatingAbility->SendAbilityNotify(NotifyName);
		}
	}
}
