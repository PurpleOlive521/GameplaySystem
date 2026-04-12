// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveableObjectInterface.generated.h"

class UGameplaySaveGame;

#define SAVING !bIsLoading
#define LOADING bIsLoading

/**
 * The blueprint representation of the structured record we're writing to.
 *
 * When serializing a binary archive, FSaveGameArchive on construction will store its initial position that it started
 * serializing from. Once FSaveGameArchive loses Section and calls its destructor, it will then serialize all of the
 * field names and their offsets, if loading, it will automatically seek to the very end of the archive. The initial
 * position and stored offsets can be used for out-of-order seeking to each of the archive's serialized fields.
 *
 * Additionally, when loading, these field names are checked against CoreRedirects and redirected if needed.
 */
USTRUCT(BlueprintType, BlueprintInternalUseOnly)
struct GAMEPLAYSYSTEM_API FSaveGameArchive
{
	GENERATED_BODY()

public:
	FSaveGameArchive()
		: Record(nullptr)
		, Object(nullptr)
		, StartPosition(0)
		, EndPosition(0)
	{
	}

	FSaveGameArchive(class FStructuredArchive::FRecord& InRecord, UObject* InObject);
	~FSaveGameArchive();

	bool IsValid() const
	{
		return Record != nullptr;
	}

	class FStructuredArchive::FRecord& GetRecord() const
	{
		return *Record;
	}

	/**
	 * Serializes a field with a custom lambda function. If a binary format, stores its offset for out-of-order reading.
	 * @param FieldName Name of the field that's being serialized
	 * @param SerializeFunction Lambda function to do the actual serialization, provides a structured slot
	 * @return true if the field was serialized
	 */
	template<typename FSerializeFunc>
	bool SerializeField(FName FieldName, FSerializeFunc SerializeFunction)
	{
		if (!IsValid())
		{
			return false;
		}

		FArchive& Archive = Record->GetUnderlyingArchive();

		if (Archive.IsSaving() && Fields.Contains(FieldName))
		{
			// We don't want to double up on saving the same property
			return false;
		}

		// Text formats don't deal with seeking very well
		if (!Archive.IsTextFormat())
		{
			if (Archive.IsLoading())
			{
				if (!Fields.Contains(FieldName))
				{
					return false;
				}

				Archive.Seek(StartPosition + Fields[FieldName]);
			}
			else
			{
				// Use an offset, in case we need to shuffle data around later!
				Fields.Add(FieldName, Archive.Tell() - StartPosition);
			}
		}

		SerializeFunction(Record->EnterField(*FieldName.ToString()));

		return true;
	}

private:
	FSaveGameArchive(FSaveGameArchive&) = delete;

	class FStructuredArchive::FRecord* Record;
	TWeakObjectPtr<> Object;
	uint64 StartPosition;
	uint64 EndPosition;

	/** This serialized fields and their offsets from the start of this archive */
	TMap<FName, uint64> Fields;
};

// Ensure that our archive can't be copied
template<>
struct TStructOpsTypeTraits<FSaveGameArchive> : public TStructOpsTypeTraitsBase2<FSaveGameArchive>
{
	enum
	{
		WithCopy = false
	};
};

UINTERFACE()
class GAMEPLAYSYSTEM_API USaveableObjectInterface : public UInterface
{
	GENERATED_BODY()
};

class GAMEPLAYSYSTEM_API ISaveableObjectInterface
{
	GENERATED_BODY()

public:
	
	// Returns a unique Spawn ID for this Actor
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "SaveGame|Spawn")
	const FGuid GetSpawnID() const;

	// Assigns a new SpawnID to this Actor
	UFUNCTION(BlueprintNativeEvent, Category = "SaveGame|Spawn")
	bool SetSpawnID(const FGuid& NewID);

	// Global data will always be loaded and saved, regardless of the active level. Important for "game wide" data such as player state, currency, and more.
	UFUNCTION(BlueprintNativeEvent, Category = "SaveGame|Spawn")
	bool IsGlobalData();

	virtual bool OnSerialize(FSaveGameArchive& Archive, bool bIsLoading);

	// Blueprint exposed version of OnSerialize. Is always called after native OnSerialize.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "On Serialize"), Category = "SaveGame")
	bool K2_OnSerialize(UPARAM(ref) FSaveGameArchive& Archive, bool bIsLoading);
};
