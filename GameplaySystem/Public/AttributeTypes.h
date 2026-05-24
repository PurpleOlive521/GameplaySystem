// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeTypes.generated.h"

UENUM(BlueprintType)
enum class EAttributeValue : uint8
{
	// The attribute unaffected by temporary modifiers like GameplayEffects.
	EAV_BaseValue			UMETA(DisplayName = "Base Value"),

	// The attribute with temporary modifiers like GameplayEffects applied.
	// Recalculated frequently so direct modifications to this value are transient.
	EAV_CurrentValue		UMETA(DisplayName = "Current Value"),
};

UENUM(BlueprintType)
enum class EAttributeType : uint8
{
	EAT_Health						UMETA(DisplayName = "Health"),
	EAT_MaxHealth					UMETA(DisplayName = "Max Health"),
	EAT_Charge						UMETA(DisplayName = "Charge"),
	EAT_MaxCharge					UMETA(DisplayName = "Max Charge"),

	EAT_Damage						UMETA(DisplayName = "Damage"),
	EAT_AppliedCharge				UMETA(DisplayName = "Applied Charge"),
	EAT_StaggerDamage				UMETA(DisplayName = "Stagger Damage"),

	EAT_RecoveryTime				UMETA(DisplayName = "Recovery Time"),
	EAT_RecoveryDelay				UMETA(DisplayName = "Recovery Delay"),

	// Damage multipliers, 1.0 is no effect, 0.5 is 50% damage, 2.0 is 100% more damage.
	EAT_NormalWeakness				UMETA(DisplayName = "Normal Weakness"),
	EAT_InfraredWeakness			UMETA(DisplayName = "Infrared Weakness"),
	EAT_UltravioletWeakness			UMETA(DisplayName = "Ultraviolet Weakness"),
	EAT_GammaWeakness				UMETA(DisplayName = "Gamma Weakness"),

	// Percentage of damage that is negated, 0 (%) leaves value unaffected and 100 (%) negates the entire value. 
	EAT_DamageReduction				UMETA(DisplayName = "Damage Reduction"),

	EAT_MovementSpeed				UMETA(DisplayName = "Movement Speed"),
	EAT_AttackSpeed					UMETA(DisplayName = "Attack Speed"),

	EAT_StaggerThreshold			UMETA(DisplayName = "Stagger Threshold"),
	EAT_OverchargedDamageMultiplier UMETA(DisplayName = "Overcharged Damage Multiplier"),

	EAT_OverheatLimit				UMETA(DisplayName = "Overheat Limit"),

	// At what percentage of the Overheat Limit that the Overclock starts. E.g. a value of 80.0f gives 80% and up.
	EAT_OverclockThreshold			UMETA(DisplayName = "Overclock Threshold"),

	EAT_InfraredOverheat			UMETA(DisplayName = "Infrared Overheat"),
	EAT_UltravioletOverheat			UMETA(DisplayName = "Ultraviolet Overheat"),
	EAT_GammaOverheat				UMETA(DisplayName = "Gamma Overheat"),

	// Percent of Ailment buildup that is negated, 0 (%) leaves value unaffected and 100 (%) negates the entire value. 
	EAT_AilmentImmunity				UMETA(DisplayName = "Ailment Immunity"),

	// The additional buildup needed to apply any Ailment. Multiplier that applies to the StackProgressLimit of any applied GameplayEffect.
	EAT_AilmentResistance			UMETA(DisplayName = "Ailment Resistance"),

	// Multiplier to Ailment buildup afflicted on enemies.
	EAT_AilmentStrength				UMETA(DisplayName = "Ailment Strength"),

	EAT_TimeDilation				UMETA(DisplayName = "Time Dilation"),

	// Used as a placeholder or stand-in for other Attribute types. Prefered over using EAT_NONE as a valid-but-empty value.
	EAT_Template					UMETA(DisplayName = "Template"), 

	EAT_NONE
};

// Helper attribute groupings
namespace AttributeGroups
{
	// The Attributes we are interested in for overheat responses
	const TArray<EAttributeType> OverheatAttributes = {
		EAttributeType::EAT_InfraredOverheat,
		EAttributeType::EAT_UltravioletOverheat,
		EAttributeType::EAT_GammaOverheat
	};

	const TArray<EAttributeType> WeaknessAttributes = {
		EAttributeType::EAT_NormalWeakness,
		EAttributeType::EAT_InfraredWeakness,
		EAttributeType::EAT_UltravioletWeakness,
		EAttributeType::EAT_GammaWeakness,
	};
}
