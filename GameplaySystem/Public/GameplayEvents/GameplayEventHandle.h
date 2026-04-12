// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameplayTagContainer.h"

#include "GameplayEventHandle.generated.h"


constexpr uint32 INVALID_EVENT_HANDLE_ID = 0U;

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayEventHandle
{
	GENERATED_BODY()

	FGameplayEventHandle() : Id(), bWasInitialized(false) {};

	// Does not ensure that NewId is unused or unique.
	FGameplayEventHandle(uint32 NewId) : Id(NewId), bWasInitialized(true) {};

	static FGameplayEventHandle CreateNew();

	void GenerateNewHandle();

	[[nodiscard]] bool IsValid() const;

	friend uint32 GetTypeHash(const FGameplayEventHandle& InHandle);

	bool operator==(const FGameplayEventHandle& Other) const
	{
		return Id == Other.Id;
	}

	bool operator!=(const FGameplayEventHandle& Other) const
	{
		return Id != Other.Id;
	}

private:

	uint32 Id = 0U;

	uint32 bWasInitialized : 1;
};