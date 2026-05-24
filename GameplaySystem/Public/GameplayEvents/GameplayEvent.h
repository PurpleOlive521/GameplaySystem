// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "GameplayTagContainer.h"
#include "GameplayEventTypes.h"
#include "GameplayTaskOwnerInterface.h"
#include "GameplayEventHandle.h"

#include "GameplayEvent.generated.h"

class UGameplayEventSubsystem;

// Invoked when the GameplayEvent ends
DECLARE_MULTICAST_DELEGATE_OneParam(FOnEventEndedSignature, UGameplayEvent* /* EndingEvent */);

// Invoked when the GameplayEvent is aborted
DECLARE_MULTICAST_DELEGATE(FOnEventAbortedSignature);

namespace GameplayEventConstants
{
	constexpr float NO_DURATION = 0.0f;
}

/**
 * Encapsulation for reusable bundles of logic. 
 * Events are said to be 'Triggered', and allow latent logic to be easily separated from a object or Actor.
 * Responds to a set of life-time events and are derivable to build your own GameplayEvents:
 * 
 *	* TriggerEvent: Startpoint for the Event.
 * 
 *	* EndEvent: Either prompted from the set duration of the Event elapsing, or by the Event itself to signal that it is finished.
 * 
 *	* AbortEvent: External request to gracefully end the Event prematurely. 
 *	  For most Events will perform the same logic as EndEvent through FinishAbortWithEndEvent
 * 
 * Supports specialized GameplayTasks that enable latent logic that is tied to the GameplayEvents lifetime.
 */
UCLASS(Blueprintable)
class GAMEPLAYSYSTEM_API UGameplayEvent : public UObject, public IGameplayTaskOwnerInterface
{
	GENERATED_BODY()

public:
	UGameplayEvent();

	// --- Begin UObject 
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual UWorld* GetWorld() const override;

	virtual void FinishDestroy() override;
	// --- End UObject

	// --- Begin IGameplayTaskOwnerInterface
	virtual UGameplayTasksComponent* GetGameplayTasksComponent(const UGameplayTask& Task) const override;
	virtual AActor* GetGameplayTaskOwner(const UGameplayTask* Task) const override;
	virtual AActor* GetGameplayTaskAvatar(const UGameplayTask* Task) const override;
	virtual void OnGameplayTaskInitialized(UGameplayTask& Task) override;
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override;
	virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) override;
	// --- End IGameplayTaskOwnerInterface

	void Init(UObject* InOwningObject);

	bool ShouldTick() const;

	void Tick(float DeltaTime);

	// Activates the GameplayEvent.
	bool TryTriggerGameplayEvent(const FGameplayEventActivationData& ActivationData);

	void StaticTryTriggerGameplayEvent(const UObject* WorldContextObject, const FGameplayEventActivationData& ActivationData) const;

	// Ends the GameplayEvent.
	bool TryEndGameplayEvent();

	// Forcefully ends the GameplayEvent, possibly before intended. 
	// Not fulfilled if the GameplayEvent has already ended.
	bool TryAbortEvent();

	UFUNCTION(BlueprintCallable, Category = "GameplayEvent")
	void SendEventNotify(FName Notify);

	UFUNCTION(BlueprintCallable, Category = "GameplayEvent")
	UObject* GetOwningObject() const;

	UFUNCTION(BlueprintCallable, Category = "GameplayEvent")
	UObject* GetOwningObject_Checked() const;

	// Returns true if a valid Actor can be retrieved from the OwningObject.
	bool HasOwningActor() const;

	// Can return nullptr if not owned by an Actor.
	// For static events, always returns nullptr. Instead use the Owner parameter for static events.
	UFUNCTION(BlueprintCallable, Category = "GameplayEvent")
	AActor* GetOwnerAsActor() const;

	// For static events, always asserts. Instead use the Owner parameter for static events.
	UFUNCTION(BlueprintCallable, Category = "GameplayEvent")
	AActor* GetOwnerAsActor_Checked() const;

	// Assumes this object is a valid WorldContextObject. 
	UGameplayEventSubsystem* GetEventSubsystem() const;

	// Gets the GameplayEventSubsystem from WorldContextObject.
	UGameplayEventSubsystem* GetEventSubsystem(const UObject* WorldContextObject) const;

	[[nodiscard]] FGameplayEventHandle GetEventHandle() const;

	float GetDeltaTimeCoefficient() const;
	
	// Returns true if the GameplayEvent is done with it's tasks and should be removed.
	bool ShouldCleanup() const;

	// Returns true if the GameplayEvent is active and running it's task. Returns false if aborted or ended.
	bool IsActive() const;

	// Returns false if already applied, true otherwise.
	bool ApplyBlockingQuery();

	// Returns false if already removed, true otherwise.
	bool RemoveBlockingQuery();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayEvent")
	virtual FString ToString() const;

	// With added text tags for improved readability.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayEvent")
	virtual FString ToStringWithDebugTags() const;
	
	// Internal name used for display in Debug views and other displays. Not meant to be presented to the player.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameplayEvent")
	FString DisplayName = "GameplayEvent";

	// How we manage this GameplayEvent.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEvent")
	EEventInstancingPolicy InstancingPolicy = EEventInstancingPolicy::EEIP_Instanced;

	// Where we source our DeltaTime from, which is used both in the tick functions and for counting duration.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEvent")
	ETickSource TickSource = ETickSource::ETS_GlobalDeltaTime;

	// Continues updating lifetime even when the game is paused.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEvent")
	bool bTickWhenPaused = false;

	// Events that have no Duration must explicitly call FinishEvent to end themselves.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEvent|Lifetime")
	EEventDurationType DurationType = EEventDurationType::EEDT_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "DurationType != EEventDurationType::EEDT_None", Units = "Seconds"), Category = "GameplayEvent|Lifetime")
	float Duration = 0;

	// The GameplayEvent will be ended if it's owner is removed.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEvent|Lifetime")
	bool bShareOwnerLifetime = true;

	// Only allows one instance of this GameplayEvent to be active at once.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEvent|Uniqueness")
	bool bGloballyUnique = false;

	// Only allows one instance per Actor to be active at once.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "!bGloballyUnique"), Category = "GameplayEvent|Uniqueness")
	bool bActorUnique = false;

	// The behaviour when enforcing Uniqueness. If true Aborts any existing Event and triggers a new one in it's place, otherwise fails to trigger.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEvent|Uniqueness")
	bool bReplaceOnUnique = false;

	// Describes the intent and purpose of a GameplayEvent. Used when enforcing Blocks and End GameplayTagQueries, or for filtering GameplayEvents.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayEvent|GameplayTags")
	FGameplayTagContainer EventTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEvent|GameplayTags")
	FGameplayTagQuery EndMatchingEvents;

	// Our policy for the EndMatchingEvents query.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEvent|GameplayTags")
	EQueryPolicy EndQueryPolicy = EQueryPolicy::EQP_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEvent|GameplayTags")
	FGameplayTagContainer BlockEventsWithTag;

	// Our policy for the BlockEventsWithTag container.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEvent|GameplayTags")
	EQueryPolicy BlockQueryPolicy = EQueryPolicy::EQP_None;

	UPROPERTY(BlueprintReadWrite)
	FObjectTickFollowers TickFollowers;

