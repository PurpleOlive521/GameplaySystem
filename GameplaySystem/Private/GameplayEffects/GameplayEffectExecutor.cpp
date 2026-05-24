// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayEffectExecutor.h"

#include "GameplaySystemComponent.h"
#include "GameplayEffect.h"

void UGameplayEffectExecutor::Apply(const FGameplayEffectExecutorParams& Params)
{
	EXPAND_EXECUTOR_PARAMS(Params, GameplaySystem, ActiveEffect, GameplayEffect);

	if (ActiveEffect->IsFullyApplied())
	{
		const bool bApplyEffectNow = GameplayEffect->PeriodType == EPeriodApplicationType::EPAT_ExecuteOnApplication;
		if (bApplyEffectNow)
		{
			for (FAttributeEffect& AttributeEffect : ActiveEffect->AttributeEffects)
			{
				GameplaySystem->ApplyAttributeEffect(AttributeEffect, GameplayEffect->DurationType);
			}

			// TODO: We don't allow the GameplayTags to be modified yet? Restricts us from doing conditional removal/application of Tags per GameplayEffect instance
			GameplayEffect->TagModifierContainer.Apply(GameplaySystem->GetGameplayTagSystem());
		}
	}
}

void UGameplayEffectExecutor::Reapply(const FGameplayEffectExecutorParams& Params)
{
	EXPAND_EXECUTOR_PARAMS(Params, GameplaySystem, ActiveEffect, GameplayEffect);

	if (ActiveEffect->IsFullyApplied())
	{
		for (FAttributeEffect& AttributeEffect : ActiveEffect->AttributeEffects)
		{
			// Force non-reversible application since we can't track and undo our reapplications anyway
			GameplaySystem->ApplyAttributeEffect(AttributeEffect, EDurationType::EDT_Instant);
		}
	}
}

void UGameplayEffectExecutor::Remove(const FGameplayEffectExecutorParams& Params)
{
	EXPAND_EXECUTOR_PARAMS(Params, GameplaySystem, ActiveEffect, GameplayEffect);

	if (ActiveEffect->IsFullyApplied())
	{
		ActiveEffect->RemoveAppliedModifiers(GameplaySystem, GameplaySystem->GetOwner());
	}
}