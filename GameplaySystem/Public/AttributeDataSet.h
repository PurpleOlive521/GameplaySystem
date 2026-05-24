// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "AttributeTypes.h"
#include "Attribute.h"

#include "AttributeDataSet.generated.h"

// Wrapper struct for easier editing of values in the editor UI.
USTRUCT(BlueprintType)
struct FEditorAttribute
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EAttributeType Type = EAttributeType::EAT_NONE;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float BaseValue = 0;

	float CurrentValue = 0;

	FEditorAttribute() = default;

	FEditorAttribute(EAttributeType InType) : Type(InType) {};

	friend uint32 GetTypeHash(const FEditorAttribute& EditorAttribute);

	bool operator==(const FEditorAttribute& Other) const
	{
		return Type == Other.Type;
	}

	bool operator!=(const FEditorAttribute& Other) const
	{
		return Type != Other.Type;
	}
};

UCLASS(Blueprintable, BlueprintType)
class GAMEPLAYSYSTEM_API UAttributeDataSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSet<FEditorAttribute> Attributes;

	// Adds all in-use attribute types. Does not modify existing attributes.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Utility")
	void UseDefaultConfiguration();

	// Does not modify existing attributes.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Utility")
	void UseEnemyConfiguration();

	// Does not modify existing attributes.
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
				Attributes.Add(Type);
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
