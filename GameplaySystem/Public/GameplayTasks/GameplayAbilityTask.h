// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTask.h"
#include "GameplayAbility.h"
#include "GameplayAbilityTask.generated.h"

// --- Implementation is a customized version of Unreal's own AbilityTask for their AbilitySystemComponent framework, thats
//	   been modified to work with our own GameplaySystem framework. All credits to them for the original implementation.

enum class EAbilityTaskWaitState : uint8
{
	// Waiting for the game to do something 
	EAT_WaitingOnGame =		0x01		UMETA(DisplayName = "Waiting On Game"),

	// Waiting for the player to do something
	EAT_WaitingOnPlayer	=	0x02	UMETA(DisplayName = "Waiting On Player"),

	// Waiting on Character to do something
	EAT_WaitingOnAvatar	=	0x04	UMETA(DisplayName = "Waiting On Avatar"),
};

/**
 * GameplayTasks that are ensured to only operate within the lifetime of a GameplayAbility. 
 */
UCLASS(Abstract)
class GAMEPLAYSYSTEM_API UGameplayAbilityTask : public UGameplayTask
{
	GENERATED_BODY()
	
public:
	UGameplayAbilityTask(const FObjectInitializer& ObjectInitializer);

	// --- Begin UGameplayTask Interface
	virtual void OnDestroy(bool bInOwnerFinished) override;

	virtual void BeginDestroy() override;

	virtual void InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent) override;
	// --- End UGameplayTask Interface

	// Instantiates and initializes a new task
	template <class T>
	static T* NewAbilityTask(UGameplayAbility* ThisAbility, FName InstanceName = FName())
	{
		check(ThisAbility);

		T* NewTask = NewObject<T>();
		NewTask->InitTask(*ThisAbility, ThisAbility->GetGameplayTaskDefaultPriority());

		NewTask->InstanceName = InstanceName;
		return NewTask;
	}

	// Catch attempts at calling GameplayTask::NewTask through us
	template <class T>
	FORCEINLINE static T* NewTask(UObject* WorldContextObject, FName InstanceName = FName())
	{
		static_assert(DelayedFalse<T>(), "UGameplayTask::NewTask should never be used. Use NewAbilityTask instead");
		return nullptr;
	}

	// Returns the owning Ability's handle
	FGameplayAbilityHandle GetAbilityHandle() const;

	void SetGameplaySystemComponent(UGameplaySystemComponent* InGameplaySystem);

	void SetGameplayAbility(UGameplayAbility* InAbility);

	// This should be called prior to broadcasting delegates back into the ability graph. This makes sure the ability is still active.
	bool ShouldBroadcastAbilityTaskDelegates() const;

	// Called when the this task is waiting for ACharacter state.
	void SetWaitingOnAvatar();

	void ClearWaitingOnAvatar();

	virtual bool IsWaitingOnAvatar() const override;

	virtual FString GetDebugString() const override;

protected:

	// The Ability that created us
	UPROPERTY()
	TObjectPtr<UGameplayAbility> Ability;

	UPROPERTY()
	TWeakObjectPtr<UGameplaySystemComponent> GameplaySystem;

	uint8 WaitStateBitMask;
	uint8 bWasSuccessfullyDestroyed : 1 = false;
};
