// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTasks/GameplayAbilityTask.h"
#include "GameplayEvent.h"
#include "GameplayEventHandle.h"
#include "GAT_TriggerGameplayEvent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameplayEventAbortedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameplayEventEndedSignature);

/**
 * 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UGAT_TriggerGameplayEvent : public UGameplayAbilityTask
{
	GENERATED_BODY()
	
public:
	UGAT_TriggerGameplayEvent(const FObjectInitializer& ObjectInitializer);

	// Starts a GameplayEvent. 
	// OptionalData		Data to pass to the triggering GameplayEvent
	// bAbortOnCancel	Aborts the GameplayEvent when the Ability is cancelled
	// bAbortOnEnd		Aborts the GameplayEvent when the Ability is ended.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "GameplayAbility|Tasks")
	static UGAT_TriggerGameplayEvent* TriggerGameplayEvent(UGameplayAbility* OwningAbility, TSubclassOf<UGameplayEvent> EventClass, 
		FGameplayEventActivationData OptionalData = FGameplayEventActivationData(), bool bAbortOnCancel = false, bool bAbortOnEnd = false);

	virtual void Activate() override;

	virtual void ExternalCancel() override;

	virtual void OnDestroy(bool bAbilityEnded) override;

	virtual FString GetDebugString() const override;

	UFUNCTION(BlueprintCallable)
	void AbortEvent();

	void OnEventEnded(UGameplayEvent* Event);

	void OnAbilityCancelled();

protected:

	FGameplayEventHandle EventHandle;
	FDelegateHandle AbilityCancelledHandle;

	bool bAbortOnCancel = false;
	bool bAbortOnEnd = false;

	TSubclassOf<UGameplayEvent> EventClass = nullptr;
	FGameplayEventActivationData ActivationData;

	bool bWasAborted = false;

public:
	// --- Delegates
	UPROPERTY(BlueprintAssignable)
	FOnGameplayEventAbortedSignature OnGameplayEventAbortedDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnGameplayEventEndedSignature OnGameplayEventEndedDelegate;
};
