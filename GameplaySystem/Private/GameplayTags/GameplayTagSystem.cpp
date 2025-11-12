// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "GameplayTagSystem.h"

#include "GameplaySaveGameTypes.h"
#include "DevelopmentTypes.h"

FGameplayTagSystem::FGameplayTagSystem()
{
}

FGameplayTagSystemSaveObject FGameplayTagSystem::GetSaveData() const
{
	FGameplayTagSystemSaveObject SaveData;
	SaveData.GameplayTags = GetGameplayTagContainer();
	GetGameplayTagTable(SaveData.TagCountTable);

	return SaveData;
}

void FGameplayTagSystem::LoadFromData(const FGameplayTagSystemSaveObject& GameplayTagsSaveData)
{
	SetGameplayTagContainer(GameplayTagsSaveData.GameplayTags);
	SetGameplayTagTable(GameplayTagsSaveData.TagCountTable);
}

void FGameplayTagSystem::AddTag(const FGameplayTag& TagToAdd)
{
	ModifyTagCount(TagToAdd, 1);
}

void FGameplayTagSystem::RemoveTag(const FGameplayTag& TagToRemove)
{
	ModifyTagCount(TagToRemove, -1);
}

void FGameplayTagSystem::RemoveTags(const FGameplayTagContainer& TagsToRemove)
{
	for(const FGameplayTag& Tag : TagsToRemove)
	{
		ModifyTagCount(Tag, -1);
	}
}


void FGameplayTagSystem::ClearTag(const FGameplayTag& TagToClear)
{
	if (TagCountTable.Contains(TagToClear))
	{
		ModifyTagCount(TagToClear, -GetTagCount(TagToClear));
	}
	else
	{
		GS_LOG(Warning, TEXT("Tried to clear tag %s, but it was not present in the TagCountTable!"), *TagToClear.ToString());
	}
}

void FGameplayTagSystem::AppendTags(FGameplayTagContainer const& Other)
{
	for (const FGameplayTag& Tag : Other)
	{
		ModifyTagCount(Tag, 1);
	}

	GameplayTags.AppendTags(Other);
}

bool FGameplayTagSystem::HasTag(const FGameplayTag& TagToCheck) const
{
	// Allow container itself to handle logic to check for parent/child relations natively
	return GameplayTags.HasTag(TagToCheck);;
}

bool FGameplayTagSystem::HasAnyTag(const FGameplayTagContainer& TagsToCheckFor) const
{
	return GameplayTags.HasAny(TagsToCheckFor);
}

bool FGameplayTagSystem::HasAllTags(const FGameplayTagContainer& TagsToCheckAgainst) const
{
	return GameplayTags.HasAll(TagsToCheckAgainst);
}

int FGameplayTagSystem::GetTagCount(const FGameplayTag& TagToCheck) const
{
	if (const int* Count = TagCountTable.Find(TagToCheck))
	{
		return *Count;
	}

	return 0;
}

int FGameplayTagSystem::GetTotalTagCount() const
{
	return GameplayTags.Num();
}

void FGameplayTagSystem::ModifyTagCount(const FGameplayTag& TagToModify, int Delta)
{
	// Extract all parent tags
	const FGameplayTagContainer ExpandedTags = TagToModify.GetGameplayTagParents();

	// Iterate over all expanded tags and add/remove them separately
	for (auto TagIt = ExpandedTags.CreateConstIterator(); TagIt; ++TagIt) // Avoids copying or passing the ExpandedTags around
	{
		const FGameplayTag& CurrentTag = *TagIt;

		// Replace old Count with Delta applied
		int Count = TagCountTable.FindOrAdd(CurrentTag);
		Count += Delta;
		TagCountTable.Add(CurrentTag, Count);

		// If the count is 0 or less, remove the tag from the table
		if (Count < 0)
		{
			GameplayTags.RemoveTag(CurrentTag, false);
		}
		else if (Count == 0) 
		{
			TagCountTable.Remove(CurrentTag); // The absence of a tag already implies a count of 0
			GameplayTags.RemoveTag(CurrentTag, false);
		}
		else // Count of 1 or more
		{
			GameplayTags.AddTag(CurrentTag);
		}
	}

	// We avoid calling it in the loop to avoid unnecessary overhead
	GameplayTags.FillParentTags();
	//OnGameplayTagChangedDelegate.Broadcast(TagToEdit, Count);
}

void FGameplayTagSystem::SetGameplayTagContainer(const FGameplayTagContainer& GameplayTagContainerIn)
{
	GameplayTags = GameplayTagContainerIn;
}

FGameplayTagContainer FGameplayTagSystem::GetGameplayTagContainer() const
{
	return GameplayTags;
}

void FGameplayTagSystem::SetGameplayTagTable(const TMap<FGameplayTag, int>& GameplayTagTableIn)
{
	TagCountTable = GameplayTagTableIn;
}

void FGameplayTagSystem::GetGameplayTagTable(TMap<FGameplayTag, int>& GameplayTagTableOut) const
{
	GameplayTagTableOut = TagCountTable;
}

TMap<FGameplayTag, int>::TConstIterator FGameplayTagSystem::GetConstGameplayTagIterator() const
{
	return TMap<FGameplayTag, int>::TConstIterator(TagCountTable);
}

void FGameplayTagSystem::ToStringArray(TArray<FString>& OutString) const
{
	OutString.Empty();

	for (const auto& TagEntry : TagCountTable)
	{
		OutString.Add(FString::Printf(TEXT("%s (%d)"), *TagEntry.Key.ToString(), TagEntry.Value));
	}
}

using namespace DebugTypes;

void FGameplayTagSystem::ToStringArrayWithDebugTags(TArray<FString>& OutString) const
{
	OutString.Empty();
	FString TagAsString;

	for (const auto& TagEntry : TagCountTable)
	{
		TagAsString = TagEntry.Key.ToString();
		TagAsString += TEXT("(") + TextTag_Highlight + FString::FromInt(TagEntry.Value) + TextTag_End + TEXT(")");

		OutString.Add(TagAsString);
	}

	OutString.Sort();
}

