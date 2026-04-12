// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "SaveGameSerializer.h"
#include "SaveGameSystem.h"
#include "PlatformFeatures.h"
#include "SaveGameVersion.h"
#include "Kismet/GameplayStatics.h"
#include "DevelopmentTypes.h"
#include "SaveableObjectInterface.h"
#include "SaveGameSettings.h"
#include "SaveGameFunctionLibrary.h"

#define LEVEL_SUBPATH_PREFIX TEXT("PersistentLevel.")
#define GET_SCOPE(Name, Type) SectionContainer, Name, FArchiveSection::EScopeType::Type
#define MAKE_SCOPE(Name, Type) FSerializationScope(*this, GET_SCOPE(Name, Type))

#define FILE_AND_LINE_STRING() (*FString::Printf(TEXT("%s, %d"), TEXT(__FILE__), __LINE__))

// Expects to be renamed using Rename() to a valid Section name.
#define MAKE_SCOPE_TEMP(Type) MAKE_SCOPE(FILE_AND_LINE_STRING(), Type)

template<bool bIsLoading, bool bIsTextFormat>
inline TSaveGameSerializer<bIsLoading, bIsTextFormat>::FSerializationScope::FSerializationScope(TSaveGameSerializer& InSerializer, FArchiveSectionContainer& InContainer, const FString& ScopeName, FArchiveSection::EScopeType Type)
	: Serializer(InSerializer),	Container(InContainer), Section(InContainer.GetOrAddSection(ScopeName, Type))
{
	Section.StartOffset = Serializer.Archive.Tell();

	// If we are starting from a default initialized Section, StartOffset will be more than Endoffset if we don't assign both
	if (Section.EndOffset == INVALID_SECTION_OFFSET)
	{
		Section.EndOffset = Section.StartOffset;
	}

	// Copy over data that we risk overwriting
	if (SAVING)
	{
		const uint64 Size = Serializer.Data.Num() - Section.EndOffset;
		if (Size != 0U)
		{
			check(Section.EndOffset < Serializer.Data.Num());

			check(Size <= INT32_MAX); // The data we want to copy is larger than what the array can hold.

			CopiedData.Reset();
			CopiedData.Reserve(int32(Size));

			for (uint64 Idx = Section.EndOffset; Idx < Serializer.Data.Num(); Idx++)
			{
				CopiedData.Emplace(Serializer.Data[int32(Idx)]);
			}
		}
	}

	// Prewrite our size
	{
		uint64 OffsetSize;

		if (SAVING)
		{
			OffsetSize = Section.GetSize();

			// If there is already data there, skip writing and move a uint64 over instead.
			// Note: This is important for when we Terminate and avoid overwriting data. If we write here, we will zero out any existing OffsetSize and
			//		 ruin data, when we can just skip it and write over it if we don't get Terminate'd in the destructor. Win-win.
			uint64 Destination = 0U;
			check(FMath::AddAndCheckForOverflow(uint64(Serializer.Archive.Tell()), sizeof(uint64), Destination));
			if (Serializer.Data.Num() >= Destination)
			{
				Serializer.Archive.Seek(Destination);
				return;
			}
		}

		Serializer.Archive << OffsetSize;

		if (LOADING)
		{
			Section.EndOffset = Section.StartOffset + OffsetSize;
			return;
		}
	}
}

