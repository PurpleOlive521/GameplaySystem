// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SaveableObjectInterface.h"
#include "SaveGameFunctionLibrary.generated.h"

UCLASS()
class GAMEPLAYSYSTEM_API USaveGameFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Check if an object was loaded from an asset (i.e. a Static Mesh, Actor from a level, etc.)
	 *
	 * @param Object The object to check if loaded
	 * @return true if object was loaded from an asset
	 */
	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	static bool WasObjectLoaded(const UObject* Object);

	/**
	 * Check to see if the current save game is loading the archive.
	 *
	 * @param Archive The archive that the save game is serializing
	 * @return true if save game is loading, false if save game is saving
	 */
	UFUNCTION(BlueprintPure, Category = "SaveGame")
	static bool IsLoading(const FSaveGameArchive& Archive);

	/**
	 * Helper method to serialize an actor's transform if the actor is movable.
	 * If loading, will set the actor's transform.
	 *
	 * @param Archive The archive that the save game is serializing
	 * @param Actor The actor whose transform will be serialized
	 * @return true if the transform was serialized
	 */
	UFUNCTION(BlueprintCallable, Category = "SaveGame|Helpers", meta = (DefaultToSelf = "Actor"))
	static bool SerializeActorTransform(UPARAM(ref) FSaveGameArchive& Archive, AActor* Actor);

	/**
	 * Helper method to serialize a generic Controller, if the Character has one.
	 *
	 * @param Archive The archive that the save game is serializing
	 * @param Character The Character whose Controller will be serialized
	 * @return true if the Controller was serialized
	 */
	UFUNCTION(BlueprintCallable, Category = "SaveGame|Helpers", meta = (DefaultToSelf = "Character"))
	static bool SerializeGenericController(UPARAM(ref) FSaveGameArchive& Archive, AController* Controller);
};
