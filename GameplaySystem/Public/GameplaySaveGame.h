// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "GameplaySaveGameTypes.h"
#include "GuidTag.h"

#include "GameplaySaveGame.generated.h"

// Base implementation for a SaveGame object that supports saving for all GameplaySystem features. 
// Intended to be derived and extended for game-specific features.
UCLASS()
class GAMEPLAYSYSTEM_API UGameplaySaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "GameplaySaveGame")
	float GetPlayTime() const;

	UFUNCTION(BlueprintCallable, Category = "GameplaySaveGame")
	void SetPlayTime(float InPlayTime);



	UFUNCTION(BlueprintCallable, Category = "GameplaySaveGame")
	void SaveGameplaySystem(const FGuidTag& Id, const FGameplaySystemSaveObject& SaveObject);

	UFUNCTION(BlueprintCallable, Category = "GameplaySaveGames")
	FGameplaySystemSaveObject LoadGameplaySystem(const FGuidTag& Id);


protected:
	// --- Save contents

	float PlayTime;

	TMap<FGuidTag, FGameplaySystemSaveObject> GameplaySystemToIdMap;
};
