// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "AnimNotifyState_TimedGameplayTag.h"

#include "GameplaySystemComponent.h"
#include "GameplaySystemOwnerInterface.h"

void UAnimNotifyState_TimedGameplayTag::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AActor* Owner = MeshComp->GetOwner(); IGameplaySystemOwnerInterface* GameplaySystemInterface = Cast<IGameplaySystemOwnerInterface>(Owner))
	{
		GameplayTagSystem = GameplaySystemInterface->GetGameplayTagSystem();

		if (!GameplayTagSystem)
		{
			return;
		}

		for (const FGameplayTagModifier& TagModifier : TagModifiersOnBegin)
		{
			GameplayTagSystem->ModifyTagCount(TagModifier.Tag, TagModifier.Count);
		}
	}
}

void UAnimNotifyState_TimedGameplayTag::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (GameplayTagSystem)
	{
		if (bUndoOnEnd)
		{
			// Operate on Begin array and reverse
			for (const FGameplayTagModifier& TagModifier : TagModifiersOnBegin)
			{
				GameplayTagSystem->ModifyTagCount(TagModifier.Tag, -TagModifier.Count);
			}
		}
		else
		{
			// Operate on End array
			for (const FGameplayTagModifier& TagModifier : TagModifiersOnEnd)
			{
				GameplayTagSystem->ModifyTagCount(TagModifier.Tag, TagModifier.Count);
			}
		}
	}
}


#if WITH_EDITOR

bool UAnimNotifyState_TimedGameplayTag::ShouldFireInEditor()
{
	return false;
}

#endif // WITH_EDITOR
