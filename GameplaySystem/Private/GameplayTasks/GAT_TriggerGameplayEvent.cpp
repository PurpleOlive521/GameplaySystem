// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayTasks/GAT_TriggerGameplayEvent.h"
#include "GameplayEventSubsystem.h"
#include "DevelopmentTypes.h"

UGAT_TriggerGameplayEvent::UGAT_TriggerGameplayEvent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGAT_TriggerGameplayEvent* UGAT_TriggerGameplayEvent::TriggerGameplayEvent(UGameplayAbility* OwningAbility, TSubclassOf<UGameplayEvent> EventClass,
	FGameplayEventActivationData OptionalData, bool bAbortOnCancel, bool bAbortOnEnd)
{
	UGAT_TriggerGameplayEvent* NewTask = NewAbilityTask<UGAT_TriggerGameplayEvent>(OwningAbility);
	NewTask->EventClass = EventClass;
	NewTask->ActivationData = OptionalData;
	NewTask->bAbortOnCancel = bAbortOnCancel;
	NewTask->bAbortOnEnd = bAbortOnEnd;

	return NewTask;
}

void UGAT_TriggerGameplayEvent::Activate()
{
	// OnDestroy handles AbilityEnded
	AbilityCancelledHandle = Ability->OnAbilityCancelledDelegate.AddUObject(this, &UGAT_TriggerGameplayEvent::OnAbilityCancelled);

	UGameplayEventSubsystem* Subsystem = UGameplayEventSubsystem::Get(Ability);
	if (!Subsystem)
	{
		GS_LOG(Warning, TEXT("UGAT_PlayGameplayEvent: Couldn't get GameplayEventSubsystem!"));
		EndTask();
		return;
	}

	EventHandle = Subsystem->TriggerEvent_ActivationData(EventClass, Ability->GetOwningActor(), ActivationData);

	// Couldn't trigger, but isn't necessarily an error. 
	if (!EventHandle.IsValid())
	{
		EndTask();
		return;
	}

	UGameplayEvent* ActiveEvent = Subsystem->GetEventFromHandle(EventHandle);
	check(ActiveEvent);

	ActiveEvent->OnEventAbortedDelegate.AddUObject(this, &UGAT_TriggerGameplayEvent::AbortEvent);
	ActiveEvent->OnEventEndedDelegate.AddUObject(this, &UGAT_TriggerGameplayEvent::OnEventEnded);
}

void UGAT_TriggerGameplayEvent::ExternalCancel()
{
	AbortEvent();

	Super::ExternalCancel();
}

void UGAT_TriggerGameplayEvent::OnDestroy(bool bAbilityEnded)
{
	if (Ability)
	{
		Ability->OnAbilityCancelledDelegate.Remove(AbilityCancelledHandle);
	}

	UGameplayEventSubsystem* Subsystem = UGameplayEventSubsystem::Get(Ability);
	if (UGameplayEvent* ActiveEvent = Subsystem->GetEventFromHandle(EventHandle))
	{
		ActiveEvent->OnEventAbortedDelegate.RemoveAll(this);
		ActiveEvent->OnEventEndedDelegate.RemoveAll(this);
	}

	if (bAbilityEnded && bAbortOnEnd)
	{
		AbortEvent();
	}

	Super::OnDestroy(bAbilityEnded);
}

FString UGAT_TriggerGameplayEvent::GetDebugString() const
{
	UGameplayEventSubsystem* Subsystem = UGameplayEventSubsystem::Get(Ability);
	if (Subsystem)
	{
		UGameplayEvent* Event = Subsystem->GetEventFromHandle(EventHandle);
		if (Event)
		{
			const bool bIsActive = Event->IsActive();
			return FString::Printf(TEXT("TriggerGameplayEvent: Active: %s (%s)"), bIsActive ? TEXT("true") : TEXT("false"), *GetNameSafe(EventClass));
		}
	}

	return FString::Printf(TEXT("TriggerGameplayEvent: %s (%s)"), *GetName(), *InstanceName.ToString());
}

void UGAT_TriggerGameplayEvent::AbortEvent()
{
	if (bWasAborted)
	{
		return;
	}

	UGameplayEventSubsystem* Subsystem = UGameplayEventSubsystem::Get(Ability);
	if (Subsystem)
	{
		const bool bCouldAbort = Subsystem->EndEventByHandle(EventHandle);
		if (bCouldAbort)
		{
			if (ShouldBroadcastAbilityTaskDelegates())
			{
				OnGameplayEventAbortedDelegate.Broadcast();
			}
		}

		bWasAborted = true;
		EndTask();
	}
}

void UGAT_TriggerGameplayEvent::OnEventEnded(UGameplayEvent* Event)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnGameplayEventEndedDelegate.Broadcast();
	}
}

void UGAT_TriggerGameplayEvent::OnAbilityCancelled()
{
	if (bAbortOnCancel)
	{
		AbortEvent();
	}
}
