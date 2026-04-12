// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayEffectExecutor.h"

#include "GameplaySystemComponent.h"
#include "GameplayEffect.h"

void UGameplayEffectExecutor::Apply(FGameplayEffectExecutorParams Params)
{
	const UGameplayEffect* Effect = Params.GameplayEffect;
	FActiveGameplayEffect* ActiveEffect = Params.ActiveGameplayEffect;
	UGameplaySystemComponent* GameplaySystem = Params.GameplaySystem;

	const bool bApplyEffectNow = Effect->PeriodType == EPeriodApplicationType::EPAT_ExecuteOnApplication;
	if (bApplyEffectNow)
	{
		for (FAttributeEffect& AttributeEffect : ActiveEffect->AttributeEffects)
		{
			GameplaySystem->ApplyAttributeEffect(AttributeEffect, Effect->DurationType);
		}

		// TODO: We don't allow the GameplayTags to be modified yet? Restricts us from doing conditional removal/application of Tags per GameplayEffect instance
		Effect->TagModifierContainer.Apply(GameplaySystem->GetGameplayTagSystem());
	}
}

void UGameplayEffectExecutor::Reapply(FGameplayEffectExecutorParams Params)
{
	FActiveGameplayEffect* ActiveEffect = Params.ActiveGameplayEffect;
	UGameplaySystemComponent* GameplaySystem = Params.GameplaySystem;

	for (FAttributeEffect& AttributeEffect : ActiveEffect->AttributeEffects)
	{
		// Force non-reversible application since we can't track and undo our reapplications anyway
		GameplaySystem->ApplyAttributeEffect(AttributeEffect, EDurationType::EDT_Instant);
	}
}

void UGameplayEffectExecutor::Remove(FGameplayEffectExecutorParams Params)
{
	FActiveGameplayEffect* ActiveGameplayEffect = Params.ActiveGameplayEffect;
	UGameplaySystemComponent* GameplaySystem = Params.GameplaySystem;

	ActiveGameplayEffect->RemoveAppliedModifiers(GameplaySystem, GameplaySystem->GetOwner());
}