template<bool bIsLoading, bool bIsTextFormat>
TSaveGameSerializer<bIsLoading, bIsTextFormat>::FSerializationScope::~FSerializationScope()
{
	if (LOADING)
	{
		Serializer.Archive.Seek(Section.EndOffset);

		// This is otherwise unset and will cause issues with shifting data later.
		if (Section.PreviousSize == 0)
		{
			Section.PreviousSize = Section.GetSize(); 
		}

		Container.RenameSection(Section, OptionalNewName);
		return;
	}

	if (bIsTerminated)
	{
		// We were meant to be renamed, but won't be as to avoid overwriting data. Remove our offset.
		if (not OptionalNewName.IsEmpty())
		{
			ensure(Container.RemoveSection(Container.GetSectionName(Section)));
		}

		return;
	}

	// We could have a Section that we will overwrite when renaming. Copy over properties so that we replace it correctly when renaming
	if (Container.HasSection(OptionalNewName))
	{
		const FArchiveSection* Predecessor = Container.GetSection(OptionalNewName);
		check(Predecessor);
		check(Predecessor->PreviousSize != 0);

		Section.StartOffset = Predecessor->StartOffset;
		Section.PreviousSize = Predecessor->PreviousSize;

		// When we have an existing Section with the same name, we start serializing from it's StartOffset, see SerializeSublevelSingle
		// This means that we have some of it's old data in CopiedData that we need to remove.
		if (Predecessor->PreviousSize > 0)
		{
			// We will go out of Section soon, don't resize
			CopiedData.RemoveAt(0, Predecessor->PreviousSize, false /* bAllowShrinking */); 
		}
	}

	Section.EndOffset = Serializer.Archive.Tell();
	Section.MarkSizeChanged();

	// Go back and write our size 
	{
		if (SAVING)
		{
			Serializer.Archive.Seek(Section.StartOffset);

			uint64 Size = Section.GetSize();
			Serializer.Archive << Size;

			Serializer.Archive.Seek(Section.EndOffset);
		}
	}
	
	// Copy back data now that we are done modifying the Archive
	if (not CopiedData.IsEmpty())
	{
		uint64 TotalSize = 0U;
		check(FMath::AddAndCheckForOverflow(uint64(Section.EndOffset), uint64(CopiedData.Num()), TotalSize));

		check(TotalSize <= INT32_MAX); // The new total size will exceed what the array can hold.

		Serializer.Data.SetNumZeroed(int32(TotalSize), EAllowShrinking::Yes);

		int32 InsertIdx = Section.EndOffset;
		for (int32 Idx = 0; Idx < CopiedData.Num(); Idx++)
		{
			Serializer.Data[InsertIdx] = CopiedData[Idx];
			InsertIdx++;
		}
	}

	check(Section.PreviousSize != 0);

	// This will invalidate Section, but we are going out of Section after this
	if (not OptionalNewName.IsEmpty())
	{
		CopiedData.Empty(); // Empty so we don't copy the data around too
		Container.RenameSection(Section, OptionalNewName); 
	}
}

template<bool bIsLoading, bool bIsTextFormat>
void TSaveGameSerializer<bIsLoading, bIsTextFormat>::FSerializationScope::Rename(FString NewName)
{
	OptionalNewName = NewName;
}

template<bool bIsLoading, bool bIsTextFormat>
void TSaveGameSerializer<bIsLoading, bIsTextFormat>::FSerializationScope::Terminate()
{
	bIsTerminated = true;
}

template<bool bIsLoading, bool bIsTextFormat>
TSaveGameSerializer<bIsLoading, bIsTextFormat>::TSaveGameSerializer(UGameplayPersistenceSubsystem* InPersistenceSubsystem)
	: PersistenceSubsystem(InPersistenceSubsystem)
	, SectionContainer(GetSectionContainer())
	, Data(GetRemoteData())
	, Archive(Data)
	, ProxyArchive(Archive)
	, Formatter(ProxyArchive)
	, StructuredArchive(Formatter)
	, RootSlot(StructuredArchive.Open())
	, RootRecord(RootSlot.EnterRecord())
{
	static_cast<FArchive&>(ProxyArchive).SetIsTextFormat(bIsTextFormat);

	// Ensure that we're using the latest save game version
	Archive.UsingCustomVersion(FSaveGameVersion::GUID);
}

template <bool bIsLoading, bool bIsTextFormat>
bool TSaveGameSerializer<bIsLoading, bIsTextFormat>::Save()
{
	check(SAVING);

	SerializeHeader();

	SerializeSublevels();

 	if (!bIsTextFormat)
	{
		// Store the version position so that we can serialize it in the header
		VersionOffset = Archive.Tell();
	}

	SerializeVersions();

	if (!bIsTextFormat)
	{
		// We've updated the VersionOffset, let's go back to the start and rewrite the header
		Archive.Seek(0);
		SerializeHeader();
	}

	// Be sure to close this, as you'll be missing closed braces for JSON archives
	StructuredArchive.Close();

	SetRemoteData();

	return true;
}

template<bool bIsLoading, bool bIsTextFormat>
bool TSaveGameSerializer<bIsLoading, bIsTextFormat>::SaveToDisk(const FString& SlotName)
{
	check(SAVING);

	const bool bResult = Save();

	if (bResult)
	{
		if (ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem())
		{
			// Compress the save game data
			if (USE_FILE_COMPRESSION && not bIsTextFormat)
			{
				TArray<uint8> CompressedData;
				FSaveGameMemoryArchive CompressorArchive(CompressedData);
				SerializeCompressedData<bIsLoading>(CompressorArchive, Data);

				return SaveSystem->SaveGame(false /* bAttemptToUseUI */, *GetSaveName(SlotName), USER_SLOT_INDEX, CompressedData);
			}

			return SaveSystem->SaveGame(false  /* bAttemptToUseUI */, *GetSaveName(SlotName), USER_SLOT_INDEX, Data);
		}
	}

	return false;
}

