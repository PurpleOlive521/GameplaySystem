// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "AnimNotifyState_TimedGameplayTag.h"

#include "GameplaySystemComponent.h"
#include "GameplaySystemOwnerInterface.h"

UAnimNotifyState_TimedGameplayTag::UAnimNotifyState_TimedGameplayTag()
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif //WITH_EDITORONLY_DATA
}

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