// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.



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

		if (!AnimatingAbility)
		{
			GS_LOG(Warning, TEXT("AnimNotify_ModifyAbility was triggered while no animating ability is active!"));
			return;
		}

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

		FGameplayAbilityHandle Handle = GameplaySystem->GetAbilityHandleFromInstance(AnimatingAbility);
		FActiveGameplayAbility* ActiveAbility = GameplaySystem->GetActiveAbilityFromHandle_Ptr(Handle);

		ensure(ActiveAbility);

		if (!ActiveAbility)
		{
			return;
		}

		FGameplayTagContainer& AbilityTags = ActiveAbility->AbilityTags;

		for (const auto& TagModifier : TagModifiers)
		{
			switch (TagModifier.Modifier)
			{
				case ETagModifier::ETM_Add: 
				AbilityTags.AddTag(TagModifier.Tag);
				break;

				case ETagModifier::ETM_Remove:
				AbilityTags.RemoveTag(TagModifier.Tag, true);
				break;	
			}
		}
		
		AbilityTags.FillParentTags();
	}
}

#if WITH_EDITOR
bool UAnimNotify_ModifyAbility::ShouldFireInEditor()
{
	return false;
}
#endif
