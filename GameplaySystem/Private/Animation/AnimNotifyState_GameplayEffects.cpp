// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "Animation/AnimNotifyState_GameplayEffects.h"
#include "GameplaySystemComponent.h"

UAnimNotifyState_GameplayEffects::UAnimNotifyState_GameplayEffects()
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif //WITH_EDITORONLY_DATA
}

void UAnimNotifyState_GameplayEffects::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AActor* Owner = MeshComp->GetOwner(); UGameplaySystemComponent* GameplaySystem = UGameplaySystemComponent::GetGameplaySystemFromActor(Owner))
	{
		FGameplayEffectHandle Handle;
		for (const auto& GameplayEffect : EffectsOnBegin)
		{
			const bool bApplied = GameplaySystem->AddGameplayEffectFromType(GameplayEffect, Handle, Owner);
			if (bApplied)
			{
				AppliedEffects.Add(Handle);
			}
		}
	}
}

void UAnimNotifyState_GameplayEffects::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AActor* Owner = MeshComp->GetOwner(); UGameplaySystemComponent * GameplaySystem = UGameplaySystemComponent::GetGameplaySystemFromActor(Owner))
	{
		if (bRemoveAppliedBeginEffectsOnEnd)
		{
			GameplaySystem->RemoveGameplayEffectsByHandles(AppliedEffects);
			AppliedEffects.Empty();
		}

		FGameplayEffectHandle Handle;
		for (const auto& GameplayEffect : EffectsOnEnd)
		{
			const bool bApplied = GameplaySystem->AddGameplayEffectFromType(GameplayEffect, Handle, Owner);
			if (bApplied)
			{
				AppliedEffects.Add(Handle);
			}
		}
	}
}