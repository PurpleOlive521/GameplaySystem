// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectCondition.generated.h"

class UGameplayEffect;
class UGameplaySystemComponent;

#define EXPAND_CONDITION_PARAMS(ParamsToExpand, GameplaySystemPropertyName, GameplayEffectPropertyName)	\
UGameplaySystemComponent* GameplaySystem = ParamsToExpand.GameplaySystem;															\
const UGameplayEffect* GameplayEffect = ParamsToExpand.GameplayEffect;																\

struct FGameplayEffectConditionParams
{
	FGameplayEffectConditionParams(const UGameplayEffect* InGameplayEffect, UGameplaySystemComponent* InComponent)
		: GameplayEffect(InGameplayEffect), GameplaySystem(InComponent) {};

	const UGameplayEffect* GameplayEffect = nullptr;

	UGameplaySystemComponent* GameplaySystem = nullptr;
};

/*
* Custom requirements that needs to pass for a GameplayEffect to be applied.
*/
UCLASS(Blueprintable, EditInlineNew)
class GAMEPLAYSYSTEM_API UGameplayEffectCondition : public UObject
{
	GENERATED_BODY()

public:
	UGameplayEffectCondition() = default;

	virtual bool CanApply(const FGameplayEffectConditionParams& Params) const { return true; };

	virtual bool CanReapply(const FGameplayEffectConditionParams& Params) const { return true; };

	// TODO: Add option for conditional removals too?
};