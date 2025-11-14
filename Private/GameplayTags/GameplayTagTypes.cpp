// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


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