template <bool bIsLoading, bool bIsTextFormat>
bool TSaveGameSerializer<bIsLoading, bIsTextFormat>::Load(const FString& SlotName)
{
	check(LOADING);

	TArray<uint8> CompressedData;
	ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	if (SaveSystem && SaveSystem->LoadGame(false /* bAttemptToUseUI */, *GetSaveName(SlotName), USER_SLOT_INDEX, CompressedData))
	{
		if (USE_FILE_COMPRESSION)
		{
			// Decompress the loaded save game data
			FSaveGameMemoryArchive CompressorArchive(CompressedData);
			SerializeCompressedData<bIsLoading>(CompressorArchive, Data);
		}
		else
		{
			Data = CompressedData;
		}

		SerializeHeader();

		{
			const uint64 InitialPosition = Archive.Tell();

			// After serializing versions, go back to initial position
			ON_SCOPE_EXIT
			{
				Archive.Seek(InitialPosition);
			};

			Archive.Seek(VersionOffset);
			SerializeVersions();
		}

		// If we don't have a map, we should bail
		if (MapName.IsNone())
		{
			GP_LOG(Error, TEXT("No MapName found in SaveFile!"));
			return false;
		}

		check(PersistenceSubsystem.IsValid());
		UWorld* World = PersistenceSubsystem->GetWorld();

		// When our map has loaded, call the OnMapLoad method and resume loading from there
		FCoreUObjectDelegates::PostLoadMapWithWorld.AddThreadSafeSP(this, &TSaveGameSerializer::OnMapLoad);

		UGameplayStatics::OpenLevel(PersistenceSubsystem.Get(), MapName, true);

		return true;
	}

	return false;
}

template<bool bIsLoading, bool bIsTextFormat>
bool TSaveGameSerializer<bIsLoading, bIsTextFormat>::SerializeSublevelSingle(ULevel* Level)
{
	UWorld* World = PersistenceSubsystem->GetWorld();
	LevelName Name = LevelUtilities::GetLevelNameFromLevel(Level);
	FString NameAsString = Name.ToString();

	if (IsDataEmpty())
	{
		// Make a valid save file with current game state before we proceed.
		TSaveGameSerializer<false, bIsTextFormat> Serializer(PersistenceSubsystem.Get());
		bool bSuccess = Serializer.Save();

		GP_LOG(Log, TEXT("Creating SaveGame for requester Sublevel '%s' finished with result: %s"), *Name.ToString(), bSuccess ? TEXT("Success") : TEXT("Failure"));

		Data = GetRemoteData();

		if (IsDataEmpty())
		{
			GP_LOG(Error, TEXT("Tried to Single Serialize Sublevel '%s' with no previous SaveData on record!"), *Name.ToString());
			return false;
		}
	}

	const USaveGameSettings* Settings = GetDefault<USaveGameSettings>();

	if (not Settings->CanSerializeLevel(Name))
	{
		GP_LOG(Log, TEXT("Serializing Sublevel '%s' skipped due to SaveGame Settings."), *Name.ToString());
		return true;
	}

	if (LOADING)
	{
		if (not SectionContainer.HasSection(NameAsString))
		{
			GP_LOG(Log, TEXT("Loading Sublevel '%s' skipped due to no previous SaveData existing for it."), *Name.ToString());
			return true;
		}
	}

	FLevelState& LevelData = PersistenceSubsystem->SaveGameState.LevelData.FindChecked(Name);

	int32 NumSublevels;

	if (SAVING)
	{
		NumSublevels = PersistenceSubsystem->SaveGameState.LevelData.Num();
	}

	// Seek to after Header data so we serialize in the right place
	const FArchiveSection* HeaderOffset = SectionContainer.GetLastSectionOfType(FArchiveSection::EScopeType::Header);
	check(HeaderOffset);
	Archive.Seek(HeaderOffset->EndOffset);

	FStructuredArchive::FMap LevelMap = RootRecord.EnterMap(TEXT("Sublevels"), NumSublevels);

	// Find the Sublevels offset to serialize at if we have serialized it before
	const FArchiveSection* LevelOffsets = SectionContainer.GetSection(NameAsString);
	if (LevelOffsets)
	{
		Archive.Seek(LevelOffsets->StartOffset);
	}
	else if(const FArchiveSection* LastSublevelOffsets = SectionContainer.GetLastSectionOfType(FArchiveSection::EScopeType::Sublevel))
	{
		// Serialize from the end of the last Sublevel entry
		Archive.Seek(LastSublevelOffsets->EndOffset);
	}

	SerializeSublevel(LevelMap, World, Name, &LevelData, true /* bUseGameState */);

	SetRemoteData();

	return true;
}

