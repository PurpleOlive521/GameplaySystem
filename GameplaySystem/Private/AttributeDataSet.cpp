// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "AttributeDataSet.h"

constexpr bool MARK_MODIFY_AS_DIRTY = true;

uint32 GetTypeHash(const FEditorAttribute& EditorAttribute)
{
	//TEnumAsByte< EAttributeType>(
	return GetTypeHash(EditorAttribute.Type);
}

void UAttributeDataSet::UseDefaultConfiguration()
{
	AddAttributeTypes(DefaultAttributeTypes);

	Modify(MARK_MODIFY_AS_DIRTY);
}

void UAttributeDataSet::UseEnemyConfiguration()
{
	AddAttributeTypes(DefaultAttributeTypes);
	AddAttributeTypes(EnemyOnlyAttributeTypes);

	Modify(MARK_MODIFY_AS_DIRTY);
}

void UAttributeDataSet::UsePlayerConfiguration()
{
	AddAttributeTypes(DefaultAttributeTypes);
	AddAttributeTypes(PlayerOnlyAttributeTypes);

	Modify(MARK_MODIFY_AS_DIRTY);
}

void UAttributeDataSet::RemovePlayerOnlyAttributes()
{
	RemoveAttributeTypes(PlayerOnlyAttributeTypes);

	Modify(MARK_MODIFY_AS_DIRTY);
}

void UAttributeDataSet::RemoveEnemyOnlyAttributes()
{
	RemoveAttributeTypes(EnemyOnlyAttributeTypes);

	Modify(MARK_MODIFY_AS_DIRTY);
}

// Contains a set of types that all combat-participating entities are expected to use.
const TArray<EAttributeType> UAttributeDataSet::DefaultAttributeTypes = {
	EAttributeType::EAT_Health,
	EAttributeType::EAT_MaxHealth,
	EAttributeType::EAT_Damage,
	EAttributeType::EAT_AppliedCharge,
	EAttributeType::EAT_StaggerDamage,
	EAttributeType::EAT_NormalWeakness,
	EAttributeType::EAT_InfraredWeakness,
	EAttributeType::EAT_UltravioletWeakness,
	EAttributeType::EAT_GammaWeakness,
	EAttributeType::EAT_DamageReduction,
	EAttributeType::EAT_MovementSpeed,
	EAttributeType::EAT_AttackSpeed,
	EAttributeType::EAT_StaggerThreshold,
	EAttributeType::EAT_AilmentImmunity,
	EAttributeType::EAT_AilmentResistance,
	EAttributeType::EAT_AilmentStrength,
	EAttributeType::EAT_TimeDilation,
};

// Player unique types
const TArray<EAttributeType> UAttributeDataSet::PlayerOnlyAttributeTypes = {
	EAttributeType::EAT_OverclockThreshold,
	EAttributeType::EAT_OverheatLimit,
	EAttributeType::EAT_InfraredOverheat,
	EAttributeType::EAT_UltravioletOverheat,
	EAttributeType::EAT_GammaOverheat,
	EAttributeType::EAT_AilmentStrength,
};

// Enemy unique types
const TArray<EAttributeType> UAttributeDataSet::EnemyOnlyAttributeTypes = {
	EAttributeType::EAT_Charge,
	EAttributeType::EAT_MaxCharge,
	EAttributeType::EAT_RecoveryTime,
	EAttributeType::EAT_RecoveryDelay,
	EAttributeType::EAT_OverchargedDamageMultiplier,
};

