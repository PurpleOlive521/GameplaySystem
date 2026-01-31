// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ActorActionUtility.h"
#include "UObject/Object.h"
#include "ActorBatchingTools.generated.h"

constexpr bool ALLOW_SHAPE_COMPONENTS = false;
constexpr bool REPLACE_SOURCE_ACTORS = true;
constexpr bool COMMIT_MERGE = true;

constexpr bool USE_RECURSIVE_SEARCH = true;
constexpr bool SEARCH_ONLY_DISK_ASSETS = true;

constexpr int INSTANCE_REPLACEMENT_THRESHOLD = 3;

class UPrimitiveComponent;
class UStaticMesh;
class AActor;
class UActorBatchingConfig;

// Wrapper for a candidate primitive component to batch. Pretty empty now but allows for extensions if needed.
struct FBatchComponentData
{
	FBatchComponentData(UPrimitiveComponent* InPrimComponent) : PrimComponent(InPrimComponent) {};

	UPrimitiveComponent* PrimComponent;
};

// Describes a candidate that we allow the Batching tool to run on.
USTRUCT(BlueprintType)
struct FBatchCandidateEntry
{
	GENERATED_BODY()

	FBatchCandidateEntry() = default;

	explicit FBatchCandidateEntry(UStaticMesh* Mesh) : StaticMesh(Mesh) {};

	explicit FBatchCandidateEntry(UStaticMesh* Mesh, bool bIsAddedThisSearch) : StaticMesh(Mesh), bWasAddedThisSearch(bIsAddedThisSearch) {};

	UPROPERTY(EditAnywhere)
	UStaticMesh* StaticMesh = nullptr;

	// Entry will be removed if not part of the latest search.
	bool bWasAddedThisSearch = false;

	friend uint32 GetTypeHash(const FBatchCandidateEntry& Entry);

	bool operator==(const FBatchCandidateEntry& Other) const
	{
		return StaticMesh == Other.StaticMesh;
	}

	bool operator!=(const FBatchCandidateEntry& Other) const
	{
		return StaticMesh != Other.StaticMesh;
	}
};

USTRUCT(BlueprintType)
struct FSearchTerm
{
	GENERATED_BODY()

	FSearchTerm() = default;

	bool IsValid() const;

	UPROPERTY(EditAnywhere)
	FString Term = {};

	friend uint32 GetTypeHash(const FSearchTerm& Entry);

	bool operator==(const FSearchTerm& Other) const
	{
		return Term == Other.Term;
	}

	bool operator!=(const FSearchTerm& Other) const
	{
		return Term != Other.Term;
	}
};

USTRUCT(BlueprintType)
struct FBatchCandidateSearch
{
	GENERATED_BODY()

	FBatchCandidateSearch() = default;

	void GetSearchPaths(TArray<FName>& OutPaths) const;

	// Returns the amount of invalid SearchTerms. 
	int HasInvalidSearchTerms() const;

	// Folders we will conduct the search on.
	UPROPERTY(EditAnywhere, meta = (RelativeToGameContentDir))
	TArray<FDirectoryPath> SearchPaths = {};

	// Queries we use to find Assets to add. Is inclusive, which means that the Asset only needs to contain this string to be added. 
	// E.g, 'SM_Wall' as a SearchTerm will cover the Assets 'SM_Wall' and 'SM_Wall01', 'SM_WallWide', and so on. Case insenstive.
	UPROPERTY(EditAnywhere, meta = (TitleProperty = "Term"))
	TSet<FSearchTerm> SearchTerms;

	// Queries we use to filter out Assets that would otherwise be added by SearchTerms. See SearchTerms for details.
	UPROPERTY(EditAnywhere, meta = (TitleProperty = "Term"))
	TSet<FSearchTerm> RemoveSearchTerms;

	// For unique meshes that are not covered by SearchTerms or are more easily added by hand.
	UPROPERTY(EditAnywhere)
	TArray<UStaticMesh*> ManuallyAddedEntries;

	// Meshes contained here are removed from the final list, even if they would otherwise be added by SearchTerms.
	UPROPERTY(EditAnywhere)
	TArray<UStaticMesh*> ManuallyRemovedEntries;
};

/**
 * A collection of functions that automate the process of optimising meshes.
 * Is opt-in, where any meshes we allow the batching to run on needs to be described as a BatchCandidateEntry.
 */
UCLASS(Blueprintable)
class UActorBatchingTools : public UActorActionUtility
{
	GENERATED_BODY()
	
public:

	UActorBatchingTools();

	// --- Begin UObject interface
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	// --- End UObject interface

	// Attempts to Batch all selected Actors, combining their Meshes and greatly reducing the required draw calls.
	// Recommended for gameplay-static objects that there are a great amount of.
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "ActorBatchingUtility")
	void TryBatchSelectedActors();

	void BuildComponentDataFromActors(const TSet<AActor*>& Actors, TArray<ULevel*>& OutLevels, TArray<FBatchComponentData>& OutComponents);

	bool IsAllowedCandidateEntry(UStaticMeshComponent* StaticMesh) const;

	const UActorBatchingConfig* GetBatchingConfig() const;

	UPROPERTY(EditAnywhere)
	FMeshInstancingSettings BatchSettings;

	// Where we source our Batching settings from. Use different config to target unique sets of meshes for different applications.
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UActorBatchingConfig> BatchingConfigAsset;
};

/**
* A Batching config that targets a subset of Meshes to perform a Batch on.
*/
UCLASS(Blueprintable, HideDropdown)
class UActorBatchingConfig : public UObject
{
	GENERATED_BODY()

public:

	UActorBatchingConfig();
	~UActorBatchingConfig();

	// --- Begin UObject interface
	virtual void PreSave(FObjectPreSaveContext SaveContext) override;
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
	// --- End UObject interface

	// Can be delayed in the case that there are blocking actions or the AssetRegistry is busy loading assets. 
	void PerformCandidateSearch();

	UFUNCTION()
	void CommitCandidateSearch();

	UPROPERTY(EditAnywhere, Category = "ActorBatchingConfig")
	FBatchCandidateSearch SearchParams;

	// The final list of entries that this config will target in a Batch.
	// Dont edit directly, other than for clearing.
	UPROPERTY(EditAnywhere, Category = "ActorBatchingConfig")
	TSet<FBatchCandidateEntry> AllowedEntries;
};

// Returns true if Name is found in any of the FSearchTerms, and not covered by the RemoveSearchTerms.
bool IsInSearchTerms(const TArray<FSearchTerm>& SearchTerms, const TArray<FSearchTerm>& RemoveSearchTerms, const FString& Name);