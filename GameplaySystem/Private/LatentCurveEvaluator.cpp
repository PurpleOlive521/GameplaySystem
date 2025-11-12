// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "LatentCurveEvaluator.h"

#include "DevelopmentTypes.h"

// Should always be a valid tick, since we define IsTickable to depend on if we are Active already
void ULatentCurveEvaluator::Tick(float DeltaTime)
{
    ElapsedTime += DeltaTime;

    const bool IsFinished = HasFinishedEvaluating();

    if (IsFinished)
    {
        Stop();
        return;
    }

    const float EvaluatedValue = EvaluateCurve();

    OnUpdateEvaluationDelegate.ExecuteIfBound(EvaluatedValue);
}

ETickableTickType ULatentCurveEvaluator::GetTickableTickType() const
{
    // Set to Conditional to ensure that CDO is not marked for ticking
    return ETickableTickType::Conditional;
}

bool ULatentCurveEvaluator::IsTickable() const
{
    // No ticking for CDO
    if (HasAnyFlags(RF_ClassDefaultObject))
    {
        return false;
    }

    return bIsActive;
}

TStatId ULatentCurveEvaluator::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(ULatentCurveEvaluator, STATGROUP_Tickables);
}

UWorld* ULatentCurveEvaluator::GetTickableGameObjectWorld() const
{
    return GetWorld();
}

bool ULatentCurveEvaluator::IsTickableWhenPaused() const
{
    return bEvaluateWhenPaused;
}

//FTickableObject End


void ULatentCurveEvaluator::FinishDestroy()
{
    Super::FinishDestroy();
}

void ULatentCurveEvaluator::Play()
{
    // Can't reactivate when already active.
    if (bIsActive)
    {
        return;
    }

    ElapsedTime = 0;
    bIsActive = true;
}

void ULatentCurveEvaluator::Stop()
{
    // Can't stop if not already active
    if (!bIsActive)
    {
        return;
    }

    bIsActive = false;

    // Move ahead to the last key and sample it's value, to ensure that we end in the same state as it would have if evaluated continously.
    ElapsedTime = TargetTime;
    const float EvaluatedValue = EvaluateCurve();

    OnUpdateEvaluationDelegate.Clear();

    OnFinishedEvaluationDelegate.ExecuteIfBound(EvaluatedValue);
    OnFinishedEvaluationDelegate.Clear();
}

bool ULatentCurveEvaluator::HasFinishedEvaluating() const
{
    if (ElapsedTime >= TargetTime)
    {
        return true;
    }

    if (!bIsActive)
    {
        return true;
    }

    return false;
}

void ULatentCurveEvaluator::AssignCurve(UCurveFloat* InCurve)
{
    check(InCurve);

    Curve = InCurve;
}

void ULatentCurveEvaluator::SetEndTime(float InEndTime)
{
    if(InEndTime == 0.0f)
    {
        if (Curve)
        {
            // Assume that we want to evaluate until the end of the curve is hit
            TargetTime = Curve->FloatCurve.GetLastKey().Time;
        }
    }
    else
    {
		TargetTime = InEndTime;
    }
}

void ULatentCurveEvaluator::SetUpdateDelegate(const FOnUpdateEvaluationSignature& InUpdateDelegate)
{
	OnUpdateEvaluationDelegate = InUpdateDelegate;
}

void ULatentCurveEvaluator::SetFinishDelegate(const FOnFinishedEvaluationSignature& InFinishDelegate)
{
	OnFinishedEvaluationDelegate = InFinishDelegate;
}

void ULatentCurveEvaluator::SetUpdatingPolicy(bool bInEvaluateWhenPaused)
{
	bEvaluateWhenPaused = bInEvaluateWhenPaused;
}

float ULatentCurveEvaluator::EvaluateCurve()
{
    ElapsedTime = FMath::Clamp(ElapsedTime, 0.0f, TargetTime);

    if (Curve)
    {
        return Curve->GetFloatValue(ElapsedTime);
    }


    return 0.0f;
}
