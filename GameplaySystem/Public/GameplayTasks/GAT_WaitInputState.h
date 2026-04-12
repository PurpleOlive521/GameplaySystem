// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTasks/GameplayAbilityTask.h"
#include "EnhancedInputComponent.h"
#include "GAT_WaitInputState.generated.h"

class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputStateSignature, float, TimeWaited);

/**
 *	Waits until the desired state is activated in the specified InputAction.
 */
UCLASS()
class GAMEPLAYSYSTEM_API UGAT_WaitInputState : public UGameplayAbilityTask
{
	GENERATED_BODY()
	
public:

	UGAT_WaitInputState(const FObjectInitializer& ObjectInitializer);

	// Wait until the user triggers the desired the input button for this ability's activation. Returns time this node spent waiting for the press. Will return 0 if input was already down.
	// bTestAlreadyInState will infer from current button state, and is not accurate. Use for testing "Is pressed" states only.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "GameplayAbility|Tasks")
	static UGAT_WaitInputState* WaitInputState(UGameplayAbility* OwningAbility, const UInputAction* Input, ETriggerEvent DesiredState, bool bTestAlreadyInState = false);

	virtual void Activate() override;

	virtual FString GetDebugString() const override;

	UFUNCTION()
	void OnInputActionCallback(const FInputActionInstance& Instance);

	void OnInputAction();

protected:

	float StartTime = 0.0f;

	bool bTestInitialState = false;

	TObjectPtr<const UInputAction> BoundInput = nullptr;

	ETriggerEvent AwaitedState = ETriggerEvent::None;

	FDelegateHandle DelegateHandle;

public:
	// --- Delegates

	UPROPERTY(BlueprintAssignable)
	FOnInputStateSignature OnInputStateReached;
};
