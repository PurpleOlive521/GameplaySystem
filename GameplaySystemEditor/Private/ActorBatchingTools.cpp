// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


#include "ActorBatchingTools.h"
#include "Selection.h"
#include "MeshMergeModule.h"
#include "Components/ShapeComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameplaySystemEditorTypes.h"
#include "AssetRegistry/AssetRegistryHelpers.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/ObjectSaveContext.h"
#include "Misc/DataValidation.h"

uint32 GetTypeHash(const FBatchCandidateEntry& Entry)
{
	return PointerHash(Entry.StaticMesh);
}

bool FSearchTerm::IsValid() const
{
	const FString StaticMeshPrefix = TEXT("SM_");

	FString Copy = Term;

	Copy.TrimStartAndEndInline();

	// Is only whitespace
	if (Copy.Len() <= 0)
	{
		return false;
	}
	
	// Doesn't have enough characters to contain prefix
	if (Copy.Len() < StaticMeshPrefix.Len())
	{
		return false;
	}
	
	// Doesn't have the prefix.
	if (Copy.Left(StaticMeshPrefix.Len()) != StaticMeshPrefix)
	{
		return false;
	}

	return true;
}

uint32 GetTypeHash(const FSearchTerm& Entry)
{
	return GetTypeHash(Entry.Term);
}

void FBatchCandidateSearch::GetSearchPaths(TArray<FName>& OutPaths) const
{
	// For some reason not included in the DirectoryPath stringifying.
	const FString Prefix = "/Game/";

	for (const FDirectoryPath& DirectoryPath : SearchPaths)
	{
		OutPaths.Add(FName(Prefix + DirectoryPath.Path));
	}
}

int FBatchCandidateSearch::HasInvalidSearchTerms() const
{
	auto CountInvalids = [](const TSet<FSearchTerm>& Terms)
		{
			int InvalidCount = 0;
			for (const FSearchTerm& Term : Terms)
			{
				if (!Term.IsValid())
				{
					InvalidCount++;
				}
			}

			return InvalidCount;
		};

	int InvalidCount = CountInvalids(SearchTerms);
	InvalidCount += CountInvalids(RemoveSearchTerms);

	return InvalidCount;
}

UActorBatchingTools::UActorBatchingTools()
{
	// Configure Batch settings
	BatchSettings.ActorClassToUse = AActor::StaticClass();
	BatchSettings.ISMComponentToUse = UInstancedStaticMeshComponent::StaticClass();

	BatchSettings.bSkipMeshesWithVertexColors = true;
	BatchSettings.bUseHLODVolumes = false;

	BatchSettings.InstanceReplacementThreshold = INSTANCE_REPLACEMENT_THRESHOLD;
}

EDataValidationResult UActorBatchingTools::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult SuperResult = Super::IsDataValid(Context);

	if (BatchingConfigAsset.IsNull())
	{
		Context.AddError(INVTEXT("No BatchingConfig selected. Please select one in the ActorBatchingTool."));
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}

