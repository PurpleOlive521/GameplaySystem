// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayPersistenceLog.h"
#include "ArchiveSectionContainer.h"
#include "GameplayPersistenceSubsystem.generated.h"

class UGameplaySaveGame;

#define GLOBAL_DATA TEXT("GlobalData")

typedef FName LevelName;

struct FLevelState
{
	FLevelState() = default;

	FLevelState(bool bIsLoaded);

	void RegisterActor(AActor* Actor);

	void MarkActorDestroyed(AActor* Actor);

	void Reset();

	bool bIsLoaded = false;
	
	// TODO: GlobalData Actors wont be unregistered, since they could be part of any unloading level and would require iterating the actors to check
	//		 Look into differentiation them further so we can filter them out when unloading sublevels?

	// Actors that want to be saved with the level.
	TSet<TWeakObjectPtr<AActor>> RegisteredActors;

	// These Actors are not limited to Saveable ones, and can be any Actor.
	TSet<FSoftObjectPath> DestroyedActors;
};

struct FSaveGameState
{
	FSaveGameState() = default;

	// Returns true if it is registered. Global data will be saved separately from per-level data.
	bool TryRegisterActor(AActor* Actor);

	// Also removes any save data for this Actor and unregisters it.
	void AddDestroyedActor(AActor* Actor);

	void MarkLevelLoaded(ULevel* Level);

	void MarkLevelLoaded(TSoftObjectPtr<UWorld> WorldAsset);

	void MarkLevelUnloaded(ULevel* Level);

	void MarkLevelUnloaded(TSoftObjectPtr<UWorld> WorldAsset);

	void ResetState();

	TMap<LevelName, FLevelState> LevelData;

	FLevelState GlobalData;

private:
	void MarkLevelLoaded_Internal(LevelName Level);

	void MarkLevelUnloaded_Internal(LevelName Level);
};

namespace LevelUtilities
{
	LevelName GetLevelNameFromWorld(UWorld* World);
	LevelName GetLevelNameFromLevel(ULevel* Level);
	LevelName GetLevelNameFromSoftObjectPtr(const TSoftObjectPtr<ULevel>& Object);
	LevelName GetLevelNameFromSoftObjectPtr(const TSoftObjectPtr<UWorld>& Object);

	LevelName GetLevelName_Internal(const FSoftObjectPath& Path);

	FString GetActorName(AActor* Actor);
	FName GetActorNameFromSoftObjectPath(const FSoftObjectPath& Path);
}
	
UCLASS()
class GAMEPLAYSYSTEM_API UGameplayPersistenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	template<bool, bool> friend class TSaveGameSerializer;

public:
	
	// -- Begin USubSystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// --- End USubSystem Interface

	// Control whether or not saving is allowed.
	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	void SetIsSavingAllowed(bool bIsAllowed);

	// Gets the cumulative playtime on the currently used save, in seconds. Sensitive to changes to system time.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayPersistance")
	float GetPlayTime();

	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	void SaveGame(const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	void LoadGame(const FString& SlotName);

protected:

	void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues InitValues);

	void OnActorsInitialized(const FActorsInitializedParams& Params);

	void OnWorldBeginTearDown(UWorld* World);

	void OnSublevelLoaded(ULevel* Level, UWorld* World);

	void OnSublevelRemoved(ULevel* Level, UWorld* World);

	void OnActorPreSpawn(AActor* Actor, UWorld* World);

	void OnActorDestroyed(AActor* Actor, UWorld* World);

	void OnLoadCompleted();

	void SaveSublevel(ULevel* Level);

	void LoadSublevel(ULevel* Level);

private:

	bool bIsSavingAllowed = false;

	// Offset used to infer the current playtime when compared to GetRealTime.
	float RealTimeValueReference = 0.0f;

	TSharedPtr<class FSaveGameSerializer, ESPMode::ThreadSafe> CurrentSerializer;

	FSaveGameState SaveGameState;

	FArchiveSectionContainer BinaryScopeContainer;
	TArray<uint8> BinaryData;

#if WITH_TEXT_ARCHIVE_SUPPORT
	FArchiveSectionContainer TextScopeContainer;
	TArray<uint8> TextData;
#endif //WITH_TEXT_ARCHIVE_SUPPORT
};