template<bool bIsLoading, bool bIsTextFormat>
const TArray<uint8> TSaveGameSerializer<bIsLoading, bIsTextFormat>::GetRemoteData()
{
#if WITH_TEXT_ARCHIVE_SUPPORT
	if (bIsTextFormat)
	{
		return PersistenceSubsystem->TextData;
	}
#endif //WITH_TEXT_ARCHIVE_SUPPORT

	return PersistenceSubsystem->BinaryData;
}

template<bool bIsLoading, bool bIsTextFormat>
void TSaveGameSerializer<bIsLoading, bIsTextFormat>::SetRemoteData() const
{
#if WITH_TEXT_ARCHIVE_SUPPORT
	if (bIsTextFormat)
	{
		PersistenceSubsystem->TextData = Data;
	}
#endif //WITH_TEXT_ARCHIVE_SUPPORT

	PersistenceSubsystem->BinaryData = Data;
}

template<bool bIsLoading, bool bIsTextFormat>
FArchiveSectionContainer& TSaveGameSerializer<bIsLoading, bIsTextFormat>::GetSectionContainer() const
{
#if WITH_TEXT_ARCHIVE_SUPPORT
	if (bIsTextFormat)
	{
		return PersistenceSubsystem->TextScopeContainer;
	}
#endif //WITH_TEXT_ARCHIVE_SUPPORT

	return PersistenceSubsystem->BinaryScopeContainer;
}

template <bool bIsLoading, bool bIsTextFormat>
FString TSaveGameSerializer<bIsLoading, bIsTextFormat>::GetSaveName(const FString& SlotName)
{
	FString SaveName = TEXT("AASaveGame"); // TODO: Switch out with slot specific

	SaveName.Append(SlotName);

	if (bIsTextFormat)
	{
		SaveName += TEXT(".json");
	}

	return SaveName;
}

template<bool bIsLoading, bool bIsTextFormat>
bool TSaveGameSerializer<bIsLoading, bIsTextFormat>::IsDataEmpty() const
{
	if (bIsTextFormat)
	{
		// JSON always inserts a opening '{' in the data.
		return Data.Num() <= 1; 
	}
	else
	{
		return Data.IsEmpty();
	}
}

template <bool bIsLoading, bool bIsTextFormat>
void TSaveGameSerializer<bIsLoading, bIsTextFormat>::OnMapLoad(UWorld* World)
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);
	check(PersistenceSubsystem->GetWorld() == World);

	SerializeSublevels();

	SetRemoteData();

	PersistenceSubsystem->OnLoadCompleted();
}

template<bool bIsLoading>
FORCEINLINE_DEBUGGABLE void SerializeCompressedData(FArchive& Ar, TArray<uint8>& Data)
{
	check(Ar.IsLoading() == bIsLoading);

	int64 UncompressedSize;

	if (SAVING)
	{
		UncompressedSize = Data.Num();
	}

	Ar << UncompressedSize;

	if (LOADING)
	{
		Data.SetNumUninitialized(UncompressedSize);
	}

	Ar.SerializeCompressedNew(Data.GetData(), UncompressedSize);
}

template <bool bIsLoading, bool bIsTextFormat>
void TSaveGameSerializer<bIsLoading, bIsTextFormat>::SerializeHeader()
{
	FSerializationScope Section = MAKE_SCOPE("Header", Header);

	const UWorld* World = PersistenceSubsystem->GetWorld();

	// If we already have a map name, don't change it
	if (SAVING && MapName.IsNone())
	{
		FString LevelPackageName = World->GetOutermost()->GetLoadedPath().GetPackageName();
		MapName = FName(LevelPackageName);
	}

	RootRecord << SA_VALUE(TEXT("Map"), MapName);

	FEngineVersion EngineVersion;
	FPackageFileVersion PackageVersion;

	if (SAVING)
	{
		EngineVersion = FEngineVersion::Current();
		PackageVersion = GPackageFileUEVersion;
	}

	RootRecord << SA_VALUE(TEXT("EngineVersion"), EngineVersion);

	if (!bIsTextFormat)
	{
		// This doesn't have a structured archive serialize method, won't show up in JSON
		Archive << PackageVersion;

		// We're a binary archive, so let's serialize where the version is
		// so that we can read it before loading anything
		RootRecord << SA_VALUE(TEXT("VersionsOffset"), VersionOffset);
	}

	if (LOADING)
	{
		Archive.SetEngineVer(EngineVersion);
		Archive.SetUEVer(PackageVersion);
	}
}