void UActorBatchingTools::TryBatchSelectedActors()
{
	FScopedSlowTask SlowTask(0, INVTEXT("Batching selected Actors..."));
	SlowTask.MakeDialog();

	const FText MessageDialogueTitle = INVTEXT("Batching Selected Actors");

	if (!GEditor)
	{
		GSED_LOG(Error, TEXT("ActorBatchingTools: TryBatchSelectedActors called without valid Engine!"));

		FText Message = INVTEXT("No valid Engine found while performing Batch.");
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, Message, MessageDialogueTitle);

		return;
	}

	const IMeshMergeModule& MeshMergeModule = FModuleManager::Get().LoadModuleChecked<IMeshMergeModule>("MeshMergeUtilities");

	if (BatchingConfigAsset.IsNull())
	{
		GSED_LOG(Error, TEXT("ActorBatchingTools: No BatchingConfig."));

		FText Message = INVTEXT("No BatchingConfig was selected. Select one in the ActorBatchingTool and try again.");
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, Message, MessageDialogueTitle);

		return;
	}

	const IMeshMergeUtilities& MeshUtilities = MeshMergeModule.GetUtilities();

	TSet<AActor*> SelectedActors;
	USelection* Selection = GEditor->GetSelectedActors();

	for (FSelectionIterator Iter(*Selection); Iter; ++Iter)
	{
		AActor* Actor = Cast<AActor>(*Iter);
		if (Actor)
		{
			SelectedActors.Add(Actor);

			// Add child actors 
			Actor->EditorGetUnderlyingActors(SelectedActors);
		}
	}

	TArray<ULevel*> UniqueLevels;
	TArray<FBatchComponentData> ComponentData;
	BuildComponentDataFromActors(SelectedActors, UniqueLevels, ComponentData);

	if (UniqueLevels.Num() != 1)
	{
		GSED_LOG(Error, TEXT("ActorBatchingTools: TryBatchSelectedActors requires all selected actors to be in the same Level!"));

		FText Message = INVTEXT("The selected Actors must be in the same level.");
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, Message, MessageDialogueTitle);
		return;
	}

	if (ComponentData.Num() <= 0)
	{
		GSED_LOG(Log, TEXT("ActorBatchingTools: No components passed testing."));

		FText Message = INVTEXT("The selected Actors did not have components that were on the allow list for Batching.");
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, Message, MessageDialogueTitle);
		return;
	}

	TArray<UPrimitiveComponent*> ComponentsToBatch;

	for (const FBatchComponentData& Data : ComponentData)
	{
		ComponentsToBatch.Add(Data.PrimComponent);
	}

	// Get current World
	UWorld* World = ComponentData[0].PrimComponent->GetWorld();

	if (!World)
	{
		GSED_LOG(Error, TEXT("ActorBatchingTools: TryBatchSelectedActors called on components without a valid World!"));

		FText Message = INVTEXT("No World found from selected component(s).");
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, Message, MessageDialogueTitle);
		return;
	}

	// Confirm with user
	FText Title = INVTEXT("Batching Actors");
	const FString MessageString = FString::Printf(TEXT("About to perform Batch on %d found components. Are you sure?"), ComponentsToBatch.Num());
	FText Message = FText::AsCultureInvariant(MessageString);
	EAppReturnType::Type DialogResult = FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::OkCancel, Message, MessageDialogueTitle);

	if (DialogResult != EAppReturnType::Type::Ok)
	{
		GSED_LOG(Error, TEXT("ActorBatchingTools: Aborting Batch!"));
		return;
	}

	FText ResultText;
	MeshUtilities.MergeComponentsToInstances(ComponentsToBatch, World, UniqueLevels[0], BatchSettings, COMMIT_MERGE, REPLACE_SOURCE_ACTORS, &ResultText);
}

void UActorBatchingTools::BuildComponentDataFromActors(const TSet<AActor*>& Actors, TArray<ULevel*>& OutLevels, TArray<FBatchComponentData>& OutComponents)
{
	OutLevels.Empty();
	for (AActor* Actor : Actors)
	{
		if (!Actor)
		{
			continue;
		}

		// Get Level
		ULevel* ActorLevel = Actor->GetLevel();
		if (!ActorLevel)
		{
			continue;
		}

		OutLevels.AddUnique(ActorLevel);

		// Get Primitive Components
		TArray<UPrimitiveComponent*> PrimComponents;
		Actor->GetComponents<UPrimitiveComponent>(PrimComponents);

		for (UPrimitiveComponent* PrimComponent : PrimComponents)
		{
			// Test for validity before adding

			if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(PrimComponent))
			{
				const bool bIsValidEntry = IsAllowedCandidateEntry(StaticMeshComponent);
				if (!bIsValidEntry)
				{
					continue;
				}
			}
			else if (UShapeComponent* ShapeComponent = Cast<UShapeComponent>(PrimComponent))
			{
				if (!ALLOW_SHAPE_COMPONENTS)
				{
					continue;
				}
			}

			// Tests passed
			OutComponents.Add(FBatchComponentData(PrimComponent));
		}


	}
}

bool UActorBatchingTools::IsAllowedCandidateEntry(UStaticMeshComponent* StaticMesh) const
{
	if (StaticMesh->GetStaticMesh() == nullptr)
	{
		return false;
	}
	
	UStaticMesh* Mesh = StaticMesh->GetStaticMesh();
	FBatchCandidateEntry Key = FBatchCandidateEntry(Mesh);

	if (!GetBatchingConfig())
	{
		return false;
	}

	const FBatchCandidateEntry* CandidateEntry = GetBatchingConfig()->AllowedEntries.FindByHash(GetTypeHash(Key), Key);

	// Not in the list, not allowed
	if (!CandidateEntry)
	{
		return false;
	}

	// Further testing of future parameters would be done here

	return true;
}

