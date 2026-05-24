// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayTasks/GAT_StartAbilityState.h"

UGAT_StartAbilityState::UGAT_StartAbilityState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UGAT_StartAbilityState* UGAT_StartAbilityState::StartAbilityState(UGameplayAbility* OwningAbility, FName StateName, bool bEndCurrentState)
{
	UGAT_StartAbilityState* NewTask = NewAbilityTask<UGAT_StartAbilityState>(OwningAbility, StateName);
	NewTask->bEndCurrentState = bEndCurrentState;

	return NewTask;
}

void UGAT_StartAbilityState::Activate()
{
	check(Ability);

	if (bEndCurrentState)
	{
		Ability->OnAbilityStateEndedDelegate.Broadcast(END_ALL_STATES);
	}

	EndStateHandle = Ability->OnAbilityStateEndedDelegate.AddUObject(this, &UGAT_StartAbilityState::OnEndState);
	InterruptStateHandle = Ability->OnAbilityCancelledDelegate.AddUObject(this, &UGAT_StartAbilityState::OnInterruptState);
}

void UGAT_StartAbilityState::ExternalCancel()
{
	bWasInterrupted = true;

	Super::ExternalCancel();
}

void UGAT_StartAbilityState::OnDestroy(bool AbilityEnded)
{	
	// Unbind delegates so this doesn't get recursively called
	if (Ability)
	{
		Ability->OnAbilityCancelledDelegate.Remove(InterruptStateHandle);
		Ability->OnAbilityStateEndedDelegate.Remove(EndStateHandle);
	}

	if (bWasInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnStateInterruptedDelegate.Broadcast();
		}
	}
	else if ((bWasEnded || AbilityEnded))
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnStateEndedDelegate.Broadcast();
		}
	}

	Super::OnDestroy(AbilityEnded);
}

FString UGAT_StartAbilityState::GetDebugString() const
{
	return FString::Printf(TEXT("StartAbilityState: %s)"), *InstanceName.ToString());
}

void UGAT_StartAbilityState::OnEndState(FName StateNameToEnd)
{
	// All states end if 'END_ALL_STATES' is passed to this delegate
	if (StateNameToEnd == END_ALL_STATES || StateNameToEnd == InstanceName)
	{
		bWasEnded = true;

		EndTask();
	}
}

void UGAT_StartAbilityState::OnInterruptState()
{
	bWasInterrupted = true;
}
