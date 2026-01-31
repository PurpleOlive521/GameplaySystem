// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeTypes.generated.h"

UENUM(BlueprintType)
enum class EAttributeType : uint8
{
	EAT_Health					UMETA(DisplayName = "Health"),
	EAT_MaxHealth				UMETA(DisplayName = "Max Health"),
	EAT_Charge					UMETA(DisplayName = "Charge"),
	EAT_MaxCharge				UMETA(DisplayName = "Max Charge"),
	EAT_Damage					UMETA(DisplayName = "Damage"),
	EAT_AppliedCharge			UMETA(DisplayName = "Applied Charge"),

	EAT_RecoveryTime			UMETA(DisplayName = "Recovery Time"),
	EAT_RecoveryDelay			UMETA(DisplayName = "Recovery Delay"),

	// Damage multipliers, 1.0 is no effect, 0.5 is 50% damage, 2.0 is 100% more damage.
	EAT_NormalWeakness			UMETA(DisplayName = "Normal Weakness"),
	EAT_InfraredWeakness		UMETA(DisplayName = "Infrared Weakness"),
	EAT_UltravioletWeakness		UMETA(DisplayName = "Ultraviolet Weakness"),
	EAT_GammaWeakness			UMETA(DisplayName = "Gamma Weakness"),

	// Percentage of damage that is negated, 0 (%) leaves value unaffected and 100 (%) negates the entire value. 
	EAT_DamageReduction			UMETA(DisplayName = "Damage Reduction"),

	EAT_MovementSpeed			UMETA(DisplayName = "Movement Speed"),
	EAT_AttackSpeed				UMETA(DisplayName = "Attack Speed"),

	EAT_StaggerThreshold		UMETA(DisplayName = "Stagger Threshold"),
	EAT_OverchargedDamageMultiplier UMETA(DisplayName = "Overcharged Damage Multiplier"),

	EAT_OverheatLimit			UMETA(DisplayName = "Overheat Limit"),

	// At what percentage of the Overheat Limit that the Overclock starts. E.g. a value of 80.0f gives 80% and up.
	EAT_OverclockThreshold		UMETA(DisplayName = "Overclock Threshold"),

	EAT_InfraredOverheat		UMETA(DisplayName = "Infrared Overheat"),
	EAT_UltravioletOverheat		UMETA(DisplayName = "Ultraviolet Overheat"),
	EAT_GammaOverheat			UMETA(DisplayName = "Gamma Overheat"),

	EAT_NONE
};
