// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayTasks/GAT_WaitInputState.h"
#include "Kismet/GameplayStatics.h"
#include "DevelopmentTypes.h"

UGAT_WaitInputState::UGAT_WaitInputState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}


UGAT_WaitInputState* UGAT_WaitInputState::WaitInputState(UGameplayAbility* OwningAbility, const UInputAction* Input, ETriggerEvent DesiredState, bool bTestAlreadyPressed)
{
	UGAT_WaitInputState* NewTask = NewAbilityTask<UGAT_WaitInputState>(OwningAbility);
	NewTask->bTestInitialState = bTestAlreadyPressed;
	NewTask->BoundInput = Input;
	NewTask->AwaitedState = DesiredState;

	return NewTask;
}

void UGAT_WaitInputState::Activate()
{
	StartTime = GetWorld()->GetTimeSeconds();

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController)
	{
		GS_LOG(Warning, TEXT("UGAT_WaitInputState: No PlayerController found!"));
		return;
	}

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerController->InputComponent);
	if (!EnhancedInput)
	{
		GS_LOG(Warning, TEXT("UGAT_WaitInputState: No EnhancedInputComponent found!"));
		return;
	}

	// This is a bit of a lie, since we can't check the actual TriggerEvent state. But we can infer some of the important ones,
	// such as if it is even pressed or not to begin with.
	if (bTestInitialState)
	{
		FInputActionValue ActionValue = EnhancedInput->GetBoundActionValue(BoundInput);

		const bool bIsNotZero = ActionValue.IsNonZero();
		bool bAlreadyInState = false;

		switch (AwaitedState)
		{
		case ETriggerEvent::None:
			bAlreadyInState = !bIsNotZero;
			break;
		case ETriggerEvent::Started:
		case ETriggerEvent::Ongoing:
		case ETriggerEvent::Triggered:
		case ETriggerEvent::Completed:
			bAlreadyInState = bIsNotZero;
			break;
		}

		if (bAlreadyInState)
		{
			OnInputAction();
			return;
		}
	}

	EnhancedInput->BindAction(BoundInput, AwaitedState, this, &UGAT_WaitInputState::OnInputActionCallback);
}

FString UGAT_WaitInputState::GetDebugString() const
{
	return FString::Printf(TEXT("GameplayAbilityTask: %s (%s)"), *GetName(), *InstanceName.ToString());
}

void UGAT_WaitInputState::OnInputActionCallback(const FInputActionInstance& Instance)
{
	OnInputAction();
}

void UGAT_WaitInputState::OnInputAction()
{
	float ElapsedTime = GetWorld()->GetTimeSeconds() - StartTime;

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnInputStateReached.Broadcast(ElapsedTime);
	}

	EndTask();
}
