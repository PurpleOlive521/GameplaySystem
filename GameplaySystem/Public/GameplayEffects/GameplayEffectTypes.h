// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeTypes.h"
#include "GameplayEffectTypes.generated.h"

class UGameplayEffect;

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

UENUM(BlueprintType)
enum class EGameplayEffectStage : uint8
{
	EGES_Apply			UMETA(DisplayName = "Apply"),
	EGES_Reapply		UMETA(DisplayName = "Reapply"),
	EGES_Remove			UMETA(DisplayName = "Remove"),
};

UENUM(BlueprintType)
enum class EStackingPolicy : uint8
{
	ESP_NoStacking		UMETA(DisplayName = "No Stacking"),
	ESP_CanStack		UMETA(DisplayName = "Can Stack"),
};

UENUM(BlueprintType)
enum class EStackingDurationRule : uint8
{
	// Applying a stack does not affect the Duration of the GameplayEffect.
	ESD_NoEffect					UMETA(DisplayName = "No Effect"),

	// Adding a stack will reset the Duration of the GameplayEffect.
	ESD_AddResetsDuration			UMETA(DisplayName = "Add Resets Duration"),

	// Removing a stack will reset the Duration of the GameplayEffect.
	ESD_RemoveResetsDuration		UMETA(DisplayName = "Remove Resets Duration"),

	// Adding and removing stacks wil reset the Duration of the GameplayEffect.
	ESD_AddAndRemoveResetsDuration	UMETA(DisplayName = "Add And Remove Resets Duration")
};

UENUM(BlueprintType)
enum class EStackingPeriodRule : uint8
{
	// Adding or removing stacks does not affect the Period of the GameplayEffect.
	ESP_NoEffect					UMETA(DisplayName = "No Effect"),

	// Adding a stack will reset the Period of the GameplayEffect.
	ESP_AddResetsPeriod				UMETA(DisplayName = "Add Resets Period"),

	// Removing a stack will reset the Period of the GameplayEffect.
	ESP_RemoveResetsPeriod			UMETA(DisplayName = "Remove Resets Period"),

	// Adding and removing stacks wil reset the Period of the GameplayEffect.
	ESP_AddAndRemoveResetsPeriod	UMETA(DisplayName = "Add And Remove Resets Period")
};

UENUM(BlueprintType)
enum class EStackingExpirationRule : uint8
{
	// All stacks of the GameplayEffect are removed when it expires.
	ESP_RemoveAll			UMETA(DisplayName = "Remove All"),

	// A single stack is removed when it expires, only removing the GameplayEffect when all stacks are removed.
	ESP_RemoveSingleStack	UMETA(DisplayName = "Remove Single Stack"),
};

UENUM(BlueprintType)
enum class EStackProgressDurationType : uint8
{
	// Stack Progress is never removed over time.
	ESP_Infinite			UMETA(DisplayName = "Infinite"),

	// Stack Progress Duration is used to determine when all applied progress is removed.
	ESP_Duration			UMETA(DisplayName = "Duration"),
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

// Modifier to a GameplayEffects stack properties.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayEffectStackModifier
{
	GENERATED_BODY();

	FGameplayEffectStackModifier() = default;

	FGameplayEffectStackModifier(bool bAddGameplayEffectIfNotApplied, int32 Stack, float StackProgress);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bAddGameplayEffectIfNotApplied = false;

	// The amount of stacks to add or remove.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 Stack = 0; 

	// The progress towards adding a stack to add or remove.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float StackProgress = 0.0f;
};

// Helper struct to associate a GameplayEffectStackModifier with a type of GameplayEffect.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FAssociatedGameplayEffectStackModifier
{
	GENERATED_BODY();

	FAssociatedGameplayEffectStackModifier() = default;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayEffectStackModifier Modifier;
};


struct GAMEPLAYSYSTEM_API FGameplayEffectConstants
{
	// The GameplayEffect has no period, meaning it does not periodically apply it's effects.
	static const float NO_PERIOD;

	// The GameplayEffect has infinite duration, meaning it needs to be expliticly removed.
	static const float INFINITE_DURATION;

	// The GameplayEffect has no duration.
	static const float NO_DURATION;

	// The GameplayEffect has no duration tied to stack progress.
	static const float NO_STACK_PROGRESS_DURATION;

	static const int32 NO_MAX_STACKS;
};
