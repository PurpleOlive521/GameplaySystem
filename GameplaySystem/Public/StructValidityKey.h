// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "StructValidityKey.generated.h"

// Checks the passed variables validity key, and if invalid, returns from the current function immediately.
// Intended for save objects, to avoid loading invalid data.
#define CHECK_VALIDITY_EARLY_RETURN(SaveObjectVariable)		\
if (SaveObjectVariable.ValidityKey.IsValid() == false)		\
{															\
	return;													\
}															\

// Simple struct to hold a validity key for other structs, useful for save games to verify that the data is valid when null-states aren't desired.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FStructValidityKey
{
	GENERATED_BODY()

	FStructValidityKey() : bIsValid(false) {};

	FStructValidityKey(bool Validity) : bIsValid(Validity) {};

	bool IsValid() const
	{
		return bIsValid;
	}
	
	void MakeValid()
	{
		bIsValid = true;
	}

	void MakeInvalid()
	{
		bIsValid = false;
	}

	void ReverseValidate()
	{
		bIsValid = !bIsValid;
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsValid = false;
};