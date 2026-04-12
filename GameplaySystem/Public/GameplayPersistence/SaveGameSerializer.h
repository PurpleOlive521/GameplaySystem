// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "SaveGameProxyArchive.h"
#include "Templates/ChooseClass.h"
#include "GameplayPersistenceSubsystem.h"

#if WITH_TEXT_ARCHIVE_SUPPORT
#include "Serialization/Formatters/JsonArchiveOutputFormatter.h"
#endif

class UGameplayPersistenceSubsystem;

constexpr int USER_SLOT_INDEX = 0;
constexpr bool USE_FILE_COMPRESSION = true;

class FSaveGameSerializer : public TSharedFromThis<FSaveGameSerializer>
{
public:
	virtual ~FSaveGameSerializer() = default;
};

/**
 * The class that manages serializing the world.
 */
template<bool bIsLoading, bool bIsTextFormat = false>
class TSaveGameSerializer final : public FSaveGameSerializer
{
	using FSaveGameMemoryArchive = typename TChooseClass<bIsLoading, FMemoryReader, FMemoryWriter>::Result;

	static_assert(!bIsLoading || !bIsTextFormat, "This serializer hasn't been implemented for text based loading, only saving!");
	static_assert(WITH_TEXT_ARCHIVE_SUPPORT || !bIsTextFormat, "Engine isn't compiled with text archive support, cannot use text based TSaveGameSerializer");

	using FSaveGameFormatter = typename TChooseClass<bIsTextFormat&& WITH_TEXT_ARCHIVE_SUPPORT,
		typename TChooseClass<bIsLoading, FBinaryArchiveFormatter, FJsonArchiveOutputFormatter>::Result,
		FBinaryArchiveFormatter>::Result;

	typedef TFunction<void(const FString&, const FSoftClassPath&, const FGuid&, FStructuredArchive::FSlot&)>&& SerializeBodyFunc;

	// A Section to perform serialization work in.
	//		- For loading, serializes and tracks the size of the Section to allow moving to the end of unread data.
	//		- For writing, protects any data after the Section while overwriting it's content to allow for seamless updates of in-save data.
	struct FSerializationScope
	{
		[[nodiscard]] FSerializationScope(TSaveGameSerializer& InSerializer, FArchiveSectionContainer& Container, const FString& ScopeName, FArchiveSection::EScopeType Type);
		~FSerializationScope();

		// Changes the key used for the Section.
		// Will not go in effect until after going out of Section!
		void Rename(FString NewName);

		// Will not perform any renaming or data copying when going out of Section.
		// Use when you want to back out of a Section without modifying remaining data.
		void Terminate();

		TSaveGameSerializer& Serializer;
		FArchiveSectionContainer& Container;
		FArchiveSection& Section;

		TArray<uint8> CopiedData;

		FString OptionalNewName;

		bool bIsTerminated = false;
	};

public:
	TSaveGameSerializer(UGameplayPersistenceSubsystem* InPersistenceSubsystem);

	static FString GetSaveName(const FString& SlotName);

	// Sublevel load/unload requests need a unique ID as to no be dropped.
	[[nodiscard]] static int32 GetNextRequestID();

	bool Save();
	template<bool bIsLoading = true, bool bIsTextFormat> bool Save(const FString& SlotName) = delete;

	bool SaveToDisk(const FString& SlotName);
	template<bool bIsLoading = true, bool bIsTextFormat> bool SaveToDisk(const FString& SlotName) = delete;

	bool Load(const FString& SlotName);
	template<bool bIsLoading = true, bool bIsTextFormat = false> bool Load(const FString& SlotName) = delete;

	bool SerializeSublevelSingle(ULevel* Level);

	// Sets our data to PersistenceSubsystems current data.
	const TArray<uint8> GetRemoteData();

	// Sets PersistenceSubsystems data to our current data.
	void SetRemoteData() const;

	FArchiveSectionContainer& GetSectionContainer() const;
	
	// Checks if no save data exists to serialize from/to.
	// For text-based, means that there is nothing except the opening brace in Data.
	// For binary, means that Data is empty.
	bool IsDataEmpty() const;

	void OnMapLoad(UWorld* World);

	// Serializes information about the archive, like Map Name, and position of versioning information
	void SerializeHeader();

	// This further calls SerializeActors and SerializeDestroyedActors within the context of each sublevel.
	void SerializeSublevels();

	void SerializeSublevel(FStructuredArchive::FMap& LevelMap, UWorld* World, const LevelName& Level, FLevelState* LevelData, bool bUseGameState);

	void SerializeGlobals(UWorld* World);

	// Serializes Actors in the sublevels context.
	void SerializeActors(FStructuredArchive::FRecord& Record, ULevel* Level, UWorld* World, bool bSerializingSublevel, FLevelState& LevelData);

	void SerializeDestroyedActors(FStructuredArchive::FRecord& Record, ULevel* Level, UWorld* World, FLevelState& LevelData);

	// Serialized at the end of the archive, the versions are useful for marshaling old data.
	void SerializeVersions();

	ULevelStreaming* GetSublevelByName(UWorld* World, LevelName Name) const;

	/**
	 * Serializes the actor's data into the structured archive.
	 * This data always comprises of the actor's object name, and optionally its:
	 * - Class: If the actor was spawned (so that it can be spawned again)
	 * - SpawnID: If the actor implements ISaveableObject. A unique identifier to map the data back to an already
	 *				spawned actor (like the player's character)
	 *
	 * It also takes a lambda function that can optionally do some work or serialization. Ultimately, once this
	 * lambda function is complete, SerializeActor will automatically seek the archive to the end of the actor's data.
	 *
	 * @param ActorMap The structured map that the actor data will be written to
	 * @param Actor The live actor that will be serialized
	 * @param BodyFunction A lambda function that will optionally do some work, whether that be serializing or spawning
	 */
	void SerializeActor(FStructuredArchive::FMap& ActorMap, AActor*& Actor, FLevelState& LevelData, SerializeBodyFunc BodyFunction);

	const TWeakObjectPtr<UGameplayPersistenceSubsystem> PersistenceSubsystem = nullptr;
	FArchiveSectionContainer& SectionContainer;

	TArray<uint8> Data;
	FSaveGameMemoryArchive Archive;
	TSaveGameProxyArchive<bIsLoading> ProxyArchive;
	FSaveGameFormatter Formatter;
	FStructuredArchive StructuredArchive;

	FStructuredArchive::FSlot RootSlot;
	FStructuredArchive::FRecord RootRecord;

	FName MapName;
	uint64 VersionOffset = 0U;
};
