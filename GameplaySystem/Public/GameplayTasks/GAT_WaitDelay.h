// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTasks/GameplayAbilityTask.h"
#include "SharedTaskTypes.h"
#include "GAT_WaitDelay.generated.h"

/**
 * Waits a specified amount of time. TimeDilation-sensitive alternative to the Delay node.
 */
UCLASS()
class GAMEPLAYSYSTEM_API UGAT_WaitDelay : public UGameplayAbilityTask
{
	GENERATED_BODY()

public:

	UGAT_WaitDelay(const FObjectInitializer& ObjectInitializer);

	// Wait specified time. This is functionally the same as a standard Delay node, but counts with the activating Ability's relative TimeDilation. 
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "GameplayAbility|Tasks")
	static UGAT_WaitDelay* WaitDelay(UGameplayAbility* OwningAbility, float Time);

	virtual void Activate() override;

	virtual void TickTask(float DeltaTime) override;

	virtual FString GetDebugString() const override;

private:

	void OnDelayFinish();

	float DelayLength = 0.0f;
	float TimeAccumulated = 0.0f;
	bool bIsCounting = false;

public:
	// --- Delegates

	UPROPERTY(BlueprintAssignable)
	FWaitDelaySignature	OnWaitDelayFinishDelegate;
};
