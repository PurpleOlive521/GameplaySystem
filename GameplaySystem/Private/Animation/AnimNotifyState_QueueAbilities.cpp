// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "AnimNotifyState_QueueAbilities.h"

#include "GameplaySystemComponent.h"
#include "GameplaySystemOwnerInterface.h"
#include "GameplayTagDefines.h"
#include "GameplayTagContainer.h"
#include "DevelopmentTypes.h"


UAnimNotifyState_QueueAbilities::UAnimNotifyState_QueueAbilities()
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif //WITH_EDITORONLY_DATA
}

void UAnimNotifyState_QueueAbilities::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AActor* Owner = MeshComp->GetOwner(); IGameplaySystemOwnerInterface* GameplaySystemInterface = Cast<IGameplaySystemOwnerInterface>(Owner))
	{
		GameplayTagSystem = GameplaySystemInterface->GetGameplayTagSystem();

		if (!GameplayTagSystem)
		{
			return;
		}

		GameplayTagSystem->ModifyTagCount(GAMEPLAYTAG_Status_BufferingAction, 1);
	}

	else
	{
		GS_LOG(Error, TEXT("No GameplaySystem found when trying to enable Ability Queuing!"));
	}
}

void UAnimNotifyState_QueueAbilities::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (TriggerPolicy == EQueueTriggers::EQT_AnimNotifyTick)
	{
		TryTriggerQueuedAbility(MeshComp);
	}

}

void UAnimNotifyState_QueueAbilities::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	// Disable buffering before trying following EQueueTriggers policy
	if (GameplayTagSystem)
	{
		GameplayTagSystem->ModifyTagCount(GAMEPLAYTAG_Status_BufferingAction, -1);
	}

	if (TriggerPolicy == EQueueTriggers::EQT_AnimNotifyEnd)
	{
		TryTriggerQueuedAbility(MeshComp);
	}

	Super::NotifyEnd(MeshComp, Animation, EventReference);
}

void UAnimNotifyState_QueueAbilities::TryTriggerQueuedAbility(USkeletalMeshComponent* MeshComp)
{
	if (AActor* Owner = MeshComp->GetOwner(); IGameplaySystemOwnerInterface * GameplaySystemInterface = Cast<IGameplaySystemOwnerInterface>(Owner))
	{
		UGameplaySystemComponent* GameplaySystemComponent = GameplaySystemInterface->Execute_GetGameplaySystemComponent(Owner);

		GameplaySystemComponent->ActivateQueuedAbility();
	}
	else
	{
		GS_LOG(Error, TEXT("No GameplaySystem found when trying to activate queued abilties!"));
	}
}

