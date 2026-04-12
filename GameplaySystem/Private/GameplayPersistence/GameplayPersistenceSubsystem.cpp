// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayPersistenceSubsystem.h"
#include "SaveGameSerializer.h"
#include "EngineUtils.h"
#include "SaveableObjectInterface.h"
#include "SaveGameFunctionLibrary.h"
#include "SaveGameSettings.h"

FLevelState::FLevelState(bool bIsLoaded)
{
	this->bIsLoaded = bIsLoaded;
}

void FLevelState::RegisterActor(AActor* Actor)
{
	check(Actor);

	RegisteredActors.Add(MakeWeakObjectPtr(Actor));
}

void FLevelState::MarkActorDestroyed(AActor* Actor)
{
	FName ActorName = FName(LevelUtilities::GetActorName(Actor));

	RegisteredActors.Remove(MakeWeakObjectPtr(Actor));

	if (USaveGameFunctionLibrary::WasObjectLoaded(Actor))
	{
		DestroyedActors.Add(FSoftObjectPath(Actor));
	}
}

void FLevelState::Reset()
{
	RegisteredActors.Empty();
	DestroyedActors.Empty();
}

void FSaveGameState::AddDestroyedActor(AActor* Actor)
{
	LevelName Name = LevelUtilities::GetLevelNameFromLevel(Actor->GetLevel());

	FLevelState* Data = LevelData.Find(Name);
	if (Data)
	{
		Data->MarkActorDestroyed(Actor);
	}
}

void FSaveGameState::MarkLevelLoaded(ULevel* Level)
{
	LevelName Name = LevelUtilities::GetLevelNameFromLevel(Level);
	MarkLevelLoaded_Internal(Name);
}

void FSaveGameState::MarkLevelLoaded(TSoftObjectPtr<UWorld> WorldAsset)
{
	LevelName Name = LevelUtilities::GetLevelNameFromSoftObjectPtr(WorldAsset);
	MarkLevelLoaded_Internal(Name);
}

void FSaveGameState::MarkLevelUnloaded(ULevel* Level)
{
	LevelName Name = LevelUtilities::GetLevelNameFromLevel(Level);
	MarkLevelUnloaded_Internal(Name);
}

void FSaveGameState::MarkLevelUnloaded(TSoftObjectPtr<UWorld> WorldAsset)
{
	LevelName Name = LevelUtilities::GetLevelNameFromSoftObjectPtr(WorldAsset);
	MarkLevelUnloaded_Internal(Name);
}

// Note: We want to keep the entries themselves, so we don't get warnings for unloading levels never being marked as loaded.
void FSaveGameState::ResetState()
{
	for (auto& [Name, Data] : LevelData)
	{
		Data.Reset();
	}
}

void FSaveGameState::MarkLevelLoaded_Internal(LevelName Level)
{
	const USaveGameSettings* Settings = GetDefault<USaveGameSettings>();

	if (not Settings->CanSerializeLevel(Level))
	{
		return;
	}

	FLevelState& Data = LevelData.FindOrAdd(Level);
	Data.bIsLoaded = true;
}

void FSaveGameState::MarkLevelUnloaded_Internal(LevelName Level)
{
	const USaveGameSettings* Settings = GetDefault<USaveGameSettings>();

	if (not Settings->CanSerializeLevel(Level))
	{
		return;
	}

	FLevelState* Data = LevelData.Find(Level);

	ensureAlways(Data); // Data should have been created when loading the level through Core delegates.

	if (Data)
	{
		Data->bIsLoaded = false;
	}
}

bool FSaveGameState::TryRegisterActor(AActor* Actor)
{
	if (IsValid(Actor) && Actor->Implements<USaveableObjectInterface>())
	{
		if (ISaveableObjectInterface::Execute_IsGlobalData(Actor))
		{
			GlobalData.RegisterActor(Actor);
			return true;
		}

		LevelName Name = LevelUtilities::GetLevelNameFromLevel(Actor->GetLevel());

		if (FLevelState* Data = LevelData.Find(Name))
		{
			Data->RegisterActor(Actor);
			return true;
		}
	}

	return false;
}

