// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "Animation/AnimNotify_ModifyAttributes.h"
#include "GameplaySystemComponent.h"
#include "GameplaySystemOwnerInterface.h"
#include "DevelopmentTypes.h"

UAnimNotify_ModifyAttributes::UAnimNotify_ModifyAttributes()
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif //WITH_EDITORONLY_DATA
}

void UAnimNotify_ModifyAttributes::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp->GetOwner();

	if (Owner)
	{
		UGameplaySystemComponent* GameplaySystem = IGameplaySystemOwnerInterface::Execute_GetGameplaySystemComponent(Owner);
		if (!GameplaySystem)
		{
			GS_LOG(Error, TEXT("No GameplaySystem found when trying to modify ability properties!"));
			return;
		}

		for (auto Modifier : AttributeModifiers)
		{
			GameplaySystem->ApplyAttributeEffect(Modifier, EDurationType::EDT_Instant);
		}
	}
}