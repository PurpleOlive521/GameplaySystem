// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayTasks/GameplayEventTask.h"
#include "DevelopmentTypes.h"

UGameplayEventTask::UGameplayEventTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGameplayEventTask::OnDestroy(bool bInOwnerFinished)
{
	if (!bWasSuccessfullyDestroyed)
	{
		bWasSuccessfullyDestroyed = true;

		Event = nullptr;

		Super::OnDestroy(bInOwnerFinished);
	}
	else
	{
		// Tried to destroy a task twice!
		GE_LOG(Warning, TEXT("Tried to destroy GameplayEventTask %s (%s) twice!"), *GetName(), *InstanceName.ToString());
		//ensureNoEntry();
	}
}

void UGameplayEventTask::BeginDestroy()
{
	Super::BeginDestroy();

	if (!bWasSuccessfullyDestroyed)
	{
		bWasSuccessfullyDestroyed = true;
	}
}

void UGameplayEventTask::InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent)
{
	UGameplayTask::InitSimulatedTask(InGameplayTasksComponent);
}

FGameplayEventHandle UGameplayEventTask::GetEventHandle() const
{
	return Event ? Event->GetEventHandle() : FGameplayEventHandle();
}

void UGameplayEventTask::SetGameplayEvent(UGameplayEvent* InEvent)
{
	check(InEvent);

	Event = InEvent;
}

bool UGameplayEventTask::ShouldBroadcastEventTaskDelegates() const
{
	bool bShouldBroadcast = (Event && Event->IsActive());

	return bShouldBroadcast;
}

FString UGameplayEventTask::GetDebugString() const
{
	return FString::Printf(TEXT("GameplayEventTask: %s (%s)"), *GetName(), *InstanceName.ToString());
}