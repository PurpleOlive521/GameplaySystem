// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeTypes.h"
#include "GameplayEffectTypes.generated.h"

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

	// The GameplayEffect has infinite duration, meaning it needs to be expliticly removed.
	static const float INFINITE_DURATION;

	// The GameplayEffect has no duration.
	static const float NO_DURATION;
};
