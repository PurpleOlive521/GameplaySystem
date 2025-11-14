// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "AnimNotify_EndAbilityEarly.h"

#include "GameplaySystemComponent.h"
#include "GameplaySystemOwnerInterface.h"
#include "DevelopmentTypes.h"

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

		// We will start by just applying the ending modifiers, since thats less intrusive on the ability's lifetime.
		// If this doesn't work we will have to fully end the ability with EndAbilityEarly.
		AnimatingAbility->TryApplyAbilityEndedModifiers();
	}
}

#if WITH_EDITOR
bool UAnimNotify_EndAbilityEarly::ShouldFireInEditor()
{
	return false;
}
#endif