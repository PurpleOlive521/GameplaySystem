// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "Attribute.h"
#include "GameplaySystemBlueprintLibrary.h"
#include "DevelopmentTypes.h"

FAttribute::FAttribute(EAttributeType Type, float BaseVal, float CurrentVal)
{
	AttributeType = Type;
	BaseValue = BaseVal;
	CurrentValue = CurrentVal;
}

FString FAttribute::ToString() const
{
	FString Output = UGameplaySystemBlueprintLibrary::ConvertAttributeToDisplayName(AttributeType);
	Output += TEXT(": ") + FString::Printf(TEXT("%.1f"), BaseValue) + TEXT("| ") + FString::Printf(TEXT("%.1f"), CurrentValue);
	return Output;
}

FAttributeString FAttribute::ToStringStruct() const
{
	FAttributeString OutStruct = {};
	OutStruct.Type = UGameplaySystemBlueprintLibrary::ConvertAttributeToDisplayName(AttributeType);
	OutStruct.BaseValue = FString::Printf(TEXT("%.1f"), BaseValue);
	OutStruct.CurrentValue = FString::Printf(TEXT("%.1f"), CurrentValue);

	return OutStruct;
}

