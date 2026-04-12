// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "GameplayTagSystem.h"

#include "GameplaySystemOwnerInterface.generated.h"

class UGameplayTagComponent;

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class GAMEPLAYSYSTEM_API UGameplaySystemOwnerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Allows access to common GameplaySystem functionality. Allows the controlling components to be spread out between Actors, or use stand-ins that point to a single component.
 */
class GAMEPLAYSYSTEM_API IGameplaySystemOwnerInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameplaySystemOwnerInterface")
	class UGameplaySystemComponent* GetGameplaySystemComponent();

	// Not all objects will use the GameplaySystemComponent or it's internal GameplayTagSystem.
	// When possible, acccess it through this interface to allow the object to define how it stores its GameplayTagSystem.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameplaySystemOwnerInterface", meta = (DisplayName = "Get GameplayTagSystem"))
	void K2_GetGameplayTagSystem(FGameplayTagSystem& OutGameplayTagSystem);

	// Returns a pointer to the GameplayTagSystem struct to avoid accidental copies and improper handling.
	virtual FGameplayTagSystem* GetGameplayTagSystem();
};
