// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "GameplayEvent.h"
#include "GameplayEventHandle.h"
#include "GameplayEventTask.generated.h"

/**
 * GameplayTasks that are ensured to only operate within the lifetime of a GameplayEvent. 
 */
UCLASS(Abstract)
class GAMEPLAYSYSTEM_API UGameplayEventTask : public UGameplayTask
{
	GENERATED_BODY()

public:
	UGameplayEventTask(const FObjectInitializer& ObjectInitializer);

	// --- Begin UGameplayTask Interface
	virtual void OnDestroy(bool bInOwnerFinished) override;

	virtual void BeginDestroy() override;

	virtual void InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent) override;
	// --- End UGameplayTask Interface

	// Instantiates and initializes a new task
	template <class T>
	static T* NewEventTask(UGameplayEvent* ThisEvent, FName InstanceName = FName())
	{
		check(ThisEvent);

		T* NewTask = NewObject<T>();
		NewTask->InitTask(*ThisEvent, ThisEvent->GetGameplayTaskDefaultPriority());

		NewTask->InstanceName = InstanceName;
		return NewTask;
	}

	// Catch attempts at calling GameplayTask::NewTask through us
	template <class T>
	FORCEINLINE static T* NewTask(UObject* WorldContextObject, FName InstanceName = FName())
	{
		static_assert(DelayedFalse<T>(), "UGameplayTask::NewTask should never be used. Use NewEventTask instead");
		return nullptr;
	}

	// Returns the owning Events's handle
	FGameplayEventHandle GetEventHandle() const;

	void SetGameplayEvent(UGameplayEvent* InEvent);

	// This should be called prior to broadcasting delegates back into the event graph. This makes sure the event is still active.
	bool ShouldBroadcastEventTaskDelegates() const;

	virtual FString GetDebugString() const override;

protected:

	// The Event that created us
	UPROPERTY()
	TObjectPtr<UGameplayEvent> Event;

	uint8 bWasSuccessfullyDestroyed : 1 = false;
};