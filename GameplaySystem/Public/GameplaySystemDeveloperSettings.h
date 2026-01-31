// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplaySystemComponent.h"
#include "GameplaySystemDeveloperSettings.generated.h"

USTRUCT()
struct FAttributeConfiguration
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	// The Attribute used as a value roof for this Attribute.
	EAttributeType MaxValueReference = EAttributeType::EAT_NONE;

	// When the MaxValueReference is increased, scale this Attribute up too to keep the values proportional. 
	// Only applied in certain scenarios such as Level-ups. 
	UPROPERTY(EditAnywhere)
	bool bScaleWithMaxValue = false;

	UPROPERTY(EditAnywhere)
	// If true, the Attribute can not have a value lower than 0.
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

	// Defines how Attributes interact with eachother across ALL GameplaySystems. 
	UPROPERTY(Config, EditAnywhere, Category = "Attributes")
	TMap<EAttributeType, FAttributeConfiguration> AttributeSettings;

	
};
