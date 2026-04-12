// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayTasks/GET_WaitDelay.h"

UGET_WaitDelay::UGET_WaitDelay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
}

UGET_WaitDelay* UGET_WaitDelay::WaitDelay(UGameplayEvent* OwningEvent, float Time)
{
	UGET_WaitDelay* NewTask = NewEventTask<UGET_WaitDelay>(OwningEvent);
	NewTask->DelayLength = Time;

	return NewTask;
}

void UGET_WaitDelay::Activate()
{
	bIsCounting = true;
}

void UGET_WaitDelay::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (bIsCounting)
	{
		check(Event);

		const float RelativeDeltaTime = DeltaTime * Event->GetDeltaTimeCoefficient();

		TimeAccumulated += RelativeDeltaTime;

		if (TimeAccumulated >= DelayLength)
		{
			bIsCounting = false;
			OnDelayFinish();
		}
	}
}

FString UGET_WaitDelay::GetDebugString() const
{
	return FString::Printf(TEXT("WaitDelay: DelayLength: %.2f. TimeLeft: %.2f"), DelayLength, TimeAccumulated);
}

void UGET_WaitDelay::OnDelayFinish()
{
	if (ShouldBroadcastEventTaskDelegates())
	{
		OnWaitDelayFinishDelegate.Broadcast();
	}

	EndTask();
}