const UActorBatchingConfig* UActorBatchingTools::GetBatchingConfig() const
{
	ensure(!BatchingConfigAsset.IsNull());

	if(BatchingConfigAsset.IsNull())
	{
		return nullptr;
	}
	
	UClass* ConfigClass = BatchingConfigAsset.LoadSynchronous();

	check(ConfigClass);

	return ConfigClass->GetDefaultObject<UActorBatchingConfig>();
}

UActorBatchingConfig::UActorBatchingConfig()
{
}

UActorBatchingConfig::~UActorBatchingConfig()
{
	// Remove all delegates in case we are closed mid-operation.
	FAssetRegistryModule* AssetRegistryModule = FModuleManager::LoadModulePtr<FAssetRegistryModule>("AssetRegistry");
	if (AssetRegistryModule)
	{
		// Null on engine shutdown
		IAssetRegistry* AssetRegistry = AssetRegistryModule->TryGet();
		if (AssetRegistry)
		{
			AssetRegistry->OnFilesLoaded().RemoveAll(this);
		}

	}
}

void UActorBatchingConfig::PreSave(FObjectPreSaveContext SaveContext)
{
	Super::PreSave(SaveContext);

	// Only perform when user-edits are present.
	if (!SaveContext.IsProceduralSave())
	{
		PerformCandidateSearch();
	}
}

EDataValidationResult UActorBatchingConfig::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult SuperResult = Super::IsDataValid(Context);

	const int RemovedSearchTerms = SearchParams.HasInvalidSearchTerms();

	if (RemovedSearchTerms > 0)
	{
		Context.AddError(INVTEXT("Invalid SearchTerms found! SearchTerms must be non-empty and contain the prefix 'SM_'."));
		return EDataValidationResult::Invalid;
	}

	return EDataValidationResult::Valid;
}

void UActorBatchingConfig::PerformCandidateSearch()
{
	const FText MessageDialogueTitle = INVTEXT("Starting Asset Search");

	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Delay search if the AssetRegistry is busy loading assets.
	if (AssetRegistry.IsLoadingAssets())
	{
		AssetRegistry.OnFilesLoaded().AddUObject(this, &UActorBatchingConfig::CommitCandidateSearch);
	}

	CommitCandidateSearch();
}

