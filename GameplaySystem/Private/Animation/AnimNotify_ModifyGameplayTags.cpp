// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "AnimNotify_ModifyGameplayTags.h"

#include "GameplaySystemComponent.h"
#include "GameplaySystemOwnerInterface.h"


UAnimNotify_ModifyGameplayTags::UAnimNotify_ModifyGameplayTags()
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif //WITH_EDITORONLY_DATA
}

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