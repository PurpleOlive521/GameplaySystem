// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "GameplayTagContainer.h"
#include "AttributeEffect.h"
#include "GameplayTagTypes.h"
#include "GuidTag.h"

#include "GameplayEffect.generated.h"

class UGameplaySystemComponent;

UENUM(BlueprintType)
enum class EDurationType : uint8
{
	EDT_Instant		UMETA(DisplayName = "Instant"),
	EDT_HasDuration	UMETA(DisplayName = "Has Duration"),
	EDT_Infinite	UMETA(DisplayName = "Infinite"),
};

UENUM(BlueprintType)
enum class EPeriodApplicationType : uint8
{
	EPAT_ExecuteOnApplication		UMETA(DisplayName = "Execute Effects When Applied"),
	EPAT_ReapplicationOnly			UMETA(DisplayName = "Reapplication Only"),
	EPAT_ExecuteOnRemoval			UMETA(DisplayName = "Execute Effects When Removed"),
};

// Might be used in more places in the future, but is currently only used for GEE's.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FCoefficientAttribute
{
	GENERATED_BODY();

	// Interpreted as a normal value if Attribute is NONE.
	UPROPERTY(EditAnywhere)
	float Coefficient = 1.0f;

	// The attribute we want to use.
	UPROPERTY(EditAnywhere)
	EAttributeType Attribute = EAttributeType::EAT_NONE;

	// The value of the given attribute that we multiply the coefficient with.
	UPROPERTY(EditAnywhere)
	EAttributeValue Target = EAttributeValue::EAV_BaseValue;
};

struct GAMEPLAYSYSTEM_API FGameplayEffectConstants
{
	// The GameplayEffect has no period, meaning it does not periodically apply it's effects.
	static const float NO_PERIOD;

	// The GameplayEffect has no duration, meaning it needs to be expliticly removed.
	static const float INFINITE_DURATION;
};

struct FGameplayEffectExecutorParams
{
	FGameplayEffectExecutorParams(const UGameplayEffect* InGameplayEffect, FActiveGameplayEffect* InActiveGameplayEffect, UGameplaySystemComponent* InComponent)
		: GameplayEffect(InGameplayEffect), ActiveGameplayEffect(InActiveGameplayEffect), GameplaySystem(InComponent) {};

	const UGameplayEffect* GameplayEffect = nullptr;

	FActiveGameplayEffect* ActiveGameplayEffect = nullptr;

	UGameplaySystemComponent* GameplaySystem = nullptr;
};

/*
* Customises the response to a GameplayEffects lifetime events. 
* Allows for replacing the default behaviour with custom per-property behaviour.
*/
UCLASS(Blueprintable)
class GAMEPLAYSYSTEM_API UGameplayEffectExecutor : public UObject
{
	GENERATED_BODY()

public:
	UGameplayEffectExecutor() = default;

	virtual void OnGameplayEffectApplied(FGameplayEffectExecutorParams Params, const TArray<FAttributeEffect>& EffectsToApply, const FGameplayTagModifierContainer& TagModifiers) const;

	virtual void OnGameplayEffectRemoved(FGameplayEffectExecutorParams Params) const;

	virtual void OnGameplayEffectReapplied(FGameplayEffectExecutorParams Params, const TArray<FAttributeEffect>& EffectsToReapply) const;

protected:
	void PerformDefaultApply(FGameplayEffectExecutorParams Params, const TArray<FAttributeEffect>& EffectsToApply, const FGameplayTagModifierContainer& TagModifiers) const;

	void PerformDefaultRemove(FGameplayEffectExecutorParams Params) const;

	void PerformDefaultReapply(FGameplayEffectExecutorParams Params, const TArray<FAttributeEffect>& EffectsToReapply) const;

private:
	void Apply_Internal(FGameplayEffectExecutorParams Params, const TArray<FAttributeEffect>& EffectsToApply, const FGameplayTagModifierContainer& TagModifiers) const;
};

/*
* Custom requirements that needs to pass for a GameplayEffect to be applied.
* Does not replace the base GameplayEffect behaviour.
*/
UCLASS(Blueprintable)
class GAMEPLAYSYSTEM_API UGameplayEffectApplicationRequirements : public UObject
{
	GENERATED_BODY()

public:
	UGameplayEffectApplicationRequirements() = default;

	virtual bool CanApply(const UGameplayEffect* GameplayEffect, UGameplaySystemComponent* Component) const { return true; };
};

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

	const UGameplayEffectExecutor* GetExecutorClass() const;

	const UGameplayEffectApplicationRequirements* GetActivationRequirementsClass() const;

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
	float Duration = 0;

	// 0.0 if none. The period of time between the Attribute Effects being applied or reapplied.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "DurationType != EDurationType::EDT_Instant"))
	float PeriodLength = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "PeriodLength != 0.0f", EditConditionHides))
	EPeriodApplicationType PeriodType = EPeriodApplicationType::EPAT_ExecuteOnApplication;

	// Only allows one instance of this GameplayEffect to exist in the same GameplaySystemComponent.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsUnique = false;

	// Will replace the existing GameplayEffect if true, or fail to apply if false.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bIsUnique"))
	bool bOverwriteOnUnique = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = false))
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

	// The class used when responding to lifetime events. Does not replace default behaviour, only adds onto it!
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Overrides")
	TSubclassOf<UGameplayEffectExecutor> ExecutorClass = nullptr;

	// The class used for custom activation requirements. Does not replace default behaviour, only adds onto it!
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Overrides")
	TSubclassOf<UGameplayEffectApplicationRequirements> ActivationRequirementsClass = nullptr;

private:
	
	void FillEmptyClasses();
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