protected:

	// Boilerplate setup and enforcing policies before triggering fully.
	bool PreTriggerEvent();

	bool StaticPreTriggerEvent(const UObject* WorldContextObject) const;
	
	virtual void TriggerEvent(const FGameplayEventActivationData& ActivationData);

	// Always called after the native TriggerEvent.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "BP Trigger Event"), Category = "GameplayEvent")
	void K2_TriggerEvent(const FGameplayEventActivationData& ActivationData);

	// Should only be implemented for GameplayEvents with EventInstancingPolicy set to Static.
	// Does nothing when set to other policy's.
	virtual void StaticTriggerEvent(const UObject* Owner, const FGameplayEventActivationData& ActivationData) const;

	// Always called after the native StaticTriggerEvent.
	// Should only be implemented for GameplayEvents with EventInstancingPolicy set to Static.
	// Does nothing when set to other policy's.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "BP Static Trigger Event"), Category = "GameplayEvent")
	void K2_StaticTriggerEvent(const UObject* Owner, const FGameplayEventActivationData& ActivationData) const;

	virtual void EndEvent();

	// Always called after the native EndEvent.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "BP End Event"), Category = "GameplayEvent")
	void K2_EndEvent();

	virtual void AbortEvent();

	// Always called after the native AbortEvent.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "BP Abort Event"), Category = "GameplayEvent")
	void K2_AbortEvent();

	virtual void ReceiveEventNotify(FName Notify);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Blueprint Receive Event Notify"), Category = "GameplayEvent")
	void K2_ReceiveEventNotify(FName Notify);

	// Prematurely end the GameplayEvent, removing it from the system.
	// Intended to be called by the GameplayEvent itself once it's finished in the case that it has no duration.
	UFUNCTION(BlueprintCallable, Category = "GameplayEvent")
	void FinishEvent(bool bTriggerEndCallback = true);
	
	// Tries to end the GameplayEvent with the EndEvent callbacks, even if it was aborted.
	// Only use when aborting!
	UFUNCTION(BlueprintCallable, Category = "GameplayEvent")
	void FinishAbortWithEndEvent();

	// Safely terminates the event by ending it early and printing a error message. 
	// Only use to avoid errors or crashing. Consider using Checked functions for gameplay-crucial events.
	UFUNCTION(BlueprintCallable, Category = "GameplayEvent")
	void ForceTerminate();

	// Use for Static Trigger Event. Safely terminates the event by ending it early and printing a error message. 
	// Only use to avoid errors or crashing. Consider using Checked functions for gameplay-crucial events.
	UFUNCTION(BlueprintCallable, Category = "GameplayEvent")
	void StaticForceTerminate() const;

	// Returns true if the GameplayEvent has been Aborted already.
	UFUNCTION(BlueprintCallable, Category = "GameplayEvent")
	bool HasBeenAborted() const;

	// Marks the Event for GC and stops any running tasks.
	void StopEvent();

	UPROPERTY()
	TArray<TObjectPtr<UGameplayTask>> ActiveTasks;
	
private:

	void CleanProperties();

	TWeakObjectPtr<AActor> OwningActor = nullptr;

	TWeakObjectPtr<UObject> OwningObject = nullptr;

	float Lifetime = 0.0f;

	// Guard against multiple end requests.
	uint32 bHasEnded : 1 = false;

	// Guard against multiple trigger requests.
	uint32 bHasTriggered : 1 = false;

	// Guard against multiple abort requests.
	uint32 bHasAborted : 1 = false;

	uint32 bIsAborting : 1 = false;

	uint32 bMarkedForCleanup : 1 = false;

	uint32 bAppliedBlockQuery : 1 = false;
	
public:
	// --- Delegates
	FOnEventEndedSignature OnEventEndedDelegate;

	FOnEventAbortedSignature OnEventAbortedDelegate;
};