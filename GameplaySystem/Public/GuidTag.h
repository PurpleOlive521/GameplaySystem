// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "GuidTag.generated.h"

// Wrapper for Guid that exposes it to the Blueprint ecosystem. 
// Allows for identifying objects between sessions or other situations where pointers would no longer be valid.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGuidTag
{
	GENERATED_BODY()

	FGuidTag()
	{
		Guid = FGuid::NewGuid();
	};

	FGuidTag(const FString& InGuid)
	{
		Guid = FGuid(InGuid);
	}

	void Reset()
	{
		Guid = FGuid::NewGuid();
	}

	[[nodiscard]] FORCEINLINE FString ToString() const
	{
		return Guid.ToString();
	}

	bool operator==(const FGuidTag& Other) const
	{
		return Guid == Other.Guid;		
	}

	friend uint32 GetTypeHash(const FGuidTag& InGuidTag)
	{
		return GetTypeHash(InGuidTag.Guid);
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	FGuid Guid = {};
};