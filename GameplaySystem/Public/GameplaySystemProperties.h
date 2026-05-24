// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AttributeDataSet.h"
#include "GameplayTagContainer.h"
#include "GameplaySystemProperties.generated.h"

/*
 *  Assetable default properties for a GameplaySystemComponents. Avoids property duplication in the GameplaySystem and easier access for designers.
 */
UCLASS(Blueprintable, BlueprintType)
class GAMEPLAYSYSTEM_API UGameplaySystemProperties : public UPrimaryDataAsset
{
	GENERATED_BODY()

	friend class UGameplaySystemComponent;

public:

	// The starting level for the GameplaySystem. A value of -1 will disable the level system and collecting experience won't trigger levelups.
	// For more info, see GameplaySystemConstants.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplaySystemProperties")
	int Level = 1;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplaySystemProperties")
	FGameplayTagContainer GameplayTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplaySystemProperties")
	TObjectPtr<UAttributeDataSet> Attributes = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplaySystemProperties")
	TObjectPtr<UCurveTable> LevelScalingSet = nullptr;
};