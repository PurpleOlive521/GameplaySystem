// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTasks/GameplayAbilityTask.h"
#include "GAT_StartAbilityState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAbilityStateSignature);

static const FName END_ALL_STATES = NAME_None;

/**
 * Starts a Ability State. Can only be active within the scope of this Ability activation.
 */
UCLASS()
class GAMEPLAYSYSTEM_API UGAT_StartAbilityState : public UGameplayAbilityTask
{
	GENERATED_BODY()

public:

	UGAT_StartAbilityState(const FObjectInitializer& ObjectInitializer);

	/**
	 * Starts a new ability state.
	 *
	 * @param StateName The name of the state.
	 * @param bEndCurrentState If true, all other active ability states will be ended.
	 */
	UFUNCTION(BlueprintCallable, Meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "GameplayAbility|Tasks")
	static UGAT_StartAbilityState* StartAbilityState(UGameplayAbility* OwningAbility, FName StateName, bool bEndCurrentState = true);

	virtual void Activate() override;

	virtual void ExternalCancel() override;

	virtual void OnDestroy(bool AbilityEnded) override;

	virtual FString GetDebugString() const override;

protected:

	void OnEndState(FName StateNameToEnd);
	void OnInterruptState();

	FDelegateHandle EndStateHandle;
	FDelegateHandle InterruptStateHandle;

	bool bWasEnded = false;
	bool bWasInterrupted = false;

	bool bEndCurrentState = true;

public: 
	// --- Delegates

	// Invoked if 'EndAbilityState' is called or if 'EndAbility' is called and this state is active.
	UPROPERTY(BlueprintAssignable)
	FOnAbilityStateSignature OnStateEndedDelegate;

	// Invoked if the ability was interrupted and this state is active. 
	UPROPERTY(BlueprintAssignable)
	FOnAbilityStateSignature OnStateInterruptedDelegate;
};
