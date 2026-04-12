// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Attribute.h"
#include "AttributeEffect.generated.h"

// When calculating values for Attributes affected by Effects, its done in the order below, starting with Override and ending with Division
UENUM(BlueprintType)
enum class EEffectApplicationType : uint8
{
	EEAT_Override		UMETA(DisplayName = "Override"),
	EEAT_Addition		UMETA(DisplayName = "Addition"),
	EEAT_Multiplication	UMETA(DisplayName = "Multiplication"),
	EEAT_Division		UMETA(DisplayName = "Division"),
};

/*
	The property that the Bonus Value is applied to.
	The Base Value is the default value, unaffected by Effects such as buffs & debuffs, and the Current Value is with all effects applied. 
*/
UENUM(BlueprintType)
enum class ETargetValue : uint8
{
	ETV_BaseValue	UMETA(DisplayName = "Base Value"),
	ETV_CurrentValue	UMETA(DisplayName = "Current Value"),
};

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FAttributeEffect
{
	GENERATED_BODY()

public:
	FAttributeEffect() {}

	FAttributeEffect(EAttributeType Attribute, float Value, EEffectApplicationType ApplicationType, ETargetValue Target);

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EAttributeType Attribute = EAttributeType::EAT_Health;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Value = 0;

	// The change this AttributeEffect has applied
	float BonusValue = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EEffectApplicationType ApplicationType = EEffectApplicationType::EEAT_Addition;

	// The property that the Effect is performed on.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ETargetValue Target = ETargetValue::ETV_BaseValue;
	
	// Applies the effect to the Attribute. 
	bool ApplyAttributeEffect(FAttribute& AffectedAttribute, bool bResetBonusValue);

	// Removes the effect to the Attribute, reverting the effects this applied on application.
	bool RemoveAttributeEffect(FAttribute& AffectedAttribute);

	// Returns true if the other AttributeEffect is identical except in the Attribute it affects.
	bool IsSameModifier(const FAttributeEffect& Other) const
	{
		return
			Value == Other.Value &&
			ApplicationType == Other.ApplicationType &&
			Target == Other.Target;
	}

	// Compares if Other is an effectively a copy of this AttributeEffect.
	bool operator==(const FAttributeEffect& Other) const 
	{
		return 
			Attribute		== Other.Attribute &&
			Value			== Other.Value &&
			ApplicationType == Other.ApplicationType &&
			Target			== Other.Target;
	}

	// Formats the AttributeEffect as 'AttributeType: Value (ApplicationType)BonusValue | TargetValue'
	FString ToString() const;
};
