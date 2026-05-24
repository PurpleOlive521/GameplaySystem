// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "AnimNotify_EndAbilityEarly.h"

#include "GameplaySystemComponent.h"
#include "GameplaySystemOwnerInterface.h"
#include "DevelopmentTypes.h"
#include "GameplayTagDefines.h"

UAnimNotify_EndAbilityEarly::UAnimNotify_EndAbilityEarly()
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif //WITH_EDITORONLY_DATA
}

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
		const FName Group = UGameplaySystemComponent::GetGroupForAnimation(Animation);
		UGameplayAbility* AnimatingAbility = GameplaySystem->GetAnimatingAbility(Group);

		if (not AnimatingAbility || not GameplaySystem->GetAnimMontageInfo()->IsActiveMontage(Group, Animation))
		{
			GS_LOG(Warning, TEXT("AnimNotify_EndAbilityEarly was triggered by an animation that is not responsible for animating in AnimMontageInfo."));
			return;
		}

		FGameplayAbilityHandle Handle = GameplaySystem->GetAbilityHandleFromInstance(AnimatingAbility);
		FActiveGameplayAbility* ActiveAbility = GameplaySystem->GetActiveAbilityFromHandle_Ptr(Handle);

		if (bCancelAbility)
		{
			GameplaySystem->CancelAbility(Handle, bIsAuthorativeCancel);
			return;
		}

		GS_LOG(Log, TEXT("EndAbility triggered on %s"), *AnimatingAbility->GetDisplayName());
		
		ensure(ActiveAbility);

		if (!ActiveAbility)
		{
			return;
		}

		FGameplayTagContainer& AbilityTags = ActiveAbility->AbilityTags;

		AbilityTags.AddTag(GAMEPLAYTAG_GameplayAbility_Status_ActionCancellable);
	}
}