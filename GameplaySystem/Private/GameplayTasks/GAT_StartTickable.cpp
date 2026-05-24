// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayTasks/GAT_StartTickable.h"

UGAT_StartTickable::UGAT_StartTickable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
}

UGAT_StartTickable* UGAT_StartTickable::StartTickable(UGameplayAbility* OwningAbility)
{
	UGAT_StartTickable* NewTask = NewAbilityTask<UGAT_StartTickable>(OwningAbility);

	return NewTask;
}

void UGAT_StartTickable::Activate()
{
	bIsTicking = true;
}

void UGAT_StartTickable::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (bIsTicking)
	{
		check(Ability);

		const float RelativeDeltaTime = DeltaTime * Ability->GetDeltaTimeCoefficient();

		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnTickDelegate.Broadcast(RelativeDeltaTime);
		}
	}
}

FString UGAT_StartTickable::GetDebugString() const
{
	return TEXT("StartTickable");
}

void UGAT_StartTickable::StopTicking()
{
	bIsTicking = false;

	EndTask();
}
