// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayPersistenceSubsystem.h"
#include "SaveGameSerializer.h"
#include "EngineUtils.h"
#include "SaveableObjectInterface.h"
#include "SaveGameFunctionLibrary.h"
#include "SaveGameSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameplaySystemComponent.h"
#include "GameFramework/Character.h"
#include "SaveGameVersion.h"

FSaveGameStatus::FSaveGameStatus(UWorld* World)
{
	check(World);

	if (ACharacter* Player = UGameplayStatics::GetPlayerCharacter(World, 0))
	{
		if (UGameplaySystemComponent* GS = UGameplaySystemComponent::GetGameplaySystemFromActor(Player))
		{
			PlayerLevel = GS->GetEntityLevel();
		}
	}

	if (UGameplayPersistenceSubsystem* GPS = UGameplayPersistenceSubsystem::Get(World))
	{
		Playtime = GPS->GetPlaytime();
	}

	Condition = ESaveGameCondition::ESGC_Empty;
}

void FSaveGameStatus::ApplyStateOnWorld(UWorld* World)
{
	if (UGameplayPersistenceSubsystem* GPS = UGameplayPersistenceSubsystem::Get(World))
	{
		GPS->SetPlaytime(Playtime);
	}
}

FSaveGameStatus FSaveGameStatus::MakeEmpty()
{
	FSaveGameStatus Status;
	Status.Condition = ESaveGameCondition::ESGC_Empty;

	return Status;
}

void FSaveGameStatus::UpdateConditionFromArchive(FArchive& Archive)
{
	const FCustomVersionContainer& CustomVersion = Archive.GetCustomVersions();
	if (const FCustomVersion* Version = CustomVersion.GetVersion(FSaveGameVersion::GUID))
	{
		if (Version->Version == FSaveGameVersion::LatestVersion)
		{
			Condition = ESaveGameCondition::ESGC_Healthy;
		}
		else
		{
			Condition = ESaveGameCondition::ESGC_Outdated;
		}
	}
	else
	{
		Condition = ESaveGameCondition::ESGC_Invalid;
	}
}

void operator<<(FArchive& Ar, FSaveGameStatus& Status)
{
	FStructuredArchiveFromArchive(Ar).GetSlot() << Status;
}

void operator<<(FStructuredArchive::FSlot Slot, FSaveGameStatus& Status)
{
	FArchive& BaseArchive = Slot.GetUnderlyingArchive();
	if (BaseArchive.IsTextFormat())
	{
		Slot << Status.PlayerLevel;
		Slot << Status.Playtime;
	}
	else
	{
		FStructuredArchive::FRecord Record = Slot.EnterRecord();
		Record << SA_VALUE(TEXT("PlayerLevel"), Status.PlayerLevel);
		Record << SA_VALUE(TEXT("Playtime"), Status.Playtime);
	}
}

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

	// Only check if the Actor is loaded if not marked as ShouldAlwaysTrackDestroyed
	if (!Actor->Implements<USaveableObjectInterface>() || !ISaveableObjectInterface::Execute_ShouldAlwaysTrackDestroyed(Actor))
	{
		if (not USaveGameFunctionLibrary::WasObjectLoaded(Actor))
		{
			return;
		}
	}

	DestroyedActors.Add(FSoftObjectPath(Actor));
}

void FLevelState::Reset()
{
	RegisteredActors.Empty();
	DestroyedActors.Empty();
}

