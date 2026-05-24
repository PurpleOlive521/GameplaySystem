// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayTagTypes.h"

#include "GameplayTagSystem.h"

void FGameplayTagModifierContainer::Apply(FGameplayTagSystem* GameplayTagSystem) const
{
	// Apply the GameplayTag modifiers
	if (GameplayTagSystem)
	{
		for (const FGameplayTagModifier& TagModifier : TagModifiers)
		{
			GameplayTagSystem->ModifyTagCount(TagModifier.Tag, TagModifier.Count);
		}
	}
}

void FGameplayTagModifierContainer::ReverseApply(FGameplayTagSystem* GameplayTagSystem) const
{
	// Apply the GameplayTag modifiers
	if (GameplayTagSystem)
	{
		for (const FGameplayTagModifier& TagModifier : TagModifiers) 
		{
			GameplayTagSystem->ModifyTagCount(TagModifier.Tag, -TagModifier.Count);
		}
	}
}

#if HAS_TYPE_HASH // FGameplayTagQuery does not have a native GetTypeHash implementation, and can not be used for TMaps

void FGameplayTagQueryContainer::AddQuery(const FGameplayTagQuery& TagQuery)
{
	ModifyQuery(TagQuery, 1);
}

void FGameplayTagQueryContainer::RemoveQuery(const FGameplayTagQuery& TagQuery)
{
	ModifyQuery(TagQuery, -1);
}

void FGameplayTagQueryContainer::ModifyQuery(const FGameplayTagQuery& TagQuery, int Delta)
{
	if (!TagQueries.Contains(TagQuery))
	{
		bIsDirty = true;
	}

	uint32& Value = TagQueries.FindOrAdd(TagQuery);

	if (Delta < 0)
	{
		uint32 AbsDelta = FMath::Abs(Delta);

		// 0 or negative count
		if (AbsDelta >= Value)
		{
			TagQueries.Remove(TagQuery);
			bIsDirty = true;
			return;
		}
	}

	Value += Delta;
}

FGameplayTagQuery FGameplayTagQueryContainer::GetCombinedQuery()
{
	if (bIsDirty)
	{
		CombineQueries();
		bIsDirty = false;
	}

	return CachedCombinedQuery;
}

void FGameplayTagQueryContainer::CombineQueries()
{
	FGameplayTagQueryExpression CombinedExpression;

	FGameplayTagQueryExpression Temp;
	for (const auto& [GameplayTagQuery, Count] : TagQueries)
	{
		GameplayTagQuery.GetQueryExpr(Temp);
		CombinedExpression.AddExpr(Temp);
	}

	CachedCombinedQuery.Build(CombinedExpression, TEXT("Combined Tag Query"));
}

#endif HAS_TYPE_HASH