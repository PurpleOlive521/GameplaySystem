// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplaySystemTypes.h"
#include "GameplayEffectExecutor.generated.h"

class UGameplayEffect;
class UGameplaySystemComponent;
struct FActiveGameplayEffect;

#define EXPAND_EXECUTOR_PARAMS(ParamsToExpand, GameplaySystemPropertyName, ActiveEffectPropertyName, GameplayEffectPropertyName)	\
UGameplaySystemComponent* GameplaySystem = ParamsToExpand.GameplaySystem;															\
FActiveGameplayEffect* ActiveEffect = ParamsToExpand.ActiveGameplayEffect;															\
const UGameplayEffect* GameplayEffect = ParamsToExpand.GameplayEffect;																\

struct FGameplayEffectExecutorParams
{
	FGameplayEffectExecutorParams(const UGameplayEffect* InGameplayEffect, FActiveGameplayEffect* InActiveGameplayEffect, UGameplaySystemComponent* InComponent)
		: GameplayEffect(InGameplayEffect), ActiveGameplayEffect(InActiveGameplayEffect), GameplaySystem(InComponent)
	{
	};

	const UGameplayEffect* GameplayEffect = nullptr;

	FActiveGameplayEffect* ActiveGameplayEffect = nullptr;

	UGameplaySystemComponent* GameplaySystem = nullptr;
};

/*
* Customises the response to a GameplayEffects lifetime events.
* Allows for changing incoming modifiers. Can be used to allow or forbid certain changes, add new ones or change exisiting ones.
*/
UCLASS(Blueprintable, EditInlineNew)
class GAMEPLAYSYSTEM_API UGameplayEffectExecutor : public UObject
{
	GENERATED_BODY()

public:
	UGameplayEffectExecutor() = default;

	// --- Default implementations - Always called after all module overrides are processed

	static void Apply(FGameplayEffectExecutorParams Params);

	static void Reapply(FGameplayEffectExecutorParams Params);

	static void Remove(FGameplayEffectExecutorParams Params);

	// --- Module overrides

	virtual void PreApply(const FGameplayEffectExecutorParams& Params) const {};

	virtual void PreReapply(const FGameplayEffectExecutorParams& Params) const {};

	virtual void PreRemove(const FGameplayEffectExecutorParams& Params) const {};

	// Snapshots are taken before Pre-calls

	virtual void PostApply(const FGameplayEffectExecutorParams& Params, const FGameplaySystemSnapshot& Snapshot) const {};

	virtual void PostReapply(const FGameplayEffectExecutorParams& Params, const FGameplaySystemSnapshot& Snapshot) const {};

	virtual void PostRemove(const FGameplayEffectExecutorParams& Params, const FGameplaySystemSnapshot& Snapshot) const {};
};