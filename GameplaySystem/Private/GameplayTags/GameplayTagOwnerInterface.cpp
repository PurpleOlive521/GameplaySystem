 // Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#include "GameplayTagOwnerInterface.h"


void IGameplayTagOwnerInterface::AddTag(const FGameplayTag& TagToAdd)
{
	return;
}
void IGameplayTagOwnerInterface::RemoveTag(const FGameplayTag& TagToRemove)
{
	return;
}

void IGameplayTagOwnerInterface::ClearTag(const FGameplayTag& TagToClear)
{
	return;
}
void IGameplayTagOwnerInterface::AppendTags(FGameplayTagContainer const& Other)
{
	return;
}

bool IGameplayTagOwnerInterface::HasTag(const FGameplayTag& TagToCheck)
{
	return false;
}

bool IGameplayTagOwnerInterface::HasAllTags(const FGameplayTagContainer& TagsToCheckAgainst)
{
	return false;
}

int IGameplayTagOwnerInterface::GetTagCount(const FGameplayTag& TagToCheck)
{
	return 0;
}

int IGameplayTagOwnerInterface::GetTotalTagCount()
{
	return 0;
}