// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "GameplayEffectTypes.h"
#include "GameplayEffectExecutor.h"
#include "GameplayEffectCondition.h"

#include "GameplayTagContainer.h"
#include "AttributeEffect.h"
#include "GameplayTagTypes.h"
#include "GameplaySystemTypes.h"
#include "GuidTag.h"

#include "GameplayEffect.generated.h"

class UGameplaySystemComponent;

/*
 * Class for creating Gameplay Effects.
 * Is immutable at run-time and should be used as a const asset. 
 * Can still perform work at life-time events, defined by the used GameplayEffectExecutor. Any state is stored in the associated ActiveGameplayEffect.
 */
UCLASS(Blueprintable)
class GAMEPLAYSYSTEM_API UGameplayEffect : public UObject
{
	GENERATED_BODY()
	
public:
	UGameplayEffect();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostEditImport() override;

	virtual void PostDuplicate(bool bDuplicateForPIE) override;

	// Tries applying the GameplayEffect on the GameplaySystem. 
	bool ApplyGameplayEffect(UGameplaySystemComponent* GameplaySystem, AActor* Actor, FActiveGameplayEffect& ActiveGameplayEffect, FGameplayEffectHandle& OutHandle) const;

	// Allows us to do any work that is not in relation to the state of the GameplayEffect on removal.
	bool RemoveGameplayEffect(UGameplaySystemComponent* GameplaySystem, AActor* Actor, FActiveGameplayEffect& ActiveGameplayEffect, const FGameplayEffectHandle& Handle) const;

	// Fires the Modules in order, finishing with the static per-Stage call.
	void FireExecutorPipeline(const FGameplayEffectExecutorParams& Params, EGameplayEffectStage Stage) const;

	// Fires the Modules in order. If any one fails, it returns false immediately. All must succeed to return true.
	bool FireConditionPipeline(const FGameplayEffectConditionParams& Params, EGameplayEffectStage Stage) const;
	
	// Returns true if the GameplayEffect is applied through Period settings. 
	bool IsAppliedOnTick() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString Name = "New Gameplay Effect";
	
	// 100.0f is 100% chance to apply, 0.0f is 0%
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 0, UIMax = 100, Units="Percent"));
	float ChanceToApply = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FAttributeEffect> AttributeEffects;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EDurationType DurationType = EDurationType::EDT_Instant;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "DurationType == EDurationType::EDT_HasDuration"))
	float Duration = FGameplayEffectConstants::NO_DURATION;

	// 0.0 if none. The period of time between the Attribute Effects being applied or reapplied.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "DurationType != EDurationType::EDT_Instant"))
	float PeriodLength = FGameplayEffectConstants::NO_PERIOD;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "PeriodLength != 0.0f", EditConditionHides))
	EPeriodApplicationType PeriodType = EPeriodApplicationType::EPAT_ExecuteOnApplication;

	// Only allows one instance of this GameplayEffect to exist in the same GameplaySystemComponent.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Uniqueness")
	bool bIsUnique = false;

	// Will replace the existing GameplayEffect if true, or fail to apply if false.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bIsUnique"), Category = "Uniqueness")
	bool bOverwriteOnUnique = false;

	// If true, any tracked modifications done by this GameplayEffect will be undone.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bUndoModifiersOnRemoval = true;

	UPROPERTY(VisibleDefaultsOnly, Category = "Uniqueness")
	FGuidTag Id;

	// Removes any GameplayEffects on the target GameplaySystem that match this query when this GameplayEffect is applied.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayTags")
	FGameplayTagQuery RemoveMatchingGameplayEffects;

	// The Tags that this GameplayEffect has.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayTags")
	FGameplayTagContainer TagsOnEffect;

	// Applied on the targets GameplayTagSystem.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayTags")
	FGameplayTagModifierContainer TagModifierContainer;

protected:

	// The classes used when responding to lifetime events. Called in the order they appear in.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "ModulePipeline")
	TArray<TObjectPtr<UGameplayEffectExecutor>> ExecutorModules;

	// The class used for custom activation requirements. Called in the order they appear in.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "ModulePipeline")
	TArray<TObjectPtr<UGameplayEffectCondition>> ConditionModules;
};

/*
* Mutable instance of a GameplayEffect, contains what Effects it applies and how we should treat it with lifetime events.
*/
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FActiveGameplayEffect
{
	GENERATED_BODY()

	FActiveGameplayEffect();

	FActiveGameplayEffect(const UGameplayEffect* Def);

	FActiveGameplayEffect(const TSubclassOf<UGameplayEffect> Def)
	: FActiveGameplayEffect(Def ? Def->GetDefaultObject<UGameplayEffect>() : UGameplayEffect::StaticClass()->GetDefaultObject<UGameplayEffect>()) {};

	// Returns the CDO of the GameplayEffect this is representing, or nullptr if not set.
	UGameplayEffect* GetDefinition() const;

	void Tick(float DeltaTime, UGameplaySystemComponent* GameplaySystem);

	// Removes any applied modifiers that were applied on the Actor.
	void RemoveAppliedModifiers(UGameplaySystemComponent* GameplaySystem, AActor* Actor);

	// Returns true if the PeriodLength has been passed.
	volatile bool IsPeriodPassed() const;

	// Returns true if the the GameplayEffect has expired, marking it for removal.
	bool IsExpired() const;
	
	// Gets the time remaining of the GameplayEffects life.
	float GetRemainingDuration() const;

	// Returns the index of the Effect, or INDEX_NONE if no Effect is found
	int GetAttributeEffect(FAttributeEffect Effect) const;

	// Returns a string representation of the GameplayEffectData
	FString ToString() const;

	float Lifetime = 0.0f;

	// Is reset when reapplied through Duration settings
	float TimeSinceLastApplication = 0.0f;

	TArray<FAttributeEffect> AttributeEffects;

	FGuidTag Id;

	TSubclassOf<UGameplayEffect> GameplayEffectDef;

	bool operator==(const FActiveGameplayEffect& Other) const
	{
		return Id == Other.Id;
	}
};

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayEffectHandle
{
	GENERATED_BODY()
	
	FGameplayEffectHandle() : Id(), bWasInitialized(true) {};
	
	FGameplayEffectHandle(FString Guid) : Id(Guid), bWasInitialized(true) {};

	void Regenerate()
	{
		Id = FGuidTag();

		bWasInitialized = true;
	}

	friend uint32 GetTypeHash(const FGameplayEffectHandle& InHandle)
	{
		return GetTypeHash(InHandle.Id);
	}

	bool operator==(const FGameplayEffectHandle& Other) const
	{
		return Id == Other.Id;
	}

	bool operator!=(const FGameplayEffectHandle& Other) const
	{
		return Id != Other.Id;
	}

private:

	FGuidTag Id = {};
	
	bool bWasInitialized = false;
};

