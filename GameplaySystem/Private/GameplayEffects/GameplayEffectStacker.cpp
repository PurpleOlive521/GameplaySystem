// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayEffectStacker.h"

#include "GameplaySystemComponent.h"
#include "GameplayEffect.h"

void UGameplayEffectStacker::ApplyStackModifier(const FGameplayEffectStackerParams& Params, const FGameplayEffectStackModifier& StackModifier)
{
	EXPAND_STACKER_PARAMS(Params, GameplaySystem, ActiveEffect, GameplayEffect);

	ActiveEffect->ApplyStack(StackModifier.Stack, GameplaySystem);
	ActiveEffect->ApplyStackProgress(StackModifier.StackProgress, GameplaySystem);
}
