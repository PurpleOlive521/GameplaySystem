// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplaySystemBlueprintLibrary.h"

#include "GameplayAbility.h"
#include "GameplaySystemComponent.h"
#include "GameplayTagDefines.h"

// --- Abilities

bool UGameplaySystemBlueprintLibrary::IsEqual(const FGameplayAbilityHandle& A, const FGameplayAbilityHandle& B)
{
	return A == B;
}

float UGameplaySystemBlueprintLibrary::GetGameplayAbilityCooldown(const FActiveGameplayAbility& ActiveGameplayAbility)
{
	return ActiveGameplayAbility.GetRemainingCooldown();
}

float UGameplaySystemBlueprintLibrary::GetGameplayAbilityDuration(const FActiveGameplayAbility& ActiveGameplayAbility)
{
	// TODO: Fix
	return 1.0f;
}

UGameplayAbility* UGameplaySystemBlueprintLibrary::GetGameplayAbilityInstance(const FActiveGameplayAbility& ActiveGameplayAbility)
{
	return ActiveGameplayAbility.Ability;
}

bool UGameplaySystemBlueprintLibrary::CheckGameplayAbilityHandle(const FActiveGameplayAbility& ActiveGameplayAbility)
{
	return IsValid(ActiveGameplayAbility.Ability) && ActiveGameplayAbility.Handle.IsValid();
}

// --- Ability Slots

bool UGameplaySystemBlueprintLibrary::ActivateAbility(FGameplayAbilitySlot& Slot)
{
	return Slot.ActivateAbility();
}

void UGameplaySystemBlueprintLibrary::SetAbility(FGameplayAbilitySlot& Slot, TSubclassOf<UGameplayAbility> Ability)
{
	Slot.SetAbility(Ability);
}

bool UGameplaySystemBlueprintLibrary::GetSlot(const FGameplayAbilitySlotContainer& SlotContainer, const FGameplayTag& SlotTag, FGameplayAbilitySlot& OutSlot)
{
	return SlotContainer.GetSlotRef(SlotTag, OutSlot);
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

		case(EAttributeType::EAT_OverclockThreshold):
			return TEXT("Overclock Threshold");

		case(EAttributeType::EAT_OverheatLimit):
			return TEXT("Overheat Limit");

		case(EAttributeType::EAT_InfraredOverheat):
			return TEXT("Infrared Overheat");

		case(EAttributeType::EAT_UltravioletOverheat):
			return TEXT("Ultraviolet Overheat");

		case(EAttributeType::EAT_GammaOverheat):
			return TEXT("Gamma Overheat");
		case(EAttributeType::EAT_NONE):
			return TEXT("NONE");

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
	check(GameplaySystem);

	const float Health = GameplaySystem->GetAttributeValue(EAttributeType::EAT_Health, EAttributeValue::EAV_CurrentValue);
	const float MaxHealth = GameplaySystem->GetAttributeValue(EAttributeType::EAT_MaxHealth, EAttributeValue::EAV_CurrentValue);

	return Health / MaxHealth;
}

float UGameplaySystemBlueprintLibrary::GetChargeAsPercentage(UGameplaySystemComponent* GameplaySystem)
{
	check(GameplaySystem);

	const float Charge = GameplaySystem->GetAttributeValue(EAttributeType::EAT_Charge, EAttributeValue::EAV_CurrentValue);
	const float MaxCharge = GameplaySystem->GetAttributeValue(EAttributeType::EAT_MaxCharge, EAttributeValue::EAV_CurrentValue);

	return Charge / MaxCharge;
}

FStructValidityKey UGameplaySystemBlueprintLibrary::MakeValidValidityKey()
{
	return FStructValidityKey(true);
}

bool UGameplaySystemBlueprintLibrary::CheckValidityKey(const FStructValidityKey& ValidityKey)
{
	return ValidityKey.IsValid();
}

float UGameplaySystemBlueprintLibrary::ModifyGlobalTimeDilation(const UObject* WorldContextObject, float Delta)
{
	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		AWorldSettings* WorldSettings = World->GetWorldSettings();
		if (WorldSettings)
		{
			const float OriginalValue = WorldSettings->TimeDilation;
			float NewValue = WorldSettings->SetTimeDilation(OriginalValue + Delta);

			return NewValue - OriginalValue;
		}

	}

	return 0.0f;
}

float UGameplaySystemBlueprintLibrary::ModifyCustomTimeDilation(const UObject* WorldContextObject, AActor* Actor, float Delta)
{
	if (!Actor)
	{
		UE_LOG(LogBlueprintUserMessages, Warning, TEXT("ModifyCustomTimeDilation: Actor must be valid to modify CustomTimeDilation!"));
		return 0.0f;
	}

	UWorld* const World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		const float OriginalValue = Actor->CustomTimeDilation;
		const float NewValue = FMath::Clamp(OriginalValue + Delta, 0.0f, 2.0f);
		Actor->CustomTimeDilation = NewValue;

		return NewValue;
	}

	return 0.0f;
}
