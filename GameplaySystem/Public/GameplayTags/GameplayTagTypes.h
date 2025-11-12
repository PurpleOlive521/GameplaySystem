// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "GameplayTagTypes.generated.h"

struct FGameplayTagSystem;

// A modifier to a GameplayTag that will modify the amount of that tag when applied.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayTagModifier
{
	GENERATED_BODY()

	FGameplayTagModifier() {};

	// The tag to modify with Count.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayTagModifier")
	FGameplayTag Tag;

	// The amount of tags to add/remove. Can be negative.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayTagModifier")
	int Count = 0;
};

// Encapsulates a collection of GameplayTagModifiers.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayTagModifierContainer
{
	GENERATED_BODY()

	FGameplayTagModifierContainer() {};
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (TitleProperty = "Tag"), Category = "GameplayTagModifierContainer")
	TArray<FGameplayTagModifier> TagModifiers;

	void Apply(FGameplayTagSystem* GameplayTagSystem) const;

	// Inverts the modifier counts before applying. Use to undo the effects of Apply().
	void ReverseApply(FGameplayTagSystem* GameplayTagSystem) const;
};