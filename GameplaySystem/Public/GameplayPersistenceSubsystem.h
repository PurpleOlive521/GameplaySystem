// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayPersistenceSubsystem.generated.h"

class UGameplaySaveGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameSavedToDiskSignature, UGameplaySaveGame*, ActiveSaveGameObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameLoadedFromDiskSignature, UGameplaySaveGame*, LoadedSaveGameObject);

UCLASS()
class GAMEPLAYSYSTEM_API UGameplayPersistenceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	
	// -- Begin USubSystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// --- End USubSystem

	// Replaces the active save game object with a new object. Functionally the same as having no active save data. 
	// Override to create your own SaveGame subclass.
	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	virtual void CreateSaveGameObject();

	// Prompts all registered objects to save, before writing the SaveGameObject to disk.
	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	void RequestSaveToDisk(bool bIsAsync);

	// Will call ISaveableObjectInterface::SaveData on the supplied object if saving is allowed.
	UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = "Object"), Category = "GameplayPersistance")
	void RequestSaveObject(UObject* Object);
	
	// Will call ISaveableObjectInterface::LoadData on the supplied object if loading is allowed.
	UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = "Object"), Category = "GameplayPersistance")
	void RequestLoadObject(UObject* Object);

	// Registers the object to respond to save & load requests from game logic.
	UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = "Object"), Category = "GameplayPersistance")
	void RegisterObject(UObject* Object);

	// Convenience function, since most objects are going to want to register & load in succession.
	// Internally just calls RegisterObject and RequestLoadObject
	UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = "Object"), Category = "GameplayPersistance")
	void RegisterAndLoadObject(UObject* Object);

	// Control whether or not saving is allowed.
	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	void SetSavingState(bool bCanSave);

	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	void SetSlotName(FString SlotName);

	// Circumvents checks to always allow saving/loading directly with the save object, use with causion when modifying save data.
	// Use when the intention is to always be able to access save/load functionality, regardless of whether this allows it.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayPersistance")
	UGameplaySaveGame* GetSaveGameObject() const;

	// Gets the cumulative playtime on the currently used save, in seconds. Sensitive to changes to system time.
	UFUNCTION(BlueprintCallable, BlueprintPure ,Category = "GameplayPersistance")
	float GetPlayTime();

protected:

	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	void SaveGameToDisk(bool bIsAsync);

	UFUNCTION(BlueprintCallable, Category = "GameplayPersistance")
	void LoadGameFromDisk(bool bIsAsync);

	UFUNCTION()
	void OnGameSavedAsync(const FString& SlotName, const int32 UserIndex, bool bSuccess);

	UFUNCTION()
	void OnGameLoadedAsync(const FString& SlotName, const int32 UserIndex, USaveGame* SaveGameObject);

	UPROPERTY()
	TObjectPtr<UGameplaySaveGame> ActiveSaveGameObject;

private:
	bool bIsSavingAllowed = false;

	// Offset used to infer the current playtime when compared to GetRealTime.
	float LatestRealTimeValue;

	// The slot that will be saved to or loaded from.
	FString CurrentSlotName = "Slot1";

	// Unused since we don't support multiple players/profiles/accounts
	int CurrentUserIndex = 0;

	// Objects that want to partake in the persistance system, and thereby are 
	// loaded and saved when requested by game logic.
	UPROPERTY()
	TArray<TWeakObjectPtr<UObject>> RegisteredObjects;

public:

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable)
	FOnGameSavedToDiskSignature OnGameSavedToDiskDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnGameLoadedFromDiskSignature OnGameLoadedFromDiskDelegate;
};