void UActorBatchingConfig::CommitCandidateSearch()
{
	// --- Pre-commit checks and setup
	FScopedSlowTask SlowTask(0, INVTEXT("Performing Asset search..."));
	SlowTask.MakeDialog();

	const FText MessageDialogueTitle = INVTEXT("Actor Batching Config: Asset Search");

	for (UStaticMesh* Mesh : SearchParams.ManuallyAddedEntries)
	{
		AllowedEntries.Add(FBatchCandidateEntry(Mesh, true));
	}

	FAssetRegistryModule* AssetRegistryModule = FModuleManager::GetModulePtr<FAssetRegistryModule>("AssetRegistry");
	if (!AssetRegistryModule)
	{
		GSED_LOG(Warning, TEXT("ActorBatchingTools: No AssetRegistry Module found while performing Search."));

		FText Message = INVTEXT("No AssetRegistry Module found while performing Search.");
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, Message, MessageDialogueTitle);

		return;
	}

	IAssetRegistry* AssetRegistry = &AssetRegistryModule->Get();

	if (!AssetRegistry)
	{
		GSED_LOG(Warning, TEXT("ActorBatchingTools: No AssetRegistry available while performing Search."));

		FText Message = INVTEXT("No AssetRegistry available while performing Search.");
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, Message, MessageDialogueTitle);

		return;
	}

	TArray<FName> SearchPaths;
	SearchParams.GetSearchPaths(SearchPaths);
	TArray<FAssetData> OutAssets;
	const bool bSearchSuccess = AssetRegistry->GetAssetsByPaths(SearchPaths, OutAssets, USE_RECURSIVE_SEARCH, SEARCH_ONLY_DISK_ASSETS);

	if (!bSearchSuccess)
	{
		GSED_LOG(Warning, TEXT("ActorBatchingTools: No Assets found in search."));

		FText Message = INVTEXT("No Assets found while performing search.");
		FMessageDialog::Open(EAppMsgCategory::Error, EAppMsgType::Ok, Message, MessageDialogueTitle);

		return;
	}

	// --- Commence filtering

	int AddedCount = 0;
	{
		const TArray<FSearchTerm> SearchTermsArray = SearchParams.SearchTerms.Array();
		const TArray<FSearchTerm> RemoveSearchTermsArray = SearchParams.RemoveSearchTerms.Array();
		UObject* Asset = nullptr;
		UStaticMesh* Mesh = nullptr;
		FString FullName = {};
		bool bMeshAlreadyAdded = false;

		for (const FAssetData& AssetData : OutAssets)
		{
			bMeshAlreadyAdded = false;
			Asset = AssetData.GetAsset();

			if (!Asset)
			{
				continue;
			}

			Mesh = Cast<UStaticMesh>(Asset);
			if (! Mesh)
			{
				continue;
			}

			// In the manually remove list, don't add
			if (SearchParams.ManuallyRemovedEntries.Contains(Mesh))
			{
				continue;
			}

			FullName = AssetData.GetFullName();
			const FString _Test = Asset->GetName();

			if (!IsInSearchTerms(SearchTermsArray, RemoveSearchTermsArray, FullName))
			{
				continue;
			}

			// Passed all filtering

			FBatchCandidateEntry& Entry = AllowedEntries.FindOrAdd(FBatchCandidateEntry(Mesh), &bMeshAlreadyAdded);
			Entry.bWasAddedThisSearch = true;

			if (!bMeshAlreadyAdded)
			{
				AddedCount++;
			}
		}
	}

	// --- Post filtering adjustments

	int RemovedCount = 0;
	for (UStaticMesh* RemovedMesh : SearchParams.ManuallyRemovedEntries)
	{
		RemovedCount += AllowedEntries.Remove(FBatchCandidateEntry(RemovedMesh));
	}

	// Entries that were not re-added are not covered by the new search, remove them
	TArray<FBatchCandidateEntry> ForgetList;
	for(const FBatchCandidateEntry& Entry : AllowedEntries)
	{
		if (!Entry.bWasAddedThisSearch)
		{
			ForgetList.Add(Entry);
		}
	}

	for (const FBatchCandidateEntry& EntryToForget : ForgetList)
	{
		AllowedEntries.Remove(EntryToForget);
		// TODO: Add to separate Forget counter?
		RemovedCount++;
	}

	// --- Filtering finished messaging

	if (AddedCount == 0 && RemovedCount == 0)
	{
		GSED_LOG(Log, TEXT("ActorBatchingTools: Search finished without modified entries."), AddedCount);

		FText Message = INVTEXT("Finished Search. No new entries found or removed.");
		FMessageDialog::Open(EAppMsgCategory::Info, EAppMsgType::Ok, Message, MessageDialogueTitle);

		return;
	}

	GSED_LOG(Log, TEXT("ActorBatchingTools: Search finished with %d added Entries and %d removed Entries."), AddedCount, RemovedCount);

	const FString MessageString = FString::Printf(TEXT("Finished Search. Searched through %d Assets, adding %d Entries and removing %d Entries."), OutAssets.Num(), AddedCount, RemovedCount);
	FText Message = FText::AsCultureInvariant(MessageString);
	FMessageDialog::Open(EAppMsgCategory::Success, EAppMsgType::Ok, Message, MessageDialogueTitle);
}

bool IsInSearchTerms(const TArray<FSearchTerm>& SearchTerms, const TArray<FSearchTerm>& RemoveSearchTerms, const FString& Name)
{
	bool bIsInAllowList = false;
	for (const FSearchTerm& SearchTerm : SearchTerms)
	{
		if (Name.Contains(SearchTerm.Term))
		{
			bIsInAllowList = true;
			break;
		}
	}

	if (!bIsInAllowList)
	{
		return false;
	}

	for (const FSearchTerm& SearchTerm : RemoveSearchTerms)
	{
		if (Name.Contains(SearchTerm.Term))
		{
			return false;
		}
	}

	return true;
}
