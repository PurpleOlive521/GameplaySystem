// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "GameplayTagSystem.generated.h"

// Broadcasted when a GameplayTag has it's count modified. Does not mean that the presence of the tag is changed.
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnGameplayTagModifiedSignature, FGameplayTag /* GameplayTag */, int32 /* NewCount */, int32 /* Delta */);

// Only broadcasted when the presence of a GameplayTag is changed. 
// bWasAdded being false means the GameplayTag was removed.
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGameplayTagChangedSignature, FGameplayTag /* GameplayTag */, bool /* bWasAdded */);

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayTagSystem
{
	GENERATED_BODY()

	FGameplayTagSystem();

	// Add one instance of the tag.
	void AddTag(const FGameplayTag& TagToAdd);

	// Remove one instance of the tag.
	void RemoveTag(const FGameplayTag& TagToRemove);

	// Remove one instance of each tag.
	void RemoveTags(const FGameplayTagContainer& TagsToRemove);

	// Sets the amount of instances of the tag to 0.
	void ClearTag(const FGameplayTag& TagToClear);

	void AppendTags(FGameplayTagContainer const& Other);

	// Calls FGameplayTagContainers function
	bool HasTag(const FGameplayTag& TagToCheck) const;

	// Calls FGameplayTagContainers function
	bool HasAnyTag(const FGameplayTagContainer& TagsToCheckFor) const;

	// Calls FGameplayTagContainers function
	bool HasAllTags(const FGameplayTagContainer& TagsToCheckAgainst) const;

	// Get the amount of tags present for a specific type.
	int32 GetTagCount(const FGameplayTag& TagToCheck) const;

	void SetTagCount(const FGameplayTag& TagToSet, int32 NewCount);

	int32 GetTotalTagCount() const;

	// Safely increment or decrement the count of a tag. If the tag is not present and becomes positive, it will be added and vise-versa.
	void ModifyTagCount(const FGameplayTag& TagToModify, int32 Delta);

	// Overwrite any present GameplayTags with GameplayTagContainerIn
	void SetGameplayTagContainer(const FGameplayTagContainer& GameplayTagContainerIn);

	// Get all the tags that are currently applied
	FGameplayTagContainer GetGameplayTagContainer() const;

	// Overwrite the count for all tags with GameplayTagTableIn
	void SetGameplayTagTable(const TMap<FGameplayTag, int32>& GameplayTagTableIn);

	// Simple accessor
	void GetGameplayTagTable(TMap<FGameplayTag, int32>& GameplayTagTableOut) const;

	TMap<FGameplayTag, int32>::TConstIterator GetConstGameplayTagIterator() const;

	// Formats the tag count for display as 'TagName (Count)'
	void ToStringArray(TArray<FString>& OutString) const;

	// Formats the tag count for display as 'TagName (Count)' with color tags included for easier readability.
	void ToStringArrayWithDebugTags(TArray<FString>& OutString) const;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "GameplayTagComponent")
	FGameplayTagContainer GameplayTags;

	// Keeps track of the amount of each tag thats currently applied, to safely track multiple sources of the same tag.
	UPROPERTY(SaveGame)
	TMap<FGameplayTag, int32> TagCountTable;

	// --- Delegates

	// All non-redundant changes are broadcasted, even if only numerical and not presence changing.
	FOnGameplayTagModifiedSignature OnGameplayTagModifiedDelegate;

	// Only changes to presence (removed / added) are broadcasted.
	FOnGameplayTagChangedSignature OnGameplayTagChangedDelegate;
};