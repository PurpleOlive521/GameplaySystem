// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "GameplayTagSystem.h"

#include "GameplayTagOwnerInterface.generated.h"

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint))
class GAMEPLAYSYSTEM_API UGameplayTagOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implement this interface on an object that owns a FGameplayTagSystem to give blueprint limited access.
 * Serves no purpose for native logic, and should generally not be used in favor of directly accessing the FGameplayTagSystem.
 */
class GAMEPLAYSYSTEM_API IGameplayTagOwnerInterface
{
	GENERATED_BODY()

public:

	// Add 1 instance of the tag.
	UFUNCTION(BlueprintCallable, Category = "GameplayTagOwnerInterface")
	virtual void AddTag(const FGameplayTag& TagToAdd);

	// Remove 1 instance of the tag.
	UFUNCTION(BlueprintCallable, Category = "GameplayTagOwnerInterface")
	virtual void RemoveTag(const FGameplayTag& TagToRemove);

	// Sets the amount of instances of the tag to 0.
	UFUNCTION(BlueprintCallable, Category = "GameplayTagOwnerInterface")
	virtual void ClearTag(const FGameplayTag& TagToClear);

	// Adds all the tags present in Other. Each tag present counts as 1 count of that tag.
	UFUNCTION(BlueprintCallable, Category = "GameplayTagOwnerInterface")
	virtual void AppendTags(FGameplayTagContainer const& Other);

	// Returns true if the tag is present in atleast 1 quantity, false otherwise.
	UFUNCTION(BlueprintCallable, Category = "GameplayTagOwnerInterface")
	virtual bool HasTag(const FGameplayTag& TagToCheck);

	// Returns true if all tags in the container are present in atleast 1 quantity, false otherwise.
	UFUNCTION(BlueprintCallable, Category = "GameplayTagOwnerInterface")
	virtual bool HasAllTags(const FGameplayTagContainer& TagsToCheckAgainst);

	UFUNCTION(BlueprintCallable, Category = "GameplayTagOwnerInterface")
	virtual int GetTagCount(const FGameplayTag& TagToCheck);

	UFUNCTION(BlueprintCallable, Category = "GameplayTagOwnerInterface")
	virtual int GetTotalTagCount();


};
