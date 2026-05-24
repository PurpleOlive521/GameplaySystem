// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayEventTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGameplayEvent, Log, All)

// Macro for logging in the LogGameplayEvent category
#define GE_LOG(Verbosity, Format, ...)								\
{																	\
	UE_LOG(LogGameplayEvent, Verbosity, Format, ##__VA_ARGS__);	\
}

UENUM(BlueprintType)
enum class EEventInstancingPolicy : uint8
{
	// No instancing required. Must use Static Trigger function!
	EEIP_Static				UMETA(DisplayName = "Static"),

	// Instancing required when triggered. Allows for per-event state and latent logic.
	EEIP_Instanced			UMETA(DisplayName = "Instanced"),
};

UENUM(BlueprintType)
enum class EEventDurationType : uint8
{
	// You must explicitly end the event with FinishEvent, or ensure that it has bShareOwnerLifetime set to true.
	EEDT_None			UMETA(DisplayName = "None"),

	// The event is ended when the duration is up. Duration accumulates per the set TickSource.
	EEDT_HasDuration	UMETA(DisplayName = "Has Duration"),
};

UENUM(BlueprintType)
enum class ETickSource : uint8
{
	// The global DeltaTime is used every frame. Affected by Global time dilation. 
	ETS_GlobalDeltaTime		UMETA(DisplayName = "Global Delta Time"),

	// If the GameplayEvent is owned by an Actor, use it's Delta Time. Otherwise, falls back to Global Delta Time. Affected by Actor's time dilation.
	ETS_SourceDeltaTime		UMETA(DisplayName = "Source Delta Time"),

	// The actual DeltaTime is used every frame. Unaffected by all time dialation. 
	ETS_AbsoluteDeltaTime	UMETA(DisplayName = "Absolute Delta Time"),
};

UENUM(BlueprintType)
enum class EQueryPolicy : uint8
{
	// We do not enforce the set query.
	EQP_None			UMETA(DisplayName = "None"),

	// The query is enforced only on the owning Actor of this GameplayEvent.
	EQP_PerActor		UMETA(DisplayName = "Per Actor"),

	// The query is enforced globally on all GameplayEvents.
	EQP_Global			UMETA(DisplayName = "Global"),
};

// Generic parameters to give context to a GameplayEvent activation.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayEventActivationData
{
	GENERATED_BODY()

	FGameplayEventActivationData() = default;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Magnitude = 0.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTagContainer Context;

	// Needs to be cast to the expected type.
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	uint8 Enum = 0U;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FHitResult HitResults;
};

constexpr uint64 INVALID_TICK_FOLLOWER_HANDLE_ID = 0U;

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FTickFollowerHandle
{
	GENERATED_BODY()

	FTickFollowerHandle() : Id() {};

	// Does not ensure that NewId is unused or unique.
	FTickFollowerHandle(uint32 NewId) : Id(NewId) {};

	static FTickFollowerHandle CreateNew();

	void GenerateNewHandle();

	[[nodiscard]] bool IsValid() const;

	friend uint32 GetTypeHash(const FTickFollowerHandle& InHandle);

	bool operator==(const FTickFollowerHandle& Other) const
	{
		return Id == Other.Id;
	}

	bool operator!=(const FTickFollowerHandle& Other) const
	{
		return Id != Other.Id;
	}

private:

	uint64 Id = 0U;
};

DECLARE_DELEGATE_OneParam(FOnLeaderTickSignature, float /* DeltaTime */);

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FObjectTickFollowers
{
	GENERATED_BODY()

	FObjectTickFollowers() = default;

	FTickFollowerHandle AddTickFollower(const FOnLeaderTickSignature& Delegate);

	void RemoveTickFollower(const FTickFollowerHandle& Handle);

	void Tick(float DeltaTime);

	void Clear();

	TMap<FTickFollowerHandle, FOnLeaderTickSignature> FollowerDelegates;
};