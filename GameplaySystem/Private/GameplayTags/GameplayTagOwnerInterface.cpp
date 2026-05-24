// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


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

bool IGameplayTagOwnerInterface::HasTag(const FGameplayTag& TagToCheck) const
{
	return false;
}

bool IGameplayTagOwnerInterface::HasAllTags(const FGameplayTagContainer& TagsToCheckAgainst) const
{
	return false;
}

int32 IGameplayTagOwnerInterface::GetTagCount(const FGameplayTag& TagToCheck) const
{
	return 0;
}

void IGameplayTagOwnerInterface::SetTagCount(const FGameplayTag& TagToSet, int32 NewCount)
{
	return;
}

int32 IGameplayTagOwnerInterface::GetTotalTagCount() const
{
	return 0;
}