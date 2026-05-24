// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTasks/GameplayEventTask.h"
#include "LatentCurveEvaluator.h"
#include "GET_EvaluateCurve.generated.h"

class ULatentCurveEvaluator;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCurveEvaluatedSignature, float, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCurveFinishedSignature);

/**
 * 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UGET_EvaluateCurve : public UGameplayEventTask
{
	GENERATED_BODY()
	
public:

	UGET_EvaluateCurve(const FObjectInitializer& ObjectInitializer);

	// Starts a Evaluator that is tied to the GameplayEvent. 
	// @param PlayType					The play type we use to start evaluation.
	// @param InCurve					The CurveFloat to evaluate.
	// @param bAlwaysEvaluateLastKey	Whether to evaluate the last key in the curve if ended prematurely.
	// @param EndTime					The time we evaluate to. If left at -1, will evaluate till the last key in the curve.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningEvent", DefaultToSelf = "OwningEvent", BlueprintInternalUseOnly = "true"), Category = "GameplayEvent|Tasks")
	static UGET_EvaluateCurve* EvaluateCurve(EEvaluatorPlayTypePins PlayType, UGameplayEvent* OwningEvent, UCurveFloat* InCurve, bool bAlwaysEvaluateLastKey, bool bScaleToEndTime, float EndTime = -1.0f);

	virtual void Activate() override;

	virtual void TickTask(float DeltaTime) override;

	virtual void ExternalCancel() override;

	virtual FString GetDebugString() const override;

	UFUNCTION()
	void OnCurveEvaluated(float Value);

	UFUNCTION()
	void OnCurveFinished();

	UFUNCTION()
	void OnGameplayEventAborted();

protected:

	virtual void OnDestroy(bool AbilityEnded) override;

	UPROPERTY()
	TObjectPtr<ULatentCurveEvaluator> Evaluator = nullptr;

	UPROPERTY()
	FLatentCurveEvaluatorParams Params;

	bool bAlwaysEvaluateLastKey = false;

	EEvaluatorPlayTypePins PlayType = EEvaluatorPlayTypePins::PlayFromStart;

	FDelegateHandle EventAbortedHandle;

public:
	// --- Delegates

	// Called each tick
	UPROPERTY(BlueprintAssignable)
	FOnCurveEvaluatedSignature OnCurveEvaluatedDelegate;

	// Called when the curve is finished evaluating
	UPROPERTY(BlueprintAssignable)
	FOnCurveFinishedSignature OnCurveFinishedDelegate;
};
