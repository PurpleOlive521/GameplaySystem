// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplaySystemComponent.h"
#include "GameplaySystemDeveloperSettings.generated.h"

USTRUCT()
struct FAttributeConfiguration
{
	GENERATED_BODY()

	// The Attribute used as a value roof for this Attribute.
	UPROPERTY(EditAnywhere)
	EAttributeType MaxValueReference = EAttributeType::EAT_NONE;

	// When the MaxValueReference is increased, scale this Attribute up too ensure proportionality.
	// Only applied in certain scenarios such as Level-ups. 
	UPROPERTY(EditAnywhere)
	bool bScaleWithMaxValue = false;

	// If true, the Attribute can not have a value lower than 0.
	UPROPERTY(EditAnywhere)
	bool bIsUnsigned = false;
};

/**
 * Global settings for all GameplaySystems to use. 
 * Tweak game rules and system interactions to fit the game being made.
 */
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Gameplay System Settings"))
class GAMEPLAYSYSTEM_API UGameplaySystemDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:

	const UGameplaySystemProperties* GetDefaultProperties() const;

	float GetGlobalAnimPlayRate() const;

	// Defines how Attributes interact with eachother across ALL GameplaySystems. 
	UPROPERTY(Config, EditAnywhere, Category = "Attributes")
	TMap<EAttributeType, FAttributeConfiguration> AttributeSettings;

	// Fallback properties used when a GameplaySystem does not have a specified asset.
	UPROPERTY(Config, EditAnywhere)
	TSoftObjectPtr<const UGameplaySystemProperties> DefaultProperties;

	UPROPERTY(Config, EditAnywhere)
	float GlobalAnimPlayRate = 1.0f;
};
