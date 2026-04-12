// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayTasks/GAT_WaitDelay.h"

UGAT_WaitDelay::UGAT_WaitDelay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
}

UGAT_WaitDelay* UGAT_WaitDelay::WaitDelay(UGameplayAbility* OwningAbility, float Time)
{
	UGAT_WaitDelay* NewTask = NewAbilityTask<UGAT_WaitDelay>(OwningAbility);
	NewTask->DelayLength = Time;

	return NewTask;
}

void UGAT_WaitDelay::Activate()
{
	bIsCounting = true;
}

void UGAT_WaitDelay::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (bIsCounting)
	{
		check(Ability);

		const float RelativeDeltaTime = DeltaTime * Ability->GetDeltaTimeCoefficient();

		TimeAccumulated += RelativeDeltaTime;

		if (TimeAccumulated >= DelayLength)
		{
			bIsCounting = false;
			OnDelayFinish();
		}
	}
}

FString UGAT_WaitDelay::GetDebugString() const
{
	return FString::Printf(TEXT("WaitDelay: DelayLength: %.2f. TimeLeft: %.2f"), DelayLength, TimeAccumulated);
}

void UGAT_WaitDelay::OnDelayFinish()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnWaitDelayFinishDelegate.Broadcast();
	}

	EndTask();
}