// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "AttributeEffect.h"

#include "DevelopmentTypes.h"
#include "GameplaySystemBlueprintLibrary.h"

FAttributeEffect::FAttributeEffect(EAttributeType AttributeType, float Value, EEffectApplicationType ApplicationType, ETargetValue Target)
{
	Attribute = AttributeType;
	this->Value = Value;
	this->ApplicationType = ApplicationType;
	this->Target = Target;
}

bool FAttributeEffect::ApplyAttributeEffect(FAttribute& AffectedAttribute, bool bResetBonusValue)
{
	if(AffectedAttribute.AttributeType != Attribute)
	{
		GS_LOG(Error, TEXT("AttributeEffect: Error - Affected Attribute Type does not match Effect Attribute Type"));
		return false;
	}

	float Magnitude = 0.0f;

	// Non-instant effects wan't this value replaced and not incremented.
	if (bResetBonusValue)
	{
		BonusValue = 0.0f;
	}

	switch (ApplicationType)
	{

	case EEffectApplicationType::EEAT_Addition:

		if (Target == ETargetValue::ETV_BaseValue) 
		{
			Magnitude = Value;

			AffectedAttribute.BaseValue += Magnitude;
			AffectedAttribute.CurrentValue += Magnitude;

		}
		else
		{
			Magnitude = Value;

			AffectedAttribute.CurrentValue += Magnitude;
		}

		break;

		// When multiplying and dividing, only the 'Bonus' or change in value is added. This is an intentional decision to avoid exponential growth, and to keep the system more predictable.
	case EEffectApplicationType::EEAT_Multiplication:

		if (Target == ETargetValue::ETV_BaseValue)
		{
			Magnitude = (AffectedAttribute.BaseValue * Value) - AffectedAttribute.BaseValue;

			AffectedAttribute.BaseValue += Magnitude;
			AffectedAttribute.CurrentValue += Magnitude;
		}
		else
		{
			Magnitude = (AffectedAttribute.CurrentValue * Value) - AffectedAttribute.CurrentValue;

			AffectedAttribute.CurrentValue += Magnitude;
		}

		break;

		// See comment on EEAT_Multiplication above
	case EEffectApplicationType::EEAT_Division:

		if (Target == ETargetValue::ETV_BaseValue)
		{
			Magnitude = AffectedAttribute.BaseValue - (AffectedAttribute.BaseValue / Value);

			AffectedAttribute.BaseValue -= Magnitude;
			AffectedAttribute.CurrentValue -= Magnitude;
		}
		else
		{
			Magnitude = AffectedAttribute.CurrentValue - (AffectedAttribute.CurrentValue / Value);

			AffectedAttribute.CurrentValue -= Magnitude;
		}

		break;

		// Replaces the value with the Effect value
	case EEffectApplicationType::EEAT_Override:

		if (Target == ETargetValue::ETV_BaseValue)
		{
			Magnitude = Value - AffectedAttribute.BaseValue;

			AffectedAttribute.BaseValue = Value;
			AffectedAttribute.CurrentValue = Value;
		}
		else
		{
			Magnitude = Value - AffectedAttribute.CurrentValue;
			AffectedAttribute.CurrentValue = Value;
		}

		break;

	default:
		checkNoEntry(); // Not supported yet.
		return false;
	}

	BonusValue += Magnitude;

	return true;
}

bool FAttributeEffect::RemoveAttributeEffect(FAttribute& AffectedAttribute)
{
	if (AffectedAttribute.AttributeType != Attribute)
	{
		GS_LOG(Error, TEXT("AttributeEffect: Error - Affected Attribute Type does not match Effect Attribute Type"));
		return false;
	}

	// Due to different ways of handling the calculation in regards to allowing changes to Base or Current Value, this is done on a case to case basis with hard-coded treatments
	// rather than a general approach to all different Effect Application Types
	switch (ApplicationType)
	{

	case EEffectApplicationType::EEAT_Addition:
		if (Target == ETargetValue::ETV_BaseValue)
		{
			AffectedAttribute.BaseValue -= BonusValue;
			AffectedAttribute.CurrentValue -= BonusValue;
		}
		else
		{
			AffectedAttribute.CurrentValue -= BonusValue;
		}

		break;

	case EEffectApplicationType::EEAT_Multiplication: //Fall through
	case EEffectApplicationType::EEAT_Division:
		if (Target == ETargetValue::ETV_BaseValue)
		{
			AffectedAttribute.BaseValue -= BonusValue;
			AffectedAttribute.CurrentValue -= BonusValue;
		}
		else
		{
			AffectedAttribute.CurrentValue -= BonusValue;
		}

		break;

	case EEffectApplicationType::EEAT_Override:
		if (Target == ETargetValue::ETV_BaseValue)
		{
			AffectedAttribute.BaseValue -= BonusValue;
			AffectedAttribute.CurrentValue -= BonusValue;
		}
		else
		{
			AffectedAttribute.CurrentValue -= BonusValue;
		}
		break;

	default:
		checkNoEntry(); // Not supported yet.
		return false;
	}

	return true;
}

FString FAttributeEffect::ToString() const
{
	FString Output = UGameplaySystemBlueprintLibrary::ConvertAttributeToDisplayName(Attribute);
	Output += TEXT(": ") + FString::Printf(TEXT("%.2f"), Value) + TEXT(" | ");
	Output += UGameplaySystemBlueprintLibrary::ConvertEffectApplicationTypeToDisplayName(ApplicationType) + TEXT(" | ");
	Output += FString::Printf(TEXT("%.2f"), BonusValue) + TEXT(" | ") + UGameplaySystemBlueprintLibrary::ConvertTargetValueToDisplayName(Target);
	return Output;
}
