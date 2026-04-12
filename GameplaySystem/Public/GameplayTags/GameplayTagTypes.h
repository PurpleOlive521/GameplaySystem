// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

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

	FGameplayTagModifier() = default;

	// The tag to modify with Count.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayTagModifier")
	FGameplayTag Tag;

	// The amount of tags to add/remove. Can be negative.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayTagModifier")
	int Count = 0;
};

UENUM(BlueprintType)
enum class ETagModifier : uint8
{
	ETM_Add			UMETA(DisplayName = "Add"),
	ETM_Remove		UMETA(DisplayName = "Remove"),
};

// A modifier to a GameplayTag that will either remove or add the tag when applied.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FSimpleGameplayTagModifier
{
	GENERATED_BODY()

	FSimpleGameplayTagModifier() = default;

	// The tag to modify.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SimpleGameplayTagModifier")
	FGameplayTag Tag;

	// The amount of tags to add/remove. Can be negative.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SimpleGameplayTagModifier")
	ETagModifier Modifier = ETagModifier::ETM_Add;
};

// Encapsulates a collection of GameplayTagModifiers.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayTagModifierContainer
{
	GENERATED_BODY()

	FGameplayTagModifierContainer() = default;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (TitleProperty = "Tag"), Category = "GameplayTagModifierContainer")
	TArray<FGameplayTagModifier> TagModifiers;

	void Apply(FGameplayTagSystem* GameplayTagSystem) const;

	// Inverts the modifier counts before applying. Use to undo the effects of Apply().
	void ReverseApply(FGameplayTagSystem* GameplayTagSystem) const;
};

#define HAS_TYPE_HASH false
#if HAS_TYPE_HASH // FGameplayTagQuery does not have a native GetTypeHash implementation, and can not be used for TMaps.

// Stores a collection of GameplayTagQueries and combines them into a single query for quick evaluation.
//USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayTagQueryContainer
{
//	GENERATED_BODY()

	FGameplayTagQueryContainer() = default;

	void AddQuery(const FGameplayTagQuery& TagQuery);

	void RemoveQuery(const FGameplayTagQuery& TagQuery);

	void ModifyQuery(const FGameplayTagQuery& TagQuery, int Delta);

	// A single Query that combines all contained queries. Quicker than iterating over all queries individually.
	FGameplayTagQuery GetCombinedQuery();
	
	// Regenerate the cached combined query for this container.
	void CombineQueries();

	TMap<FGameplayTagQuery, uint32> TagQueries;

private:
	uint32 bIsDirty : 1;

	FGameplayTagQuery CachedCombinedQuery;
};

#endif // HAS_TYPE_HASH
