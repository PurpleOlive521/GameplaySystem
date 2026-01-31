// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


#include "AnimNotify_EndAbilityEarly.h"

#include "GameplaySystemComponent.h"
#include "GameplaySystemOwnerInterface.h"
#include "DevelopmentTypes.h"
#include "GameplayTagDefines.h"

void UAnimNotify_EndAbilityEarly::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp->GetOwner();

	if (Owner)
	{
		UGameplaySystemComponent* GameplaySystem = IGameplaySystemOwnerInterface::Execute_GetGameplaySystemComponent(Owner);
		if (!GameplaySystem)
		{
			GS_LOG(Error, TEXT("No GameplaySystem found on AnimMontage actor."));
			return;
		}

		// Ensure that the ability we are modifying is the one that started the montage that triggered this AnimNotify.
		UGameplayAbility* AnimatingAbility = GameplaySystem->GetAnimatingAbility();
		if (!GameplaySystem->GetAnimMontageInfo()->IsActiveMontage(Cast<UAnimMontage>(Animation)))
		{
			GS_LOG(Warning, TEXT("AnimNotify_EndAbilityEarly was triggered by an animation that is not responsible for animating in AnimMontageInfo."));
			return;
		}

		GS_LOG(Log, TEXT("EndAbility triggered on %s"), *AnimatingAbility->GetDisplayName());

		FGameplayAbilityHandle Handle = GameplaySystem->GetAbilityHandleFromInstance(AnimatingAbility);
		FActiveGameplayAbility* ActiveAbility = GameplaySystem->GetActiveAbilityFromHandle_Ptr(Handle);
		
		ensure(ActiveAbility);

		if (!ActiveAbility)
		{
			return;
		}

		FGameplayTagContainer& AbilityTags = ActiveAbility->AbilityTags;

		// Add a Tag that indicates that we want to be cancelled by other PrimaryActions
		AbilityTags.AddTag(GAMEPLAYTAG_GameplayAbility_Status_PrimaryActionCancellable);
	}
}

#if WITH_EDITOR
bool UAnimNotify_EndAbilityEarly::ShouldFireInEditor()
{
	return false;
}
#endif