// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once


#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayPersistenceSubsystem.h"
#include "SaveGameSettings.generated.h"

/**
 * Global settings for saving and loading the game.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Save Game Settings"))
class GAMEPLAYSYSTEM_API USaveGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	bool CanSerializeLevel(LevelName Level) const;

	// Levels that will never be saved on game save.
	// Use for static sublevels that will never have to be serialized.
	UPROPERTY(EditAnywhere, Config, Category = "Levels")
	TArray<TSoftObjectPtr<UWorld>> NeverSerialize;
};
