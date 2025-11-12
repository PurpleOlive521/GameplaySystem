// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "AttributeDataSet.h"

void UAttributeDataSet::UseDefaultConfiguration()
{
	AddAttributeTypes(DefaultAttributeTypes);
}

void UAttributeDataSet::UseEnemyConfiguration()
{
	AddAttributeTypes(EnemyAttributeTypes);
}

void UAttributeDataSet::UsePlayerConfiguration()
{
	AddAttributeTypes(PlayerAttributeTypes);
}

void UAttributeDataSet::RemovePlayerOnlyAttributes()
{
	RemoveAttributeTypes(PlayerOnlyAttributeTypes);
}

void UAttributeDataSet::RemoveEnemyOnlyAttributes()
{
	RemoveAttributeTypes(EnemyOnlyAttributeTypes);
}

// Contains all currently available types
const TArray<EAttributeType> UAttributeDataSet::DefaultAttributeTypes = {
	EAttributeType::EAT_Health,
	EAttributeType::EAT_MaxHealth,
	EAttributeType::EAT_Charge,
	EAttributeType::EAT_MaxCharge,
	EAttributeType::EAT_Damage,
	EAttributeType::EAT_AppliedCharge,
	EAttributeType::EAT_RecoveryTime,
	EAttributeType::EAT_RecoveryDelay,
	EAttributeType::EAT_NormalWeakness,
	EAttributeType::EAT_InfraredWeakness,
	EAttributeType::EAT_UltravioletWeakness,
	EAttributeType::EAT_GammaWeakness,
	EAttributeType::EAT_DamageReduction,
	EAttributeType::EAT_MovementSpeed,
	EAttributeType::EAT_AttackSpeed,
	EAttributeType::EAT_StaggerThreshold,
	EAttributeType::EAT_OverchargedDamageMultiplier,
	EAttributeType::EAT_Energy,
	EAttributeType::EAT_MaxEnergy,
};

// All types that enemies are required to use
const TArray<EAttributeType> UAttributeDataSet::EnemyAttributeTypes = {
	EAttributeType::EAT_Health,
	EAttributeType::EAT_MaxHealth,
	EAttributeType::EAT_Charge,
	EAttributeType::EAT_MaxCharge,
	EAttributeType::EAT_Damage,
	EAttributeType::EAT_AppliedCharge,
	EAttributeType::EAT_RecoveryTime,
	EAttributeType::EAT_RecoveryDelay,
	EAttributeType::EAT_NormalWeakness,
	EAttributeType::EAT_InfraredWeakness,
	EAttributeType::EAT_UltravioletWeakness,
	EAttributeType::EAT_GammaWeakness,
	EAttributeType::EAT_DamageReduction,
	EAttributeType::EAT_MovementSpeed,
	EAttributeType::EAT_AttackSpeed,
	EAttributeType::EAT_StaggerThreshold,
	EAttributeType::EAT_OverchargedDamageMultiplier,
};

// All types that the player is required to use
const TArray<EAttributeType> UAttributeDataSet::PlayerAttributeTypes = {
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
	EAttributeType::EAT_OverchargedDamageMultiplier,
};

// Player unique types
const TArray<EAttributeType> UAttributeDataSet::PlayerOnlyAttributeTypes = {
	EAttributeType::EAT_Energy,
	EAttributeType::EAT_MaxEnergy,
};

// Enemy unique types
const TArray<EAttributeType> UAttributeDataSet::EnemyOnlyAttributeTypes = {
	EAttributeType::EAT_Charge,
	EAttributeType::EAT_MaxCharge,
	EAttributeType::EAT_RecoveryTime,
	EAttributeType::EAT_RecoveryDelay,
};

