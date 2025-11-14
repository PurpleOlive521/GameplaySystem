// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#include "Attribute.h"
#include "GameplaySystemBlueprintLibrary.h"
#include "DevelopmentTypes.h"

FString FAttribute::ToString() const
{
	FString Output = UGameplaySystemBlueprintLibrary::ConvertAttributeToDisplayName(AttributeType);
	Output += TEXT(": ") + FString::Printf(TEXT("%.1f"), BaseValue) + TEXT("| ") + FString::Printf(TEXT("%.1f"), CurrentValue);
	return Output;
}

void FAttribute::ToStringArray(TArray<FString>& OutArray) const
{
	OutArray.SetNumZeroed(3, true);

	OutArray.EmplaceAt(0, UGameplaySystemBlueprintLibrary::ConvertAttributeToDisplayName(AttributeType));
	OutArray.EmplaceAt(1, FString::Printf(TEXT("%.1f"), BaseValue));
	OutArray.EmplaceAt(2, FString::Printf(TEXT("%.1f"), CurrentValue));
}

