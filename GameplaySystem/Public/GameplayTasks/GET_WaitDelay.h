// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTasks/GameplayEventTask.h"
#include "SharedTaskTypes.h"
#include "GET_WaitDelay.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UGET_WaitDelay : public UGameplayEventTask
{
	GENERATED_BODY()
	
public:

	UGET_WaitDelay(const FObjectInitializer& ObjectInitializer);

	// Wait specified time. This is functionally the same as a standard Delay node, but counts with the activating Events's relative TimeDilation. 
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningEvent", DefaultToSelf = "OwningEvent", BlueprintInternalUseOnly = "true"), Category = "GameplayEvent|Tasks")
	static UGET_WaitDelay* WaitDelay(UGameplayEvent* OwningEvent, float Time);

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
