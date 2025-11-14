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
	EPAT_ExecuteOnApplication	UMETA(DisplayName = "Execute Effects When Applied"),
	EPAT_ExecuteOnRemoval		UMETA(DisplayName = "Execute Effects When Removed")
};

struct GAMEPLAYSYSTEM_API FGameplayEffectConstants
{
	// The GameplayEffect has no period, meaning it does not periodically apply it's effects.
	static const float NO_PERIOD;

	// The GameplayEffect has no duration, meaning it needs to be expliticly removed.
	static const float INFINITE_DURATION;
	
};

/*
 * Class for creating Gameplay Effects.
 * FActiveGameplayEffect works as a mutable representation for applied instances this class
 */
UCLASS(Blueprintable)
class GAMEPLAYSYSTEM_API UGameplayEffect : public UObject
{
	GENERATED_BODY()

protected:
	
public:
	UGameplayEffect();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	static void GenerateGUID(FString& IdRef);

	// Call on the CDO to avoid needing to instantiate the class.
	// Needs to be called to apply the ActiveGameplayEffect!
	virtual bool ApplyGameplayEffect(UGameplaySystemComponent* GameplaySystem, AActor* Actor) const;

	// Call on the CDO to avoid needing to instantiate the class.
	// Allows us to do any work that is not in relation to the state of the GameplayEffect on removal.
	virtual bool RemoveGameplayEffect(UGameplaySystemComponent* GameplaySystem, AActor* Actor) const;

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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bIsUnique == true"))
	bool bOverwriteOnUnique = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay Effect|Utility", meta = (EditCondition = "false"))
	FString Id = "";

	// The Tags that this GameplayEffect has.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayTags")
	FGameplayTagContainer TagsOnEffect;

	// Applied on the targets GameplayTagSystem.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayTags")
	FGameplayTagModifierContainer TagModifierContainer;
};

/*
* Handle that tells us what GameplayEffect it represents, what Effects it applies and how we should treat it with lifetime events.
* Mutable to keep track of state.
* Derive to build your own GameplayEffect states.
*/
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FActiveGameplayEffect
{
	GENERATED_BODY()

	FActiveGameplayEffect();

	FActiveGameplayEffect(const UGameplayEffect* Def);

	FActiveGameplayEffect(const TSubclassOf<UGameplayEffect> Def)
		: FActiveGameplayEffect(Def ? Def->GetDefaultObject<UGameplayEffect>() : UGameplayEffect::StaticClass()->GetDefaultObject<UGameplayEffect>()) {};

	virtual ~FActiveGameplayEffect() = default;

	// Returns the CDO of the GameplayEffect this is representing, or nullptr if not set.
	UGameplayEffect* GetDefinition() const;

	// Updates the state and lifetime of the GameplayEffect. Updating with Deltatime makes it sensitive to time dialation, which timers are not.
	virtual void TickGameplayEffect(float DeltaTime);

	// Called when the GameplayEffect is removed from an Actor. Allows us to respond in accordance with the GameplayEffect's state.
	virtual bool OnGameplayEffectRemoved(UGameplaySystemComponent* GameplaySystem, AActor* Actor);

	// Returns true if the PeriodLength has been passed.
	bool IsPeriodPassed() const;

	// Returns true if the the GameplayEffect has expired, marking it for removal.
	bool IsExpired() const;
	
	// Gets the time remaining of the GameplayEffects life.
	float GetRemainingDuration() const;

	// Returns the index of the Effect, or INDEX_NONE if no Effect is found
	int GetAttributeEffect(FAttributeEffect Effect) const;

	// Returns a string representation of the GameplayEffectData
	FString ToString() const;

	// Starts ticking when applied to this Component
	float Lifetime = 0.0f;

	// Is reset when reapplied through Duration settings
	float TimeSinceLastApplication = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Name = "New Gameplay Effect";

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FAttributeEffect> AttributeEffects;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EDurationType DurationType = EDurationType::EDT_Instant;

	// 100.0f is 100% chance to apply, 0.0f is 0%
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (UIMin = 0, UIMax = 100, Units = "Percent"))
	float ChanceToApply = 100.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "DurationType == EDurationType::EDT_HasDuration"))
	float Duration = 0.0f;

	// 0.0 if none. The period of time between the Attribute Effects being applied or reapplied.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "DurationType != EDurationType::EDT_Instant"))
	float PeriodLength = 0.0f;

	// Specifies when we want the GameplayEffect to apply its effects.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (EditCondition = "PeriodLength != 0.0f", EditConditionHides))
	EPeriodApplicationType PeriodType = EPeriodApplicationType::EPAT_ExecuteOnApplication;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayTags")
	FGameplayTagContainer TagsOnEffect;

	// Applied on the targets GameplayTagSystem.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayTags")
	FGameplayTagModifierContainer TagModifierContainer;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIsUnique = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Utility", meta = (EditCondition = "false"))
	FString Id = "";

	TSubclassOf<UGameplayEffect> GameplayEffectDef;

	bool operator==(const FActiveGameplayEffect& Other) const
	{
		// Both are without Id 
		if (Id == "" && Other.Id == "")
		{
			const bool bIsEqual =
				GameplayEffectDef == Other.GameplayEffectDef &&
				Name == Other.Name &&
				AttributeEffects == Other.AttributeEffects &&
				DurationType == Other.DurationType &&
				Duration == Other.Duration &&
				ChanceToApply == Other.ChanceToApply &&
				PeriodLength == Other.PeriodLength &&
				PeriodType == Other.PeriodType &&
				TagsOnEffect == Other.TagsOnEffect &&
				bIsUnique == Other.bIsUnique;

			return bIsEqual;
		}

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