LevelName LevelUtilities::GetLevelNameFromWorld(UWorld* World)
{
	if (!World)
	{
		return FName();
	}

	FString LevelName = World->GetMapName();
	LevelName.RemoveFromStart(World->StreamingLevelsPrefix);

	return FName(LevelName);
}

LevelName LevelUtilities::GetLevelNameFromLevel(ULevel* Level)
{
	if (!Level)
	{
		return FName();
	}

	TSoftObjectPtr<ULevel> Ptr = MakeSoftObjectPtr(Level);
	return GetLevelName_Internal(Ptr.ToSoftObjectPath());
}

LevelName LevelUtilities::GetLevelNameFromSoftObjectPtr(const TSoftObjectPtr<ULevel>& Object)
{
	if(Object.IsNull())
	{
		return LevelName();
	}

	return GetLevelName_Internal(Object.ToSoftObjectPath());
}

LevelName LevelUtilities::GetLevelNameFromSoftObjectPtr(const TSoftObjectPtr<UWorld>& Object)
{
	if (Object.IsNull())
	{
		return LevelName();
	}

	return GetLevelName_Internal(Object.ToSoftObjectPath());
}

LevelName LevelUtilities::GetLevelName_Internal(const FSoftObjectPath& Path)
{
	if (Path.IsNull())
	{
		return LevelName();
	}

	FName Name = Path.GetAssetFName();

	return Name;
}

FString LevelUtilities::GetActorName(AActor* Actor)
{
	return Actor->GetName();
}

FName LevelUtilities::GetActorNameFromSoftObjectPath(const FSoftObjectPath& Path)
{
	FString ActorSubPath = Path.GetSubPathString();
	ActorSubPath.RemoveFromStart("PersistentLevel.");
	return FName(ActorSubPath);
}

void UGameplayPersistenceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::OnWorldInitialized);
	FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &ThisClass::OnActorsInitialized);

	FWorldDelegates::LevelAddedToWorld.AddUObject(this, &ThisClass::OnSublevelLoaded);
	FWorldDelegates::PreLevelRemovedFromWorld.AddUObject(this, &ThisClass::OnSublevelRemoved);

	FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &ThisClass::OnWorldBeginTearDown);

	OnWorldInitialized(GetWorld(), UWorld::InitializationValues());
}

void UGameplayPersistenceSubsystem::Deinitialize()
{
	FWorldDelegates::OnPostWorldInitialization.RemoveAll(this);
	FWorldDelegates::OnWorldInitializedActors.RemoveAll(this);
	FWorldDelegates::OnWorldCleanup.RemoveAll(this);

	FWorldDelegates::LevelAddedToWorld.RemoveAll(this);
	FWorldDelegates::PreLevelRemovedFromWorld.RemoveAll(this);

	Super::Deinitialize();
}

void UGameplayPersistenceSubsystem::SetIsSavingAllowed(bool bIsAllowed)
{
	bIsSavingAllowed = bIsAllowed;
}

float UGameplayPersistenceSubsystem::GetPlayTime()
{
	/*
	// Game instance has the same lifetime as this
	float RealTimeSeconds = UGameplayStatics::GetRealTimeSeconds(GetGameInstance());

	float NewPlayTime = ActiveSaveGameObject->GetPlayTime() + RealTimeSeconds - LatestRealTimeValue;

	LatestRealTimeValue = RealTimeSeconds;

	return NewPlayTime;
	*/
	return 0.0f;
}

void UGameplayPersistenceSubsystem::SaveGame(const FString& SlotName)
{	
	TSaveGameSerializer<false> BinarySerializer(this);
	bool bSuccess = BinarySerializer.SaveToDisk(SlotName);

#if !UE_BUILD_SHIPPING && WITH_TEXT_ARCHIVE_SUPPORT
	// This is for debug purposes only, we want to use binary serialization for smallest file sizes
	TSaveGameSerializer<false, true> TextSerializer(this);
	bSuccess &= TextSerializer.SaveToDisk(SlotName);
#endif

	GP_LOG(Log, TEXT("SaveGame completed for slot '%s' with result: %s"), *SlotName, bSuccess ? TEXT("Success") : TEXT("Failure"));
}