template<bool bIsLoading, bool bIsTextFormat>
void TSaveGameSerializer<bIsLoading, bIsTextFormat>::SerializeSublevels()
{
	UWorld* World = PersistenceSubsystem->GetWorld();

	int32 NumSublevels;

	if (SAVING)
	{
		NumSublevels = PersistenceSubsystem->SaveGameState.LevelData.Num();
	}

	FStructuredArchive::FMap LevelMap = RootRecord.EnterMap(TEXT("Sublevels"), NumSublevels);

	if (LOADING)
	{
		PersistenceSubsystem->SaveGameState.ResetState();
		PersistenceSubsystem->SaveGameState.LevelData.Reserve(NumSublevels);
	}

	auto LevelDataIt = PersistenceSubsystem->SaveGameState.LevelData.CreateIterator();
	for (int32 SubLevelIndex = 0; SubLevelIndex < NumSublevels; ++SubLevelIndex)
	{
		ON_SCOPE_EXIT
		{
			if (SAVING)
			{
				++LevelDataIt;
			}
		};

		FLevelState* LevelData = nullptr;
		LevelName Name;
		
		if (SAVING)
		{
			LevelData = &LevelDataIt.Value();
			Name = LevelDataIt.Key();
		}

		SerializeSublevel(LevelMap, World, Name, LevelData, false /* bUseGameState */);
	}
	
	// Note: It's important that we do this last, in case any sublevels that are serialized contain global data Actors that we want to find.
	SerializeGlobals(World);
}

template<bool bIsLoading, bool bIsTextFormat>
void TSaveGameSerializer<bIsLoading, bIsTextFormat>::SerializeSublevel(FStructuredArchive::FMap& LevelMap, UWorld* World, const LevelName& Level, FLevelState* LevelData, bool bUseGameState)
{
	FSerializationScope Section = MAKE_SCOPE_TEMP(Sublevel);

	FString LevelNameAsString;

	if (SAVING)
	{
		LevelNameAsString = Level.ToString();
	}

	FStructuredArchive::FSlot SublevelSlot = LevelMap.EnterElement(LevelNameAsString);
	FStructuredArchive::FRecord SublevelRecord = SublevelSlot.EnterRecord();

	Section.Rename(LevelNameAsString);

	bool bIsLevelLoaded;

	if (SAVING)
	{
		bIsLevelLoaded = LevelData->bIsLoaded;
	}

	SublevelRecord << SA_VALUE(TEXT("IsLoaded"), bIsLevelLoaded);

	ULevelStreaming* LevelStreaming = GetSublevelByName(World, LevelName(LevelNameAsString));

	// We don't care about the serialized state, use the actual level state (otherwise we enforce serialized state, undoing any in-game state)
	if (bUseGameState)
	{
		if (LevelStreaming)
		{
			bIsLevelLoaded = LevelStreaming->IsLevelLoaded();
		}
	}

	if (SAVING)
	{
		// Previous save data for it should already be in there, with bIsLevelLoaded updated to match
		if (not bIsLevelLoaded)
		{

			// Move to the end of our data before returning
			if (const FArchiveSection* SublevelScope = SectionContainer.GetSection(LevelNameAsString))
			{
				Archive.Seek(SublevelScope->EndOffset);
			}

			Section.Terminate();
			return;
		}
	}

	if (LOADING)
	{
		ensureAlways(LevelStreaming); // Should point to valid sublevel, unless the name is changed or sublevel was removed
		if (not LevelStreaming)		  // Might also be invalid if the sublevel is a sublevel of another level than the one we are saving
		{
			return;
		}

		TSoftObjectPtr<UWorld> WorldAsset = LevelStreaming->GetWorldAsset();

		// Load level
		if (bIsLevelLoaded)
		{
			if (not LevelStreaming->IsLevelLoaded())
			{
				FLatentActionInfo ActionInfo;
				ActionInfo.UUID = GetNextRequestID();

				UGameplayStatics::LoadStreamLevelBySoftObjectPtr(World, WorldAsset, true /* bMakeVisibleAfterLoad */, true /* bShouldBlockOnLoad */, ActionInfo);

				ensure(LevelStreaming->IsLevelLoaded());
			}
			else
			{
				PersistenceSubsystem->SaveGameState.MarkLevelLoaded(WorldAsset);
			}
		}
		else // Unload level
		{
			if (LevelStreaming->IsLevelLoaded())
			{
				FLatentActionInfo ActionInfo;
				ActionInfo.UUID = GetNextRequestID();

				UGameplayStatics::UnloadStreamLevelBySoftObjectPtr(World, WorldAsset, ActionInfo, true /* bShouldBlockOnUnload */);
				UGameplayStatics::FlushLevelStreaming(World); // Forces LevelStreaming to use bShouldBlockOnUnload and block the main thread, otherwise it's done asyncronously

				ensure(not LevelStreaming->IsLevelLoaded());
			}
			else
			{
				PersistenceSubsystem->SaveGameState.MarkLevelUnloaded(WorldAsset);
			}

			return;
		}
	}

	if (LOADING)
	{
		LevelData = PersistenceSubsystem->SaveGameState.LevelData.Find(LevelName(LevelNameAsString));
		check(LevelData); // Should have been created when the sublevel was loaded
	}

	SerializeActors(SublevelRecord, LevelStreaming->GetLoadedLevel(), World, true /* bSerializingSublevel */, *LevelData);
	SerializeDestroyedActors(SublevelRecord, LevelStreaming->GetLoadedLevel(), World, *LevelData);
}

