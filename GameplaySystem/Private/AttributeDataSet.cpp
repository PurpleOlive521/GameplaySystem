// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


#include "AttributeDataSet.h"

uint32 GetTypeHash(const FEditorAttribute& EditorAttribute)
{
	//TEnumAsByte< EAttributeType>(
	return GetTypeHash(EditorAttribute.Type);
}

void UAttributeDataSet::UseDefaultConfiguration()
{
	AddAttributeTypes(DefaultAttributeTypes);
}

void UAttributeDataSet::UseEnemyConfiguration()
{
	AddAttributeTypes(DefaultAttributeTypes);
	AddAttributeTypes(EnemyOnlyAttributeTypes);
}

void UAttributeDataSet::UsePlayerConfiguration()
{
	AddAttributeTypes(DefaultAttributeTypes);
	AddAttributeTypes(PlayerOnlyAttributeTypes);
}

void UAttributeDataSet::RemovePlayerOnlyAttributes()
{
	RemoveAttributeTypes(PlayerOnlyAttributeTypes);
}

void UAttributeDataSet::RemoveEnemyOnlyAttributes()
{
	RemoveAttributeTypes(EnemyOnlyAttributeTypes);
}

// Contains a set of types that all combat-participating entities are expected to use.
const TArray<EAttributeType> UAttributeDataSet::DefaultAttributeTypes = {
	EAttributeType::EAT_Health,
	EAttributeType::EAT_MaxHealth,
	EAttributeType::EAT_Damage,
	EAttributeType::EAT_AppliedCharge,
	EAttributeType::EAT_NormalWeakness,
	EAttributeType::EAT_InfraredWeakness,
	EAttributeType::EAT_UltravioletWeakness,
	EAttributeType::EAT_GammaWeakness,
	EAttributeType::EAT_DamageReduction,
	EAttributeType::EAT_MovementSpeed,
	EAttributeType::EAT_AttackSpeed,
	EAttributeType::EAT_StaggerThreshold,
};

// Player unique types
const TArray<EAttributeType> UAttributeDataSet::PlayerOnlyAttributeTypes = {
	EAttributeType::EAT_OverclockThreshold,
	EAttributeType::EAT_OverheatLimit,
	EAttributeType::EAT_InfraredOverheat,
	EAttributeType::EAT_UltravioletOverheat,
	EAttributeType::EAT_GammaOverheat,
};

// Enemy unique types
const TArray<EAttributeType> UAttributeDataSet::EnemyOnlyAttributeTypes = {
	EAttributeType::EAT_Charge,
	EAttributeType::EAT_MaxCharge,
	EAttributeType::EAT_RecoveryTime,
	EAttributeType::EAT_RecoveryDelay,
	EAttributeType::EAT_OverchargedDamageMultiplier,
};

