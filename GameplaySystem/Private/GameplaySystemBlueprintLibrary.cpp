// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "GameplaySystemBlueprintLibrary.h"

#include "GameplayAbility.h"
#include "GameplaySystemComponent.h"
#include "GameplayTagDefines.h"

// --- Abilities

float UGameplaySystemBlueprintLibrary::GetGameplayAbilityCooldown(const FActiveGameplayAbility& ActiveGameplayAbility)
{
	return ActiveGameplayAbility.GetCurrentCooldown();
}

float UGameplaySystemBlueprintLibrary::GetGameplayAbilityDuration(const FActiveGameplayAbility& ActiveGameplayAbility)
{
	return ActiveGameplayAbility.GetCurrentDuration();
}

UGameplayAbility* UGameplaySystemBlueprintLibrary::GetGameplayAbilityInstance(const FActiveGameplayAbility& ActiveGameplayAbility)
{
	return ActiveGameplayAbility.GameplayAbility;
}

bool UGameplaySystemBlueprintLibrary::CheckGameplayAbilityHandle(const FActiveGameplayAbility& ActiveGameplayAbility)
{
	return IsValid(ActiveGameplayAbility.GameplayAbility);
}

// --- Ability Slots

bool UGameplaySystemBlueprintLibrary::ActivateAbility(const FGameplayAbilitySlot& Slot)
{
	return Slot.ActivateAbility();
}

void UGameplaySystemBlueprintLibrary::SetAbility(FGameplayAbilitySlot& Slot, TSubclassOf<UGameplayAbility> Ability)
{
	Slot.SetAbility(Ability);
}

bool UGameplaySystemBlueprintLibrary::GetSlot(const FGameplayAbilitySlotContainer& SlotContainer, const FGameplayTag& SlotTag, FGameplayAbilitySlot& OutSlot)
{
	return SlotContainer.GetSlot(SlotTag, OutSlot);
}

// --- Attributes

FString UGameplaySystemBlueprintLibrary::ConvertAttributeToDisplayName(EAttributeType Attribute)
{
	switch (Attribute)
	{
		case(EAttributeType::EAT_Health):
			return TEXT("Health");

		case(EAttributeType::EAT_MaxHealth):
			return TEXT("Max Health");

		case(EAttributeType::EAT_Charge):
			return TEXT("Charge");

		case(EAttributeType::EAT_MaxCharge):
			return TEXT("Max Charge");

		case(EAttributeType::EAT_Damage):
			return TEXT("Damage");

		case(EAttributeType::EAT_AppliedCharge):
			return TEXT("Applied Charge");

		case(EAttributeType::EAT_RecoveryTime):
			return TEXT("Recovery Time");

		case(EAttributeType::EAT_RecoveryDelay):
			return TEXT("Recovery Delay");

		case(EAttributeType::EAT_NormalWeakness):
			return TEXT("Normal Weakness");

		case(EAttributeType::EAT_InfraredWeakness):
			return TEXT("Infrared Weakness");

		case(EAttributeType::EAT_UltravioletWeakness):
			return TEXT("Ultraviolet Weakness");

		case(EAttributeType::EAT_GammaWeakness):
			return TEXT("Gamma Weakness");

		case(EAttributeType::EAT_DamageReduction):
			return TEXT("Damage Reduction");

		case(EAttributeType::EAT_MovementSpeed):
			return TEXT("Movement Speed");

		case(EAttributeType::EAT_AttackSpeed):
			return TEXT("Attack Speed");

		case(EAttributeType::EAT_StaggerThreshold):
			return TEXT("Stagger Threshold");

		case(EAttributeType::EAT_OverchargedDamageMultiplier):
			return TEXT("Overcharged Damage Multiplier");

		case(EAttributeType::EAT_Energy):
			return TEXT("Energy");
		
		case(EAttributeType::EAT_MaxEnergy):
			return TEXT("Max Energy");

		default:
			checkNoEntry();	// There should always be a corresponding type
			return FString();
	}
}

FString UGameplaySystemBlueprintLibrary::ConvertEffectApplicationTypeToDisplayName(EEffectApplicationType ApplicationType)
{
	switch (ApplicationType)
	{
		case EEffectApplicationType::EEAT_Addition:
			return TEXT("+");
		case EEffectApplicationType::EEAT_Multiplication:
			return TEXT("x");
		case EEffectApplicationType::EEAT_Division:
			return TEXT("/");
		case EEffectApplicationType::EEAT_Override:
			return TEXT("Override");
		default:
			checkNoEntry();	// There should always be a corresponding type
			return FString();
	}
}

FString UGameplaySystemBlueprintLibrary::ConvertTargetValueToDisplayName(ETargetValue TargetType)
{
	switch (TargetType)
	{
		case ETargetValue::ETV_BaseValue:
			return TEXT("Base Value");
		case ETargetValue::ETV_CurrentValue:
			return TEXT("Current Value");
		default:
			checkNoEntry(); // There should always be a corresponding type
			return FString();
	}
}

float UGameplaySystemBlueprintLibrary::GetHealthAsPercentage(UGameplaySystemComponent* GameplaySystem)
{
	check(GameplaySystem); // Make sure that a GameplaySystem is connected to the pin
	return GameplaySystem->GetAttributeValue(EAttributeType::EAT_Health) / GameplaySystem->GetAttributeValue(EAttributeType::EAT_MaxHealth);
}

float UGameplaySystemBlueprintLibrary::GetChargeAsPercentage(UGameplaySystemComponent* GameplaySystem)
{
	check(GameplaySystem); // Make sure that a GameplaySystem is connected to the pin
	return GameplaySystem->GetAttributeValue(EAttributeType::EAT_Charge) / GameplaySystem->GetAttributeValue(EAttributeType::EAT_MaxCharge);
}

float UGameplaySystemBlueprintLibrary::GetEnergyAsPercentage(UGameplaySystemComponent* GameplaySystem)
{
	check(GameplaySystem); // Make sure that a GameplaySystem is connected to the pin
	return GameplaySystem->GetAttributeValue(EAttributeType::EAT_Energy) / GameplaySystem->GetAttributeValue(EAttributeType::EAT_MaxEnergy);
}

FStructValidityKey UGameplaySystemBlueprintLibrary::MakeValidValidityKey()
{
	return FStructValidityKey(true);
}

bool UGameplaySystemBlueprintLibrary::CheckValidityKey(const FStructValidityKey& ValidityKey)
{
	return ValidityKey.IsValid();
}