template<bool bIsLoading, bool bIsTextFormat>
void TSaveGameSerializer<bIsLoading, bIsTextFormat>::SerializeGlobals(UWorld* World)
{
	FSerializationScope Section = MAKE_SCOPE("GlobalData", Global);

	FStructuredArchive::FSlot GlobalSlot = RootRecord.EnterField(GLOBAL_DATA);
	FStructuredArchive::FRecord GlobalRecord = GlobalSlot.EnterRecord();

	FLevelState& GlobalData = PersistenceSubsystem->SaveGameState.GlobalData;
	SerializeActors(GlobalRecord, World->GetCurrentLevel(), World, false /* bSerializingSublevel */, GlobalData);
	SerializeDestroyedActors(GlobalRecord, World->GetCurrentLevel(), World, GlobalData);
}

template<bool bIsLoading, bool bIsTextFormat>
void TSaveGameSerializer<bIsLoading, bIsTextFormat>::SerializeActors(FStructuredArchive::FRecord& Record, ULevel* Level, UWorld* World, bool bSerializingSublevel, FLevelState& LevelData)
{
	FTopLevelAssetPath LevelAssetPath;

	// Serializing sublevel data
	if (bSerializingSublevel)
	{
		LevelAssetPath = FTopLevelAssetPath(Level->GetPackage()->GetFName(), Level->GetOuter()->GetFName());
	}
	else // Serializing global data
	{
		LevelAssetPath = FTopLevelAssetPath(World->GetCurrentLevel()->GetPackage()->GetFName(), World->GetCurrentLevel()->GetOuter()->GetFName());
	}

	int32 NumActors;
	TArray<AActor*> Actors;
	TMap<FGuid, AActor*> SpawnIDs;

	const uint64 ActorsPosition = Archive.Tell();

	if (LOADING)
	{
		// Iterate through this Levels live Actors so that we can map their SpawnIDs
		for (const TWeakObjectPtr<AActor>& ActorPtr : LevelData.RegisteredActors)
		{
			AActor* Actor = ActorPtr.Get();
			if (IsValid(Actor) && Actor->Implements<USaveableObjectInterface>())
			{
				const FGuid SpawnID = ISaveableObjectInterface::Execute_GetSpawnID(Actor);

				if (SpawnID.IsValid())
				{
					SpawnIDs.Add(SpawnID, Actor);
				}
			}
		}

		FStructuredArchive::FMap ActorMap = Record.EnterMap(TEXT("LevelActors"), NumActors);

		Actors.SetNumZeroed(NumActors);

		// Iterate through the saved actors and spawn or find their live equivalent
		for (int32 ActorIdx = 0; ActorIdx < NumActors; ++ActorIdx)
		{
			AActor*& Actor = Actors[ActorIdx];

			// Populate our actors list with spawned actors or level references to actors
			SerializeActor(ActorMap, Actor, LevelData, [&](const FString& ActorName, const FSoftClassPath& Class, const FGuid& SpawnID, FStructuredArchive::FSlot&)
				{
					ensureAlways(!ActorName.IsEmpty());

					if (Class.IsNull())
					{
						// This is a loaded actor (is a level actor), let's find it
						Actor = FindObjectFast<AActor>(Level, *ActorName);
					}
					else if (SpawnID.IsValid() && SpawnIDs.Contains(SpawnID))
					{
						Actor = SpawnIDs[SpawnID];
					}
					else
					{
						UClass* ActorClass = Class.TryLoadClass<AActor>();

						// This is a spawned actor, let's spawn it
						FActorSpawnParameters SpawnParameters;

						SpawnParameters.OverrideLevel = Level;
						SpawnParameters.Name = *ActorName;
						SpawnParameters.bNoFail = true;

						Actor = World->SpawnActor(ActorClass, nullptr, nullptr, SpawnParameters);

						if (SpawnID.IsValid() && Actor->Implements<USaveableObjectInterface>())
						{
							ISaveableObjectInterface::Execute_SetSpawnID(Actor, SpawnID);
						}
					}

					if (SpawnID.IsValid())
					{
						const FString ActorSubPath = LEVEL_SUBPATH_PREFIX + ActorName;

						// We potentially have a spawned actor that other actors reference
						// If the name has changed, be sure to redirect the old actor path to the new one
						ProxyArchive.AddRedirect(FSoftObjectPath(LevelAssetPath, ActorSubPath), FSoftObjectPath(Actor));
					}

					check(IsValid(Actor));
				});
		}
	}
	else
	{
		NumActors = LevelData.RegisteredActors.Num();
	}

	{
		if (LOADING && !bIsTextFormat)
		{
			// Go back to the start of the actor data
			Archive.Seek(ActorsPosition);
		}

		FStructuredArchive::FMap ActorMap = Record.EnterMap(TEXT("LevelActors"), NumActors);

		auto ActorsIt = LevelData.RegisteredActors.CreateConstIterator();

		// Actually serialize the actor data and their properties
		for (int32 ActorIdx = 0; ActorIdx < NumActors; ++ActorIdx)
		{
			AActor* Actor;

			if (LOADING)
			{
				Actor = Actors[ActorIdx];
			}
			else
			{
				Actor = ActorsIt->Get();
				++ActorsIt;
			}

			check(IsValid(Actor));

			// Do the actual serialization of the properties
			SerializeActor(ActorMap, Actor, LevelData, [&](const FString&, const FSoftClassPath&, const FGuid& SpawnID, FStructuredArchive::FSlot& ActorSlot)
				{
					Actor->SerializeScriptProperties(ActorSlot.EnterAttribute(TEXT("Properties")));

					FStructuredArchive::FSlot CustomDataSlot = ActorSlot.EnterAttribute(TEXT("Data"));
					FStructuredArchive::FRecord CustomDataRecord = CustomDataSlot.EnterRecord();

					// Encapsulate the record in something a Blueprint can access 
					FSaveGameArchive SaveGameArchive(CustomDataRecord, Actor);

					// OnSerialize can't be implemented if the interface isn't natively implemented in which case this already fails
					if (ISaveableObjectInterface* Interface = Cast<ISaveableObjectInterface>(Actor))
					{
						Interface->OnSerialize(SaveGameArchive, bIsLoading);
					}

					// Always call after native OnSerialize
					ISaveableObjectInterface::Execute_K2_OnSerialize(Actor, SaveGameArchive, bIsLoading);
				});
		}
	}
}

