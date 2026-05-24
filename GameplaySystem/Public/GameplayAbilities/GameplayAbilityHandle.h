// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameplayAbilityHandle.generated.h"


constexpr uint64 INVALID_ABILITY_HANDLE_ID = 0U;

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayAbilityHandle
{
	GENERATED_BODY()

	FGameplayAbilityHandle() : Id() {};

	// Does not ensure that NewId is unused or unique.
	FGameplayAbilityHandle(uint32 NewId) : Id(NewId) {};

	static FGameplayAbilityHandle CreateNew();

	void GenerateNewHandle();

	[[nodiscard]] bool IsValid() const;

	friend uint32 GetTypeHash(const FGameplayAbilityHandle& InHandle);

	bool operator==(const FGameplayAbilityHandle& Other) const
	{
		return Id == Other.Id;
	}

	bool operator!=(const FGameplayAbilityHandle& Other) const
	{
		return Id != Other.Id;
	}

private:

	uint64 Id = 0U;
};