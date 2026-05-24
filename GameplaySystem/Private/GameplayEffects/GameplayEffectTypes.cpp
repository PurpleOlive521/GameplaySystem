// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayEffectTypes.h"

const float FGameplayEffectConstants::INFINITE_DURATION = 0.0f;

const float FGameplayEffectConstants::NO_PERIOD = 0.0f;

const float FGameplayEffectConstants::NO_DURATION = 0.0f;

const float FGameplayEffectConstants::NO_STACK_PROGRESS_DURATION = 0.0f;

const int32 FGameplayEffectConstants::NO_MAX_STACKS = 0;

FGameplayEffectStackModifier::FGameplayEffectStackModifier(bool bInAddGameplayEffectIfNotApplied, int32 InStack, float InStackProgress)
{
	bAddGameplayEffectIfNotApplied = bInAddGameplayEffectIfNotApplied;
	Stack = InStack;
	StackProgress = InStackProgress;
}
