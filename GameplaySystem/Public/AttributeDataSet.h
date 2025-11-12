// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "Attribute.h"

#include "AttributeDataSet.generated.h"

// Intermediary struct for easier editing of values in the editor
// Avoids an issue where you need to assign the AttributeType both in Key and in Struct when editing a DataSet
USTRUCT(BlueprintType)
struct FEditorAttribute
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float BaseValue = 0;

	float CurrentValue = 0;

	FEditorAttribute() = default;

	FEditorAttribute(float Value) 
		: BaseValue(Value), CurrentValue(Value) {}

	FEditorAttribute(float BaseVal, float CurrentVal)
		: BaseValue(BaseVal), CurrentValue(CurrentVal) {}
};

UCLASS(Blueprintable, BlueprintType)
class GAMEPLAYSYSTEM_API UAttributeDataSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// The key type and member type should be the same.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TMap<EAttributeType, FEditorAttribute> Attributes;

	// Adds all in-use attribute types. Does not clear the container first to avoid wiping data by accident.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Utility")
	void UseDefaultConfiguration();

	// Does not clear the container first to avoid wiping data by accident.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Utility")
	void UseEnemyConfiguration();

	// Does not clear the container first to avoid wiping data by accident.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Utility")
	void UsePlayerConfiguration();


	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Utility")
	void RemovePlayerOnlyAttributes();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Utility")
	void RemoveEnemyOnlyAttributes();
	
private:

	static const TArray<EAttributeType> DefaultAttributeTypes;
	static const TArray<EAttributeType> EnemyAttributeTypes;
	static const TArray<EAttributeType> PlayerAttributeTypes;
	static const TArray<EAttributeType> PlayerOnlyAttributeTypes;
	static const TArray<EAttributeType> EnemyOnlyAttributeTypes;

	// Utility for adding a selection of attributes
	void AddAttributeTypes(const TArray<EAttributeType>& Types)
	{
		for (const EAttributeType& Type : Types)
		{
			if (!Attributes.Contains(Type))
			{
				Attributes.Add(Type, FEditorAttribute());
			}
		}
	}

	// Utility for removing a selection of attributes
	void RemoveAttributeTypes(const TArray<EAttributeType>& Types)
	{
		for (const EAttributeType& Type : Types)
		{
			if (Attributes.Contains(Type))
			{
				Attributes.Remove(Type);
			}
		}
	}
};
