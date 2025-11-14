// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "GameplayPersistenceSubsystem.h"
#include "Kismet/GameplayStatics.h"

#include "GameplaySaveGame.h"
#include "DevelopmentTypes.h"

#include "SaveableObjectInterface.h"

// -- Begin USubSystem

void UGameplayPersistenceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	// Enables easier PIE, since a save object is grabbed immediately regardless of if the expected gameplay-loop was achieved
	LoadGameFromDisk(false);
}

// --- End USubSystem

void UGameplayPersistenceSubsystem::CreateSaveGameObject()
{  
   ActiveSaveGameObject = Cast<UGameplaySaveGame>(UGameplayStatics::CreateSaveGameObject(UGameplaySaveGame::StaticClass()));
}

void UGameplayPersistenceSubsystem::SaveGameToDisk(bool bIsAsync)
{	
	if (bIsSavingAllowed == false)
	{
		return;
	}

	ActiveSaveGameObject->SetPlayTime(GetPlayTime());

	if (bIsAsync)
	{
		FAsyncSaveGameToSlotDelegate SaveGameAsyncDelegate;
		SaveGameAsyncDelegate.BindUObject(this, &UGameplayPersistenceSubsystem::OnGameSavedAsync);

		UGameplayStatics::AsyncSaveGameToSlot(ActiveSaveGameObject, CurrentSlotName, CurrentUserIndex, SaveGameAsyncDelegate);
	}
	else
	{
		UGameplayStatics::SaveGameToSlot(ActiveSaveGameObject, CurrentSlotName, CurrentUserIndex);

		OnGameSavedToDiskDelegate.Broadcast(ActiveSaveGameObject);	
	}
}

void UGameplayPersistenceSubsystem::LoadGameFromDisk(bool bIsAsync)
{
	const bool bSaveGameExists = UGameplayStatics::DoesSaveGameExist(CurrentSlotName, CurrentUserIndex);
	if (bSaveGameExists == false)
	{
		GS_LOG(Error, TEXT("No Save exists in slot '%s'."), *CurrentSlotName);
		return;
	}

	if (bIsAsync)
	{
		FAsyncLoadGameFromSlotDelegate LoadGameAsyncDelegate;
		LoadGameAsyncDelegate.BindUObject(this, &UGameplayPersistenceSubsystem::OnGameLoadedAsync);

		UGameplayStatics::AsyncLoadGameFromSlot(CurrentSlotName, CurrentUserIndex, LoadGameAsyncDelegate);
	}
	else
	{
		ActiveSaveGameObject = Cast<UGameplaySaveGame>(UGameplayStatics::LoadGameFromSlot(CurrentSlotName, CurrentUserIndex));
		OnGameLoadedFromDiskDelegate.Broadcast(ActiveSaveGameObject);
	}
}

void UGameplayPersistenceSubsystem::RequestSaveToDisk(bool bIsAsync)
{
	if (IsValid(ActiveSaveGameObject) == false)
	{
		GS_LOG(Error, TEXT("No SaveGameObject is currently active, cannot save to disk. Did you call CreateSaveGameObject?"));
		return;
	}

	if (bIsSavingAllowed == true)
	{
		// Save all valid objects and mark invalid ones for removal
		TArray<TWeakObjectPtr<UObject>> InvalidObjects;

		for (TWeakObjectPtr<UObject> WeakObject : RegisteredObjects)
		{
			UObject* Object = WeakObject.Get();
			if (Object)
			{
				ISaveableObjectInterface::Execute_SaveToObject(Object, ActiveSaveGameObject);
			}
			else
			{
				InvalidObjects.Add(WeakObject);
			}
		}

		// Clean up any invalid WeakPtr's
		for (TWeakObjectPtr<UObject> ObjectToRemove : InvalidObjects)
		{
			RegisteredObjects.Remove(ObjectToRemove);
		}

		SaveGameToDisk(bIsAsync);
	}
}

void UGameplayPersistenceSubsystem::RequestSaveObject(UObject* Object)
{
	if (IsValid(ActiveSaveGameObject))
	{
		ISaveableObjectInterface::Execute_SaveToObject(Object, ActiveSaveGameObject);
	}
}

void UGameplayPersistenceSubsystem::RequestLoadObject(UObject* Object)
{
	if (IsValid(ActiveSaveGameObject))
	{
		ISaveableObjectInterface::Execute_LoadFromObject(Object, ActiveSaveGameObject);
	}
}

void UGameplayPersistenceSubsystem::RegisterObject(UObject* Object)
{  
	TWeakObjectPtr<UObject> WeakObject = Object;
	RegisteredObjects.AddUnique(WeakObject);  
}

void UGameplayPersistenceSubsystem::RegisterAndLoadObject(UObject* Object)
{
	RegisterObject(Object);
	RequestLoadObject(Object);
}

void UGameplayPersistenceSubsystem::SetSavingState(bool bCanSave)
{
	bIsSavingAllowed = bCanSave;
}

void UGameplayPersistenceSubsystem::SetSlotName(FString SlotName)
{
	CurrentSlotName = SlotName;
}

UGameplaySaveGame* UGameplayPersistenceSubsystem::GetSaveGameObject() const
{
	return ActiveSaveGameObject;
}

float UGameplayPersistenceSubsystem::GetPlayTime()
{
	// Game instance has the same lifetime as this
	float RealTimeSeconds = UGameplayStatics::GetRealTimeSeconds(GetGameInstance());

	float NewPlayTime = ActiveSaveGameObject->GetPlayTime() + RealTimeSeconds - LatestRealTimeValue;

	LatestRealTimeValue = RealTimeSeconds;

	return NewPlayTime;
}

void UGameplayPersistenceSubsystem::OnGameSavedAsync(const FString& SlotName, const int32 UserIndex, bool bSuccess)
{
	OnGameSavedToDiskDelegate.Broadcast(ActiveSaveGameObject);
}

void UGameplayPersistenceSubsystem::OnGameLoadedAsync(const FString& SlotName, const int32 UserIndex, USaveGame* SaveGameObject)
{
	ActiveSaveGameObject = Cast<UGameplaySaveGame>(SaveGameObject);
	OnGameLoadedFromDiskDelegate.Broadcast(ActiveSaveGameObject);
}
