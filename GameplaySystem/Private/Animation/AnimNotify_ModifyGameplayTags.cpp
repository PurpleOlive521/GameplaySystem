// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


#include "AnimNotify_ModifyGameplayTags.h"

#include "GameplaySystemComponent.h"
#include "GameplaySystemOwnerInterface.h"


void UAnimNotify_ModifyGameplayTags::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AActor* Owner = MeshComp->GetOwner(); IGameplaySystemOwnerInterface * GameplaySystemInterface = Cast<IGameplaySystemOwnerInterface>(Owner))
	{
		FGameplayTagSystem* GameplayTagSystem = GameplaySystemInterface->GetGameplayTagSystem();

		if (!GameplayTagSystem)
		{
			return;
		}

		for (const FGameplayTagModifier& TagModifier : TagModifiers)
		{
			GameplayTagSystem->ModifyTagCount(TagModifier.Tag, TagModifier.Count);
		}
	}
}


#if WITH_EDITOR

bool UAnimNotify_ModifyGameplayTags::ShouldFireInEditor()
{
	return false;
}

#endif // WITH_EDITOR
