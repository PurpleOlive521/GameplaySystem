// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayTasks/GameplayAbilityTask.h"
#include "DevelopmentTypes.h"
#include "GameplayTasksComponent.h"
#include "GameplaySystemComponent.h"

UGameplayAbilityTask::UGameplayAbilityTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WaitStateBitMask = static_cast<uint8>(EAbilityTaskWaitState::EAT_WaitingOnGame);
}

void UGameplayAbilityTask::OnDestroy(bool bInOwnerFinished)
{
	if (!bWasSuccessfullyDestroyed)
	{
		bWasSuccessfullyDestroyed = true;

		Ability = nullptr;

		Super::OnDestroy(bInOwnerFinished);
	}
	else
	{
		// Tried to destroy a task twice!
		GS_LOG(Warning, TEXT("Tried to destroy GameplayAbilityTask %s (%s) twice!"), *GetName(), *InstanceName.ToString());
		//ensureNoEntry();
	}
}

void UGameplayAbilityTask::BeginDestroy()
{
	Super::BeginDestroy();

	if (!bWasSuccessfullyDestroyed)
	{
		bWasSuccessfullyDestroyed = true;
	}
}

void UGameplayAbilityTask::InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent)
{
	UGameplayTask::InitSimulatedTask(InGameplayTasksComponent);

	SetGameplaySystemComponent(Cast<UGameplaySystemComponent>(TasksComponent.Get()));
}

FGameplayAbilityHandle UGameplayAbilityTask::GetAbilityHandle() const
{
	return Ability ? Ability->GetAbilityHandle() : FGameplayAbilityHandle();
}

void UGameplayAbilityTask::SetGameplaySystemComponent(UGameplaySystemComponent* InGameplaySystem)
{
	check(InGameplaySystem);

	GameplaySystem = InGameplaySystem;
}

void UGameplayAbilityTask::SetGameplayAbility(UGameplayAbility* InAbility)
{
	check(InAbility);

	Ability = InAbility;
}

bool UGameplayAbilityTask::ShouldBroadcastAbilityTaskDelegates() const
{
	bool bShouldBroadcast = (Ability && Ability->IsActive());

	return bShouldBroadcast;
}

void UGameplayAbilityTask::SetWaitingOnAvatar()
{
	if (IsValid(Ability) && GameplaySystem.IsValid())
	{
		WaitStateBitMask |= (uint8)EAbilityTaskWaitState::EAT_WaitingOnAvatar;
		Ability->NotifyAbilityTaskWaitingOnAvatar(this);
	}
}

void UGameplayAbilityTask::ClearWaitingOnAvatar()
{
	WaitStateBitMask &= ~((uint8)EAbilityTaskWaitState::EAT_WaitingOnAvatar);

}

bool UGameplayAbilityTask::IsWaitingOnAvatar() const
{
	return (WaitStateBitMask & (uint8)EAbilityTaskWaitState::EAT_WaitingOnAvatar) != 0;
}

FString UGameplayAbilityTask::GetDebugString() const
{
	return FString::Printf(TEXT("GameplayAbilityTask: %s (%s)"), *GetName(), *InstanceName.ToString());
}
