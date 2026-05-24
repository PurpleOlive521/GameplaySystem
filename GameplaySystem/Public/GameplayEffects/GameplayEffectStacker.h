// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplaySystemTypes.h"
#include "GameplayEffectTypes.h"
#include "GameplayEffectStacker.generated.h"

class UGameplayEffect;
class UGameplaySystemComponent;
struct FActiveGameplayEffect;

#define EXPAND_STACKER_PARAMS(ParamsToExpand, GameplaySystemPropertyName, ActiveEffectPropertyName, GameplayEffectPropertyName)		\
UGameplaySystemComponent* GameplaySystem = ParamsToExpand.GameplaySystem;															\
FActiveGameplayEffect* ActiveEffect = ParamsToExpand.ActiveGameplayEffect;															\
const UGameplayEffect* GameplayEffect = ParamsToExpand.GameplayEffect;																\

struct FGameplayEffectStackerParams
{
	FGameplayEffectStackerParams(const UGameplayEffect* InGameplayEffect, FActiveGameplayEffect* InActiveGameplayEffect, UGameplaySystemComponent* InComponent)
		: GameplayEffect(InGameplayEffect), ActiveGameplayEffect(InActiveGameplayEffect), GameplaySystem(InComponent)
	{
	};

	const UGameplayEffect* GameplayEffect = nullptr;

	FActiveGameplayEffect* ActiveGameplayEffect = nullptr;

	UGameplaySystemComponent* GameplaySystem = nullptr;
};

/*
* Customises the behaviour when receiving StackModifiers and how multiple stacks behave when applied.
* Allows for changing incoming StackModifiers, and defining how stacking GameplayEffects are handled.
*/
UCLASS(Blueprintable, EditInlineNew)
class GAMEPLAYSYSTEM_API UGameplayEffectStacker : public UObject
{
	GENERATED_BODY()

public:
	UGameplayEffectStacker() = default;

	// --- Default implementations - Always called after all module overrides are processed

	static void ApplyStackModifier(const FGameplayEffectStackerParams& Params, const FGameplayEffectStackModifier& StackModifier);

	// --- Module overrides

	// Intentionally not const so we can modify StackModifier before passing it on
	virtual void PreApplyStackModifier(const FGameplayEffectStackerParams& Params, FGameplayEffectStackModifier& StackModifier) const {};

	virtual void PostApplyStackModifier(const FGameplayEffectStackerParams& Params, const FGameplayEffectStackModifier& StackModifier) const {};
};