template <bool bIsLoading, bool bIsTextFormat>
void TSaveGameSerializer<bIsLoading, bIsTextFormat>::SerializeDestroyedActors(FStructuredArchive::FRecord& Record, ULevel* Level, UWorld* World, FLevelState& LevelData)
{	
	int32 NumDestroyedActors;

	if (SAVING)
	{
		// We should be getting a valid level name from upstream
		NumDestroyedActors = LevelData.DestroyedActors.Num();
	}

	FStructuredArchive::FArray DestroyedActorsArray = Record.EnterArray(TEXT("DestroyedActors"), NumDestroyedActors);

	if (LOADING)
	{
		// Allocate our expected number of actors
		LevelData.DestroyedActors.Reset();
		LevelData.DestroyedActors.Reserve(NumDestroyedActors);
	}

	auto DestroyedActorsIt = LevelData.DestroyedActors.CreateConstIterator();
	for (int32 ActorIdx = 0; ActorIdx < NumDestroyedActors; ++ActorIdx)
	{
		FName ActorName;

		if (SAVING)
		{
			ActorName = LevelUtilities::GetActorNameFromSoftObjectPath(*DestroyedActorsIt);

			++DestroyedActorsIt;
		}

		DestroyedActorsArray.EnterElement() << ActorName;

		if (LOADING)
		{
			// Find the live actor in the level
			if (AActor* DestroyedActor = FindObjectFast<AActor>(Level, ActorName))
			{
				// Be sure to add any valid destroyed actors back in for saving later!
				PersistenceSubsystem->SaveGameState.AddDestroyedActor(DestroyedActor);

				DestroyedActor->Destroy();
			}
		}
	}
}

