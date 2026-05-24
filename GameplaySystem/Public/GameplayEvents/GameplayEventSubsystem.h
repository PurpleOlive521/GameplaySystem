// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayEventHandle.h"
#include "GameplayEventTypes.h"
#include "GameplayTagTypes.h"
#include "GameplayTagSystem.h"
#include "functional"
#include "GameplayEventSubsystem.generated.h"

class UGameplayEvent;
class UGameplaySystemDebugWidget;

constexpr AActor* NO_ACTOR = nullptr;

struct GAMEPLAYSYSTEM_API FActorGameplayEventContainer
{
	FActorGameplayEventContainer() = default;

	TArray<FGameplayEventHandle> GameplayEvents;

	FGameplayTagSystem BlockedEventTags;
};

USTRUCT()
struct GAMEPLAYSYSTEM_API FQueuedEvent
{
	GENERATED_BODY()

	FQueuedEvent() = default;

	explicit FQueuedEvent(UGameplayEvent* InEvent, FGameplayEventHandle InHandle) : Event(InEvent), Handle(InHandle) {};

	UPROPERTY()
	UGameplayEvent* Event = nullptr;

	FGameplayEventHandle Handle;
};

UCLASS()
class GAMEPLAYSYSTEM_API UGameplayEventSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	
	friend UGameplaySystemDebugWidget;
	friend UGameplayEvent;

public:

	// --- Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const;
	// --- End USubsystem Interface

	// --- Begin FTickableObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual bool IsTickableWhenPaused() const override;
	// --- End FTickableObject Interface

	// Helper getter
	static UGameplayEventSubsystem* Get(const UObject* WorldContext);

	// The entrypoint for creating and activating any GameplayEvent. 
	// Returns a valid GameplayEventHandle if activated successfully.
	FGameplayEventHandle TriggerEvent(TSubclassOf<UGameplayEvent> EventClass, UObject* Owner);

	FGameplayEventHandle TriggerEvent_ActivationData(TSubclassOf<UGameplayEvent> EventClass, UObject* Owner, const FGameplayEventActivationData& ActivationData);

	// Can return nullptr if the Handle is invalid.
	UGameplayEvent* GetEventFromHandle(const FGameplayEventHandle& Handle);

	// Each index maps 1:1 between Handles and OutEvents. Any indices can be nullptr if the corresponding handle is invalid.
	void GetEventsFromHandles(const TArray<FGameplayEventHandle>& Handles, TArray< UGameplayEvent*> OutEvents);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayEventSubsystem")
	void GetEventsByType(TSubclassOf<UGameplayEvent> EventClass, TArray<FGameplayEventHandle>& OutEvents, const UGameplayEvent* Ignore = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayEventSubsystem")
	void GetEventsByTypeFromActor(TSubclassOf<UGameplayEvent> EventClass, TArray<FGameplayEventHandle>& OutEvents, AActor* Actor, const UGameplayEvent* Ignore = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayEventSubsystem")
	void GetEventsByGameplayTagQuery(const FGameplayTagQuery& Query, TArray<FGameplayEventHandle>& OutEvents, const UGameplayEvent* Ignore = nullptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayEventSubsystem")
	void GetEventsByGameplayTagQueryFromActor(const FGameplayTagQuery& Query, TArray<FGameplayEventHandle>& OutEvents, AActor* Actor, const UGameplayEvent* Ignore = nullptr);

	// Returns true if the event was ended successfully.
	UFUNCTION(BlueprintCallable, Category = "GameplayEventSubsystem")
	bool EndEventByHandle(const FGameplayEventHandle& Handle);

	// Returns true if any one event was ended successfully.
	UFUNCTION(BlueprintCallable, Category = "GameplayEventSubsystem")
	bool EndEventsByHandles(const TArray<FGameplayEventHandle>& Handles);

	// Safely remove any finished GameplayEvents and abort ones that are without a valid owner.
	void CleanupGameplayEvents();

	UFUNCTION(BlueprintCallable, Category = "GameplayEventSubsystem")
	void ClearAllGameplayEvents();

	void RequestCleanup();

	UFUNCTION(BlueprintCallable, Category = "GameplayEventSubsystem")
	void SetBlockAllEvents(bool InState);

	UFUNCTION(BlueprintCallable, Category = "GameplayEventSubsystem")
	bool GetBlockAllEvents() const;

	UFUNCTION(BlueprintCallable, Category = "GameplayEventSubsystem")
	FGameplayEventHandle GetHandleForEvent(const UGameplayEvent* Event) const;

protected:

	// If valid, OptionalHandle will be used for the GameplayEvent if triggered successfully.
	FGameplayEventHandle TriggerEvent_Internal(TSubclassOf<UGameplayEvent> EventClass, UObject* Owner, const FGameplayEventActivationData& ActivationData);

	void TickEvent(UGameplayEvent* Event, float DeltaTime, float AbsoluteDeltaTime, float GlobalDeltaTime);

	// Returns true if the GameplayEvent can trigger.
	// Does not ensure that it can trigger fully, as that depends on user code, but checks blocking queries and policies that we enforce.
	bool CanTrigger(const UGameplayEvent* EventCDO, UObject* Owner);

	void GetEventsByPredicate(std::function<bool(const UGameplayEvent*)> Predicate, TWeakObjectPtr<AActor> Target, const UGameplayEvent* Ignore, TArray<FGameplayEventHandle>& OutEvents);

	void HandlePreGarbageCollect();

	void ProcessQueuedEvents();

	// Queues up a GameplayEvent, triggering it without adding to the event list until lock is removed.
	void EnqueueEvent(const FQueuedEvent& Event);

	inline bool IsLockActive() const;

	UPROPERTY()
	TMap<FGameplayEventHandle, TObjectPtr<UGameplayEvent>> EventMap;

	TMap<TWeakObjectPtr<AActor>, FActorGameplayEventContainer> PerActorEventMap;

	FGameplayTagSystem GloballyBlockedEventTags;

	uint32 bBlockAllEvents = false;

	uint32 bHasQueuedCleanup : 1 = false;

	uint32 EventMapLockCount = 0;

	TArray<FQueuedEvent> EventQueue;

	struct FGameplayEventMapLock
	{
		UGameplayEventSubsystem& EventSubsystem;

		FGameplayEventMapLock(UGameplayEventSubsystem& InSystem) : EventSubsystem(InSystem)
		{
			EventSubsystem.EventMapLockCount++;
		}

		~FGameplayEventMapLock()
		{
			EventSubsystem.EventMapLockCount--;

			if (EventSubsystem.EventMapLockCount <= 0)
			{
				EventSubsystem.ProcessQueuedEvents();
			}
		}
	};
};