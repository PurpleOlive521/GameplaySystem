// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTasks/GameplayAbilityTask.h"
#include "GAT_StartTickable.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStartTickableSignature, float, DeltaTime);

/**
 * Starts a Tickable GameplayTask, allowing the Ability to get Tick callbacks like an Actor would. 
 * Deltatime should match the GameplaySystems owner, and is affected by TimeDilation.
 */
UCLASS()
class GAMEPLAYSYSTEM_API UGAT_StartTickable : public UGameplayAbilityTask
{
	GENERATED_BODY()

public:

	UGAT_StartTickable(const FObjectInitializer& ObjectInitializer);

	// Starts a Tickable GameplayTask. Enables Tick callbacks for the GameplayAbility in the same way an Actor would.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "GameplayAbility|Tasks")
	static UGAT_StartTickable* StartTickable(UGameplayAbility* OwningAbility);

	virtual void Activate() override;

	virtual void TickTask(float DeltaTime) override;

	virtual FString GetDebugString() const override;

	// Effectively ends the GameplayTask.
	UFUNCTION(BlueprintCallable)
	void StopTicking();

private:

	bool bIsTicking = false;

public:
	// --- Delegates

	UPROPERTY(BlueprintAssignable)
	FStartTickableSignature OnTickDelegate;
};