void FLevelState::RemoveStaleActors()
{
	TArray<TWeakObjectPtr<AActor>> StaleObjects;

	for (const auto& RegisteredActor : RegisteredActors)
	{
		// We clacify the object as stale if the Actor is the WeakObjectPtr is stale or IsValid returns false.
		if (AActor* Actor = RegisteredActor.Get())
		{
			if (IsValid(Actor))
			{
				continue;
			}
		}

		StaleObjects.Emplace(RegisteredActor);
	}

	for (const auto& StaleObject : StaleObjects)
	{
		RegisteredActors.Remove(StaleObject);
	}
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

void FSaveGameState::AddLevel(LevelName Level, bool bIsLoaded)
{
	const USaveGameSettings* Settings = GetDefault<USaveGameSettings>();

	if (not Settings->CanSerializeLevel(Level))
	{
		return;
	}

	if (not LevelData.Contains(Level))
	{
		FLevelState State;
		State.bIsLoaded = bIsLoaded;
		State.bIsPredictedState = true;
		LevelData.Emplace(Level, State);
	}
}

// Note: We want to keep the entries themselves, so we don't get warnings for unloading levels never being marked as loaded.
void FSaveGameState::ResetState()
{
	for (auto& [Name, Data] : LevelData)
	{
		Data.Reset();
	}

	GlobalData.Reset();
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
	Data.bIsPredictedState = false;
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
		Data->bIsPredictedState = false;
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

UGameplayPersistenceSubsystem* UGameplayPersistenceSubsystem::Get(const UObject* WorldContext)
{
	if (WorldContext)
	{
		if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContext))
		{
			UGameplayPersistenceSubsystem* PersistenceSubsystem = GameInstance->GetSubsystem<UGameplayPersistenceSubsystem>();
			ensure(PersistenceSubsystem);

			return PersistenceSubsystem;
		}
	}

	return nullptr;
}

void UGameplayPersistenceSubsystem::SetIsSavingAllowed(bool bIsAllowed)
{
	bIsSavingAllowed = bIsAllowed;
}

double UGameplayPersistenceSubsystem::GetPlaytime()
{
	double RealTimeSeconds = UGameplayStatics::GetRealTimeSeconds(this);

	Playtime += RealTimeSeconds - RealTimeValueTimestamp;

	RealTimeValueTimestamp = RealTimeSeconds;

	return Playtime;
}

void UGameplayPersistenceSubsystem::SetPlaytime(double InPlaytime)
{
	Playtime = InPlaytime;
	RealTimeValueTimestamp = UGameplayStatics::GetRealTimeSeconds(this);;
}

void UGameplayPersistenceSubsystem::SaveGame(const FString& SlotName)
{	
	TSaveGameSerializer<false> BinarySerializer(this);
	bool bSuccess = BinarySerializer.SaveToDisk(SlotName);

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

FSaveGameStatus UGameplayPersistenceSubsystem::GetSaveGameStatus(const FString& SlotName)
{
	// The scopes will be overwritten with whatever values we receive when serializing the Status
	// Lets copy it and then replace the modified container when we are done.
	FArchiveSectionContainer ScopesCopy = BinaryScopeContainer;

	const TSharedRef<TSaveGameSerializer<true>> BinarySerializer = MakeShared<TSaveGameSerializer<true>>(this);
	FSaveGameStatus Status = FSaveGameStatus::MakeEmpty();
	const bool bIsValid = BinarySerializer->LoadStatus(SlotName, Status);

	BinaryScopeContainer = ScopesCopy;

	return Status;
}

bool UGameplayPersistenceSubsystem::IsGameLoadInProgress()
{
	return CurrentSerializer.IsValid();
}

void UGameplayPersistenceSubsystem::ClearData()
{
	check(not CurrentSerializer.IsValid()); // We are currently loading. This will cause crashes or undefined behaviour.

	RealTimeValueTimestamp = 0.0;
	Playtime = 0.0;
	SaveGameState.ResetState();
	BinaryScopeContainer.Reset();
	BinaryData.Empty();

#if WITH_TEXT_ARCHIVE_SUPPORT
	TextScopeContainer.Reset();;
	TextData.Empty();
#endif //WITH_TEXT_ARCHIVE_SUPPORT
}

void UGameplayPersistenceSubsystem::OnWorldInitialized(UWorld* World, const UWorld::InitializationValues InitValues)
{
	check(World);

	// Note: We need to preadd the possible streaming levels when initializing world, as OnSublevelLoaded and
	//		 OnSublevelRemoved are only triggered after loading and unloading respectively, and we need the levels registered before that.
	TArray<ULevelStreaming*> StreamingLevels = World->GetStreamingLevels();
	for (auto Level : StreamingLevels)
	{
		SaveGameState.AddLevel(LevelUtilities::GetLevelNameFromSoftObjectPtr(Level->GetWorldAsset()), Level->IsLevelLoaded());
	}
	
	World->AddOnPostRegisterAllActorComponentsHandler(FOnPostRegisterAllActorComponents::FDelegate::CreateUObject(this, &ThisClass::OnActorPreSpawn, World));
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
	OnGameLoadedDelegate.Broadcast();
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