template <bool bIsLoading, bool bIsTextFormat>
void TSaveGameSerializer<bIsLoading, bIsTextFormat>::SerializeVersions()
{
	FSerializationScope Section = { *this, GET_SCOPE("Versions", Versions)};

	FCustomVersionContainer VersionContainer;

	if (SAVING)
	{
		// Grab a copy of our archive's current versions
		VersionContainer = Archive.GetCustomVersions();
	}

	VersionContainer.Serialize(RootRecord.EnterField(TEXT("Versions")));

	if (LOADING)
	{
		// Assign our serialized versions
		Archive.SetCustomVersions(VersionContainer);
	}
}

template<bool bIsLoading, bool bIsTextFormat>
ULevelStreaming* TSaveGameSerializer<bIsLoading, bIsTextFormat>::GetSublevelByName(UWorld* World, LevelName Name) const
{
	const TArray<ULevelStreaming*>& StreamingLevels = World->GetStreamingLevels();
	for (const auto& StreamingLevel : StreamingLevels)
	{
		TSoftObjectPtr<UWorld> WorldAsset = StreamingLevel->GetWorldAsset();
		LevelName StreamingLevelName = LevelUtilities::GetLevelNameFromSoftObjectPtr(WorldAsset);
		if (StreamingLevelName == Name)
		{
			return StreamingLevel;
		}
	}

	return nullptr;
}

template <bool bIsLoading, bool bIsTextFormat>
void TSaveGameSerializer<bIsLoading, bIsTextFormat>::SerializeActor(FStructuredArchive::FMap& ActorMap, AActor*& Actor, FLevelState& LevelData, SerializeBodyFunc BodyFunction)
{
	FString ActorName;
	FSoftClassPath Class;
	FGuid SpawnID;

	if (SAVING)
	{
		ActorName = LevelUtilities::GetActorName(Actor);

		if (!USaveGameFunctionLibrary::WasObjectLoaded(Actor))
		{
			// We're a spawned actor, stash the class
			Class = Actor->GetClass();
		}

		if (Actor->Implements<USaveableObjectInterface>())
		{
			SpawnID = ISaveableObjectInterface::Execute_GetSpawnID(Actor);
		}
	}

	FStructuredArchive::FSlot ActorSlot = ActorMap.EnterElement(ActorName);

	// If we have a class, we're a spawned actor
	if (TOptional<FStructuredArchive::FSlot> ClassSlot = ActorSlot.TryEnterAttribute(TEXT("Class"), !Class.IsNull()))
	{
		ClassSlot.GetValue() << Class;
	}

	// If we have a GUID, we're a spawn actor that needs to be mapped by GUID
	TOptional<FStructuredArchive::FSlot> GuidSlot = ActorSlot.TryEnterAttribute(TEXT("GUID"), SpawnID.IsValid());
	if (GuidSlot.IsSet())
	{
		GuidSlot.GetValue() << SpawnID;
	}

	uint64 DataSize;

	if (!bIsTextFormat)
	{
		// Pre-write how much data (in bytes) was serialized for this actor
		Archive << DataSize;
	}

	const uint64 BeginDataPosition = Archive.Tell();

	BodyFunction(ActorName, Class, SpawnID, ActorSlot);

	if (!bIsTextFormat)
	{
		if (LOADING)
		{
			// Skip our data and onto the next actor
			Archive.Seek(BeginDataPosition + DataSize);
		}
		else
		{
			const uint64 EndDataPosition = Archive.Tell();
			DataSize = EndDataPosition - BeginDataPosition;

			// Store the amount of data we've serialized (in bytes), back before the actual data
			Archive.Seek(BeginDataPosition - sizeof(DataSize));
			Archive << DataSize;

			// Go back to our current position
			Archive.Seek(EndDataPosition);
		}
	}
}

template<bool bIsLoading, bool bIsTextFormat>
int32 TSaveGameSerializer<bIsLoading, bIsTextFormat>::GetNextRequestID()
{
	static int32 RequestID = 0;
	return RequestID++;
}

// Instantiate the permutations of TSaveGameSerializer

#if WITH_TEXT_ARCHIVE_SUPPORT
template TSaveGameSerializer<false, true>;
#endif

template TSaveGameSerializer<false, false>;
template TSaveGameSerializer<true, false>;