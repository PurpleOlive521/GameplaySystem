// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "GameplayEffectTypes.h"
#include "GameplayEffectExecutor.h"
#include "GameplayEffectCondition.h"
#include "GameplayEffectStacker.h"

#include "GameplayTagContainer.h"
#include "AttributeEffect.h"
#include "GameplayTagTypes.h"
#include "GameplaySystemTypes.h"
#include "GuidTag.h"
#include "SaveableObjectInterface.h"
#include "GameplayEventHandle.h"

#include "GameplayEffect.generated.h"

class UGameplaySystemComponent;
class UGameplayEffectVisualsAsset;

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
	bool ApplyGameplayEffect(UGameplaySystemComponent* GameplaySystem, FActiveGameplayEffect& ActiveGameplayEffect, FGameplayEffectHandle& OutHandle) const;

	// Allows us to do any work that is not in relation to the state of the GameplayEffect on removal.
	bool RemoveGameplayEffect(UGameplaySystemComponent* GameplaySystem, FActiveGameplayEffect& ActiveGameplayEffect, const FGameplayEffectHandle& Handle) const;

	void ApplyGameplayEffectStackModifier(UGameplaySystemComponent* GameplaySystem, FActiveGameplayEffect& ActiveGameplayEffect, const FGameplayEffectStackModifier& Modifier) const;

	// Fires the Modules in order, finishing with the static per-Stage call.
	void FireExecutorPipeline(const FGameplayEffectExecutorParams& Params, EGameplayEffectStage Stage) const;

	// Fires the Modules in order. If any one fails, it returns false immediately. All must succeed to return true.
	bool FireConditionPipeline(const FGameplayEffectConditionParams& Params, EGameplayEffectStage Stage) const;

	void FireStackerPipeline(const FGameplayEffectStackerParams& Params, const FGameplayEffectStackModifier& StackModifier) const;
	
	// Returns true if the GameplayEffect is applied through Period settings. 
	bool IsAppliedOnTick() const;

	// Returns true if the GameplayEffect can stack.
	bool IsStackingEffect() const;

	// Returns true if Duration is used, and the value is above 0.
	bool HasDuration() const;

	// Returns true if StackProgressDuration is used, and the value is above 0.
	bool HasStackProgressDuration() const;

	// Returns true if Period is used, and the Period is above 0.
	bool HasPeriod() const;

	// Returns true if there is a Max Stack limit.
	bool HasMaxStacks() const;

	float EvaluateScalingCurve(float InTime) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffect")
	FString Name = "New Gameplay Effect";
	
	// 100.0f is 100% chance to apply, 0.0f is 0%
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (UIMin = 0, UIMax = 100, Units="Percent"), Category = "GameplayEffect");
	float ChanceToApply = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayEffect")
	TArray<FAttributeEffect> AttributeEffects;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffect")
	EDurationType DurationType = EDurationType::EDT_Instant;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "DurationType == EDurationType::EDT_HasDuration"), Category = "GameplayEffect")
	float Duration = FGameplayEffectConstants::NO_DURATION;

	// 0.0 if none. The period of time between the Attribute Effects being applied or reapplied.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "DurationType != EDurationType::EDT_Instant"), Category = "GameplayEffect")
	float PeriodLength = FGameplayEffectConstants::NO_PERIOD;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "PeriodLength != 0", EditConditionHides), Category = "GameplayEffect")
	EPeriodApplicationType PeriodType = EPeriodApplicationType::EPAT_ExecuteOnApplication;

	// Only allows one instance of this GameplayEffect to exist in the same GameplaySystemComponent.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffect|Uniqueness")
	bool bIsUnique = false;

	// Will replace the existing GameplayEffect if true, or fail to apply if false.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bIsUnique"), Category = "GameplayEffect|Uniqueness")
	bool bOverwriteOnUnique = false;

	UPROPERTY(VisibleDefaultsOnly, Category = "GameplayEffect|Uniqueness")
	FGuidTag Id;

	UPROPERTY(EditDefaultsOnly, Category = "GameplayEffect|Stacking")
	EStackingPolicy StackingPolicy = EStackingPolicy::ESP_NoStacking;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "StackingPolicy != EStackingPolicy::ESP_NoStacking"), Category = "GameplayEffect|Stacking")
	EStackProgressDurationType StackProgressDurationType = EStackProgressDurationType::ESP_Infinite;

	// Until a stack is applied, StackProgressDuration determines how long progress is kept before being removed. 
	// Does nothing once one or more stacks are applied, or if the GameplayEffect is not stacking.
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "StackingPolicy != EStackingPolicy::ESP_NoStacking"), Category = "GameplayEffect|Stacking")
	float StackProgressDuration = FGameplayEffectConstants::NO_STACK_PROGRESS_DURATION;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "StackingPolicy != EStackingPolicy::ESP_NoStacking"), Category = "GameplayEffect|Stacking")
	EStackingExpirationRule StackingExpirationRule = EStackingExpirationRule::ESP_RemoveAll;

	// Does nothing if the GameplayEffect does not have a finite Duration.
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "StackingPolicy != EStackingPolicy::ESP_NoStacking"), Category = "GameplayEffect|Stacking")
	EStackingDurationRule StackingDurationRule = EStackingDurationRule::ESD_NoEffect;

	// Does nothing if the GameplayEffect does not have a Period.
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "StackingPolicy != EStackingPolicy::ESP_NoStacking"), Category = "GameplayEffect|Stacking")
	EStackingPeriodRule StackingPeriodRule = EStackingPeriodRule::ESP_NoEffect;

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "StackingPolicy != EStackingPolicy::ESP_NoStacking"), Category = "GameplayEffect|Stacking")
	int32 MaxStacks = FGameplayEffectConstants::NO_MAX_STACKS;

	// Once progress reaches or passes this value, a Stack is applied and progress is reset.
	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "StackingPolicy != EStackingPolicy::ESP_NoStacking"), Category = "GameplayEffect|Stacking")
	float StackProgressLimit = 100.0f;

	// If true, any tracked modifications done by this GameplayEffect will be undone.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffect")
	bool bUndoModifiersOnRemoval = true;

	// Applies the value at the current duration point as a multiplier to the AttributeEffects this contains.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffect|Curves")
	TObjectPtr<UCurveFloat> ScalingCurve = nullptr;

	// Stretches the ScalingCurve to match the duration of the GameplayEffect. Does nothing if no duration is set or if it is infinite.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffect|Curves")
	bool bScaleToDuration = false;

	// Removes any GameplayEffects on the target GameplaySystem that match this query when this GameplayEffect is applied.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffect|GameplayTags")
	FGameplayTagQuery RemoveMatchingGameplayEffects;

	// The Tags that this GameplayEffect has.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffect|GameplayTags")
	FGameplayTagContainer TagsOnEffect;

	// Applied on the targets GameplayTagSystem.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffect|GameplayTags")
	FGameplayTagModifierContainer TagModifierContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffect|Visual")
	TObjectPtr<UGameplayEffectVisualsAsset> VisualsAsset = nullptr;

