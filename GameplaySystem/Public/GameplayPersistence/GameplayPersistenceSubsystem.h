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

UENUM(BlueprintType)
enum class ESaveGameCondition : uint8
{
	// The save file is up to date with game version and contains data.
	ESGC_Healthy		UMETA(DisplayName = "Healthy"),

	// The save file contains data but is not up to date with game version.
	ESGC_Outdated		UMETA(DisplayName = "Outdated"),

	// The save file is empty.
	ESGC_Empty			UMETA(DisplayName = "Empty"),

	// This should never be encountered in game.
	ESGC_Invalid		UMETA(DisplayName = "Invalid"),
};

USTRUCT(BlueprintType)
struct FSaveGameStatus
{
	GENERATED_BODY()

	FSaveGameStatus() = default;

	// Fetches properties given a valid World to perform work in.
	FSaveGameStatus(UWorld* World);

	// Applies some stored properties, such as Playtime.
	// Player related data is not applied, as it's serialized separately.
	void ApplyStateOnWorld(UWorld* World);

	inline static FSaveGameStatus MakeEmpty();

	void UpdateConditionFromArchive(FArchive& Archive);

	UPROPERTY(BlueprintReadWrite)
	int32 PlayerLevel = 1;

	UPROPERTY(BlueprintReadWrite)
	double Playtime = 0.0;

	UPROPERTY(BlueprintReadWrite)
	ESaveGameCondition Condition = ESaveGameCondition::ESGC_Invalid;

	// Serialization functions
	friend void operator<<(class FArchive& Ar, FSaveGameStatus& Status);
	friend void operator<<(FStructuredArchive::FSlot Slot, FSaveGameStatus& Status);
};

struct FLevelState
{
	FLevelState() = default;

	FLevelState(bool bIsLoaded);

	void RegisterActor(AActor* Actor);

	void MarkActorDestroyed(AActor* Actor);

	void Reset();

	// Will invalidate any active iterators and for-loops, use with caution.
	void RemoveStaleActors();

	bool bIsLoaded = false;

	bool bIsPredictedState = false;
	
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

	void AddLevel(LevelName Level, bool bIsLoaded = false);

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameLoadedSignature);
	
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

	// Helper getter
	static UGameplayPersistenceSubsystem* Get(const UObject* WorldContext);

	// Control whether or not saving is allowed.
	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	void SetIsSavingAllowed(bool bIsAllowed);

	// Gets the playtime in seconds. Sensitive to changes to system time.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayPersistance")
	double GetPlaytime();

	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	void SetPlaytime(double InPlaytime);

	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	void SaveGame(const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	void LoadGame(const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	FSaveGameStatus GetSaveGameStatus(const FString& SlotName);

	// Returns true if we are in the process of loading the game.
	// Does not return true if only loading an individual sublevel.
	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	bool IsGameLoadInProgress();

	// This will remove any existing SaveData that we have stored. This does not affect saves that are written to disk.
	// Use with caution!
	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	void ClearData();

protected:

	void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues InitValues);

	void OnActorsInitialized(const FActorsInitializedParams& Params);

	void OnWorldBeginTearDown(UWorld* World);

	void OnSublevelLoaded(ULevelStreaming* StreamingObject);

	void OnSublevelLoaded(ULevel* Level, UWorld* World);

	void OnSublevelRemoved(ULevelStreaming* StreamingObject);

	void OnSublevelRemoved(ULevel* Level, UWorld* World);

	void OnActorPreSpawn(AActor* Actor, UWorld* World);

	void OnActorDestroyed(AActor* Actor, UWorld* World);

	void OnLoadCompleted();

	void SaveSublevel(ULevel* Level);

	void LoadSublevel(ULevel* Level);

private:

	bool bIsSavingAllowed = false;

	double RealTimeValueTimestamp = 0.0;

	double Playtime = 0.0;

	TSharedPtr<class FSaveGameSerializer, ESPMode::ThreadSafe> CurrentSerializer;

	FSaveGameState SaveGameState;

	FArchiveSectionContainer BinaryScopeContainer;
	TArray<uint8> BinaryData;

#if WITH_TEXT_ARCHIVE_SUPPORT
	FArchiveSectionContainer TextScopeContainer;
	TArray<uint8> TextData;
#endif //WITH_TEXT_ARCHIVE_SUPPORT

public:

	// --- Delegates
	UPROPERTY(BlueprintAssignable, Category = "GameplayPersistance")
	FOnGameLoadedSignature OnGameLoadedDelegate;

};
