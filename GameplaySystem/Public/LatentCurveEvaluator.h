// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Tickable.h"
#include "GameplayEventTypes.h"

#include "LatentCurveEvaluator.generated.h"

class UCurveFloat;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnUpdateEvaluationSignature, float, Value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnFinishedEvaluationSignature, float, FinalValue);

/**
 * Evaluates a CurveFloat, allowing objects to process on each update tick based on the curves value.
 * An alternative to Timeline that works on any UObject. 
 */
UCLASS(BlueprintType)
class GAMEPLAYSYSTEM_API ULatentCurveEvaluator : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	ULatentCurveEvaluator() {};

	// FTickableObject Begin
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual bool IsTickableWhenPaused() const override;
	//FTickableObject End

	virtual void FinishDestroy() override;

	UFUNCTION()
	void TickCurve(float DeltaTime);
 	
	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void Play();

	// Stops the Evaluator. If bEvaluateLastKey is true, will evaluate and broadcast the OnFinishedEvaluationDelegate with the last key in the curve.
	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void Stop(bool bBroadcastLastKey = false);

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void ReverseFromEnd();

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	bool HasFinishedEvaluating() const;

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void AssignCurve(UCurveFloat* InCurve);

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void SetEndTime(float InEndTime);

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void SetUpdateDelegate(const FOnUpdateEvaluationSignature& InUpdateDelegate);

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void SetFinishDelegate(const FOnFinishedEvaluationSignature& InFinishDelegate);

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void SetUpdatingPolicy(bool bInEvaluateWhenPaused);

	// Will source the DeltaTime from Leader while evaluating, allowing the LatentCurveEvaluator to match the objects ticking rate.
	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void SetLeaderTickObject(UPARAM(ref) FObjectTickFollowers& LeaderTickObject);

protected:

	float EvaluateCurve();

private:
	UCurveFloat* Curve;

	FTickFollowerHandle TickFollowerHandle;

	bool bReliesOnLeaderForTick = false;

	float ElapsedTime = 0;

	float TargetTime = 0;

	bool bEvaluateWhenPaused = false;
	
	bool bIsActive = false;

	// Count down instead of up
	bool bIsReversed = false;

	FOnUpdateEvaluationSignature OnUpdateEvaluationDelegate;
	FOnFinishedEvaluationSignature OnFinishedEvaluationDelegate;
};