protected:

	// The classes used when responding to lifetime events. Called in the order they appear in.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "GameplayEffect|ModulePipeline")
	TArray<TObjectPtr<UGameplayEffectExecutor>> ExecutorModules;

	// The class used for custom activation requirements. Called in the order they appear in.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "GameplayEffect|ModulePipeline")
	TArray<TObjectPtr<UGameplayEffectCondition>> ConditionModules;

	// The class used for custom stacking behaviour. Called in the order they appear in.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "GameplayEffect|ModulePipeline")
	TArray<TObjectPtr<UGameplayEffectStacker>> StackerModules;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnStackChangedSignature, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStackProgressChangedSignature, float);

/*
* Mutable instance of a GameplayEffect, contains what Effects it applies and how we should treat it with lifetime events.
*/
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FActiveGameplayEffect
{
	GENERATED_BODY()

	FActiveGameplayEffect();

	FActiveGameplayEffect(const UGameplayEffect* Definition);

	FActiveGameplayEffect(const TSubclassOf<UGameplayEffect> Definition, AActor* Instigator);

	bool OnSerialize(FSaveGameArchive& Archive, bool bIsLoading);

	// Returns the CDO of the GameplayEffect this is representing, or nullptr if not set.
	UGameplayEffect* GetDefinition() const;

	void Tick(float DeltaTime, UGameplaySystemComponent* GameplaySystem);

	void ScaleModifiers(float LastScalar, float NewScalar, UGameplaySystemComponent* GameplaySystem);

	void ApplyStackProgress(float AddedStackProgress, UGameplaySystemComponent* GameplaySystem);

	void ApplyStack(int32 AddedStacks, UGameplaySystemComponent* GameplaySystem);

	// Removes any applied modifiers that were applied on the Actor.
	void RemoveAppliedModifiers(UGameplaySystemComponent* GameplaySystem, AActor* Actor);

	void SetStackProgressLimit(float Limit, UGameplaySystemComponent* GameplaySystem);

	// Returns true if the PeriodLength has been passed.
	bool IsPeriodPassed() const;

	// Returns true if the the GameplayEffect has expired, marking it for removal.
	bool IsExpired() const;
	
	// Gets the time remaining of the GameplayEffects life.
	float GetRemainingDuration() const;

	// Returns the index of the Effect, or INDEX_NONE if no Effect is found
	int GetAttributeEffect(FAttributeEffect Effect) const;

	// Returns false if the Effect is stacking and does not have one full stack applied, true otherwise.
	bool IsFullyApplied() const;

	// Indicates that any further StackProgress or Stacks will not be applied.
	// Always returns true if the Effect is not stacking.
	bool IsAtFullStacks() const;

	bool IsPendingRemove() const;

	// Returns true if expiration and Duration of the GameplayEffect is dictated by StackProgressDuration, and not the typical Duration property.
	bool IsStackProgressDurationInControl() const; 

	// Removal is assumed to be due to an external source if the duration is not elapsed.
	// Returns true if deemed external, false otherwise or if not pending remove.
	bool IsRemoveExternal() const;

	// Only has meaning if IsStackProgressDurationInControl is true.
	float GetRemainingStackProgressDuration() const;

	// Returns a string representation of the GameplayEffectData
	FString ToString() const;

	// The amount of time this GameplayEffect has been added to a GameplaySystem.
	UPROPERTY(SaveGame)
	float Lifetime = 0.0f;

	// Progress towards applying effect again.
	UPROPERTY(SaveGame)
	float PeriodCounter = 0.0f;

	// Progress towards duration running out.
	UPROPERTY(SaveGame)
	float DurationCounter = 0.0f;

	// Progress towards stack progress duration running out.
	UPROPERTY(SaveGame)
	float StackProgressDurationCounter = 0.0f;

	// For Stacking Effects, a value of 0 means we are not yet applied. AppliedStacks has no meaning for non-stacking effects.
	UPROPERTY(BlueprintReadOnly, SaveGame)
	int32 AppliedStacks = 0;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	float StackProgress = 0.0f;

	UPROPERTY(BlueprintReadOnly, SaveGame)
	float StackProgressLimit = 0.0f;

	UPROPERTY(SaveGame)
	TArray<FAttributeEffect> AttributeEffects;

	UPROPERTY(SaveGame)
	FGuidTag Id;

	// The type of GameplayEffect this ActiveGameplayEffect is representing.
	UPROPERTY(SaveGame)
	TSubclassOf<UGameplayEffect> Definition;

	// The Actor that applied this GameplayEffect
	UPROPERTY(SaveGame)
	TWeakObjectPtr<AActor> Instigator = nullptr;

	TArray<FGameplayEventHandle> ActivatedEvents;

	bool bIsPendingRemove = false;

	FOnStackChangedSignature OnStackChangedSignature;

	FOnStackProgressChangedSignature OnStackProgressChangedSignature;

	bool operator==(const FActiveGameplayEffect& Other) const
	{
		return Id == Other.Id;
	}

	bool operator!=(const FActiveGameplayEffect& Other) const
	{
		return Id != Other.Id;
	}
};

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayEffectHandle
{
	GENERATED_BODY()
	
	FGameplayEffectHandle() = default;
	
	FGameplayEffectHandle(FString Guid) : Id(Guid), bWasInitialized(true) {};

	static FGameplayEffectHandle CreateNew();

	void Regenerate();

	bool IsValid() const;

	friend uint32 GetTypeHash(const FGameplayEffectHandle& InHandle);

	bool operator==(const FGameplayEffectHandle& Other) const
	{
		return Id == Other.Id;
	}

	bool operator!=(const FGameplayEffectHandle& Other) const
	{
		return Id != Other.Id;
	}

	UPROPERTY(SaveGame)
	FGuidTag Id = {};
	
	UPROPERTY(SaveGame)
	bool bWasInitialized = false;
};

