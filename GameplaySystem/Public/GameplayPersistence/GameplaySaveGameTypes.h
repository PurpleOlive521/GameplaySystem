// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "AttributeEffect.h"
#include "GameplayTagContainer.h"
#include "GameplayEffect.h"
#include "StructValidityKey.h"

#include "GameplaySaveGameTypes.generated.h"

enum class EAttributeType: uint8;

USTRUCT(BlueprintType)
struct FGameplayTagSystemSaveObject
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FStructValidityKey ValidityKey;

	// DONE
	UPROPERTY(BlueprintReadWrite)
	FGameplayTagContainer GameplayTags;

	// DONE
	TMap<FGameplayTag, int> TagCountTable;
};

USTRUCT(BlueprintType)
struct FGameplaySystemSaveObject
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FStructValidityKey ValidityKey;

	// DONE
	UPROPERTY(BlueprintReadWrite)
	TMap<EAttributeType, FAttribute> Attributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FAttributeEffect> ActiveEffects;

	// DONE
	UPROPERTY(BlueprintReadWrite)
	int EntityLevel = 0;

	// DONE
	UPROPERTY(BlueprintReadWrite)
	float Experience = 0;

	UPROPERTY(BlueprintReadWrite)
	TMap<FGameplayEffectHandle, FActiveGameplayEffect> ActiveGameplayEffects;

	// DONE
	FGameplayTagSystemSaveObject GameplayTags;
};