void UGameplayPersistenceSubsystem::LoadGame(const FString& SlotName)
{
	const TSharedRef<TSaveGameSerializer<true>> BinarySerializer = MakeShared<TSaveGameSerializer<true>>(this);
	CurrentSerializer = BinarySerializer.ToSharedPtr();

	// We want to build up the Sections from scratch since we are receiving completely new data
	BinaryScopeContainer.Reset();
	bool bSuccess = BinarySerializer->Load(SlotName);

	GP_LOG(Log, TEXT("LoadGame completed for slot '%s' with result: %s"), *SlotName, bSuccess ? TEXT("Success") : TEXT("Failure"));
}

void UGameplayPersistenceSubsystem::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues InitValues)
{
	check(World);

	World->AddOnActorPreSpawnInitialization(FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::OnActorPreSpawn, World));
	World->AddOnActorDestroyedHandler(FOnActorDestroyed::FDelegate::CreateUObject(this, &ThisClass::OnActorDestroyed, World));

	bIsSavingAllowed = true;
}

void UGameplayPersistenceSubsystem::OnActorsInitialized(const FActorsInitializedParams& Params)
{
	UWorld* World = Params.World;
	if (!IsValid(World) || GetWorld() != World)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;

		SaveGameState.TryRegisterActor(Actor);
	}
}

void UGameplayPersistenceSubsystem::OnWorldBeginTearDown(UWorld* World)
{
	bIsSavingAllowed = false;
}

void UGameplayPersistenceSubsystem::OnSublevelLoaded(ULevel* Level, UWorld* World)
{
	check(Level && World);

	SaveGameState.MarkLevelLoaded(Level);

	// We have an active Serializer - this is triggered from loading an existing savefile
	if (not CurrentSerializer.IsValid())
	{
		LoadSublevel(Level);
	}
}

void UGameplayPersistenceSubsystem::OnSublevelRemoved(ULevel* Level, UWorld* World)
{
	check(World);

	// Level is invalid when the persistent level is unloaded
	if (not Level)
	{
		return;
	}

	SaveGameState.MarkLevelUnloaded(Level);

	// We have an active Serializer, this is triggered from loading an existing savefile OR if saving is disabled, which happens when the world is being torn down
	if (not CurrentSerializer.IsValid() && bIsSavingAllowed)
	{
		SaveSublevel(Level);
	}
}

void UGameplayPersistenceSubsystem::OnActorPreSpawn(AActor* Actor, UWorld* World)
{
	SaveGameState.TryRegisterActor(Actor);
}

void UGameplayPersistenceSubsystem::OnActorDestroyed(AActor* Actor, UWorld* World)
{
	SaveGameState.AddDestroyedActor(Actor);
}

void UGameplayPersistenceSubsystem::OnLoadCompleted()
{
	CurrentSerializer = nullptr;
}

void UGameplayPersistenceSubsystem::SaveSublevel(ULevel* Level)
{
	TSaveGameSerializer<false> BinarySerializer(this);
	bool bSuccess = BinarySerializer.SerializeSublevelSingle(Level);

	LevelName Name = LevelUtilities::GetLevelNameFromLevel(Level);
	GP_LOG(Log, TEXT("Save Sublevel completed for '%s' with result: %s"), *Name.ToString(), bSuccess ? TEXT("Success") : TEXT("Failure"));
}

void UGameplayPersistenceSubsystem::LoadSublevel(ULevel* Level)
{
	TSaveGameSerializer<true> BinarySerializer(this);
	bool bSuccess = BinarySerializer.SerializeSublevelSingle(Level);

	LevelName Name = LevelUtilities::GetLevelNameFromLevel(Level);
	GP_LOG(Log, TEXT("Load Sublevel completed for '%s' with result: %s"), *Name.ToString(), bSuccess ? TEXT("Success") : TEXT("Failure"));
}