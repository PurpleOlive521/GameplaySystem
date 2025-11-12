// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#pragma once

#include "CoreMinimal.h"
#include "Attribute.generated.h"


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

	// Damage multiplier, 1.0 is no effect, 0.5 is half damage, 2.0 is double damage, etc.

	EAT_NormalWeakness			UMETA(DisplayName = "Normal Weakness"),
	EAT_InfraredWeakness		UMETA(DisplayName = "Infrared Weakness"),
	EAT_UltravioletWeakness		UMETA(DisplayName = "Ultraviolet Weakness"),
	EAT_GammaWeakness			UMETA(DisplayName = "Gamma Weakness"),

	// 0 to 100 value. 0 does nothing, 100 is the same as 100% which results in the damage being negated.
	EAT_DamageReduction			UMETA(DisplayName = "Damage Reduction"),

	EAT_MovementSpeed			UMETA(DisplayName = "Movement Speed"),
	EAT_AttackSpeed				UMETA(DisplayName = "Attack Speed"),

	EAT_StaggerThreshold		UMETA(DisplayName = "Stagger Threshold"),
	EAT_OverchargedDamageMultiplier UMETA(DisplayName = "Overcharged Damage Multiplier"),

	// Currently Punkbot only
	EAT_Energy					UMETA(DisplayName = "Energy"),
	EAT_MaxEnergy				UMETA(DisplayName = "Max Energy"),
};


USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FAttribute
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EAttributeType AttributeType = EAttributeType::EAT_Health;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float BaseValue = 0;

	// This value is volatile - for permanent changes, use BaseValue
	float CurrentValue = 0;

	FAttribute() {}

	FAttribute(EAttributeType Type, float BaseVal, float CurrentVal)
	{
		AttributeType = Type;
		BaseValue = BaseVal;
		CurrentValue = CurrentVal;
	}

	// Formats the FAttribute as 'Type: BaseValue | CurrentValue'
	FString ToString() const;

	// Formats the FAttribute into an array of FStrings as ['Type', 'BaseValue', 'CurrentValue']
	void ToStringArray(TArray<FString>& OutArray) const;
};
