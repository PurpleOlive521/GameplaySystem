// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "AttributeTypes.h"
#include "Attribute.generated.h"

UENUM(BlueprintType)
enum class EAttributeValue : uint8
{
	// The attribute unaffected by temporary modifiers like GameplayEffects.
	EAV_BaseValue			UMETA(DisplayName = "Base Value"),

	// The attribute with temporary modifiers like GameplayEffects applied.
	// Recalculated frequently so direct modifications to this value are transient.
	EAV_CurrentValue		UMETA(DisplayName = "Current Value"),
};

// Wrapper for stringifying a Attribute.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FAttributeString
{
	GENERATED_BODY()

	FAttributeString() = default;

	UPROPERTY(BlueprintReadWrite, Category = "AttributeString")
	FString Type = {};

	UPROPERTY(BlueprintReadWrite, Category = "AttributeString")
	FString BaseValue = {};

	UPROPERTY(BlueprintReadWrite, Category = "AttributeString")
	FString CurrentValue = {};
};

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FAttribute
{
	GENERATED_BODY()

	FAttribute() = default;

	FAttribute(EAttributeType Type, float BaseVal, float CurrentVal);

	// Formats the FAttribute as 'Type: BaseValue | CurrentValue'
	FString ToString() const;

	// Formats the FAttribute in the FAttributeString struct.
	FAttributeString ToStringStruct() const;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EAttributeType AttributeType = EAttributeType::EAT_Health;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float BaseValue = 0;

	// This value is volatile - for permanent changes, use BaseValue
	float CurrentValue = 0;
};
