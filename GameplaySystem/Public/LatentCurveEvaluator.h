// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "Tickable.h"
#include "GameplayEventTypes.h"

#include "LatentCurveEvaluator.generated.h"

class UCurveFloat;

constexpr float NO_TARGET_TIME = -1.0f;

UENUM(BlueprintType)
enum class EPlayDirection : uint8
{
	EPD_Forward		UMETA(DisplayName = "Forward"),
	EPD_Backward	UMETA(DisplayName = "Backward"),
};

UENUM()
enum class EEvaluatorPlayTypePins : uint8
{
	// Resume playing. Same as PlayFromStart for newly created evaluators.
	Play,

	// Play from the beginning of the curve, evaluating towards the end.
	PlayFromStart,

	// Play from the end of the curve, evaluating towards the beginning.
	ReverseFromEnd,
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnEvaluateSignature, float, Value);
DECLARE_DYNAMIC_DELEGATE(FOnFinishedSignature);

/**
 * Evaluates a CurveFloat, allowing objects to process on each update tick based on the curves value.
 * An alternative to Timeline that works on any UObject. 
 */
UCLASS(BlueprintType)
class GAMEPLAYSYSTEM_API ULatentCurveEvaluator : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	ULatentCurveEvaluator() = default;

	// --- Begin FTickableObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual bool IsTickableWhenPaused() const override;
	// --- End FTickableObject Interface

	virtual void FinishDestroy() override;

	UFUNCTION()
	void TickCurve(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void PlayByType(EEvaluatorPlayTypePins PlayType);
 	
	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void Play();

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void PlayFromStart();

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void ReverseFromEnd();

	// Stops the Evaluator. If bBroadcastLastKey is true, will evaluate and broadcast the OnFinishedEvaluationDelegate with the last key in the curve.
	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void Stop(bool bBroadcastLastKey = false);

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void SetPlayDirection(EPlayDirection Direction);

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	bool HasFinishedEvaluating() const;

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void AssignCurve(UCurveFloat* InCurve);

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void SetEndTime(float InEndTime);

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void SetUpdateDelegate(const FOnEvaluateSignature& InUpdateDelegate);

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void SetFinishDelegate(const FOnFinishedSignature& InFinishDelegate);

	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void SetUpdatingPolicy(bool bInEvaluateWhenPaused);

	// Will source the DeltaTime from Leader while evaluating, allowing the LatentCurveEvaluator to match the objects ticking rate.
	UFUNCTION(BlueprintCallable, Category = "LatentCurveEvaluator")
	void SetLeaderTickObject(UPARAM(ref) FObjectTickFollowers& LeaderTickObject);

	float GetLastKey() const;

	float ForceEvaluateAt(float InTime);

	// Sets the current progress, e.g. the time we evaluate at and count from.
	void SetEvaluatedTime(float InTime);

	// Evaluates at the current state of progress.
	float EvaluateCurve();

	void DisableTicking();

private:
	TObjectPtr<UCurveFloat> Curve = nullptr;

	FTickFollowerHandle TickFollowerHandle;

	float EvaluatedTime = 0.0f;

	float TargetTime = NO_TARGET_TIME;

	float StartTime = 0.0f;

	uint32 bEvaluateWhenPaused : 1 = false;
	
	uint32 bIsActive : 1 = false;

	// We are registered to a Leader that ticks us 
	uint32 bReliesOnLeaderForTick : 1 = false;

	// Some other non-leader object is responsible for ticking us directly
	uint32 bHasDisabledTicking : 1 = false;
	
	EPlayDirection Direction = EPlayDirection::EPD_Forward;

	FOnEvaluateSignature OnUpdateDelegate;
	FOnFinishedSignature OnFinishedDelegate;
};
