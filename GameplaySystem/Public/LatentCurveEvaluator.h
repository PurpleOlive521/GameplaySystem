// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Tickable.h"

#include "LatentCurveEvaluator.generated.h"

class UCurveFloat;

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnUpdateEvaluationSignature, float, Value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnFinishedEvaluationSignature, float, FinalValue);

/**
 * Evalutes a CurveFloat, allowing objects to process on each update tick based on the curves value.
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
 	
	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void Play();

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void Stop();

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

protected:

	float EvaluateCurve();

private:
	UCurveFloat* Curve;

	float ElapsedTime = 0;

	float TargetTime = 0;

	bool bEvaluateWhenPaused = false;
	
	bool bIsActive = false;

	FOnUpdateEvaluationSignature OnUpdateEvaluationDelegate;
	FOnFinishedEvaluationSignature OnFinishedEvaluationDelegate;
};
