// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "LatentCurveEvaluator.h"

#include "DevelopmentTypes.h"

// Should always be a valid tick, since we define IsTickable to depend on if we are Active already
void ULatentCurveEvaluator::Tick(float DeltaTime)
{
    TickCurve(DeltaTime);
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

    return bIsActive && !bReliesOnLeaderForTick && !bHasDisabledTicking;
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
    return Params.bEvaluateWhenPaused;
}

//FTickableObject End

void ULatentCurveEvaluator::FinishDestroy()
{
    Super::FinishDestroy();
}

void ULatentCurveEvaluator::TickCurve(float DeltaTime)
{
    if (!bIsActive)
    {
        return;
    }
    
    const float Coefficient = Direction == EPlayDirection::EPD_Forward ? 1.0f : -1.0f;
    EvaluatedTime += DeltaTime * Coefficient * TimeScale;

    SetEvaluatedTime(EvaluatedTime);

    const float EvaluatedValue = EvaluateCurve();

    Params.OnEvaluateDelegate.ExecuteIfBound(EvaluatedValue);

    const bool IsFinished = HasFinishedEvaluating();
    if (IsFinished)
    {
        bIsActive = false;
        Params.OnFinishedDelegate.Execute();
    }
}

void ULatentCurveEvaluator::PlayByType(EEvaluatorPlayTypePins PlayType)
{
    switch (PlayType)
    {
    case(EEvaluatorPlayTypePins::Play):
    {
        Play();
        break;
    }

    case(EEvaluatorPlayTypePins::ReverseFromEnd):
    {
        ReverseFromEnd();
        break;
    }

    case(EEvaluatorPlayTypePins::PlayFromStart):
    {
        PlayFromStart();
        break;
    }

    default:
        checkNoEntry(); // Type not supported yet.
    }
}

void ULatentCurveEvaluator::Play()
{
    // Can't reactivate when already active.
    if (bIsActive)
    {
        return;
    }

    bIsActive = true;
}

void ULatentCurveEvaluator::PlayFromStart()
{
    // Can't reactivate when already active.
    if (bIsActive)
    {
        return;
    }

    bIsActive = true;

    StartTime = 0.0f;
    EvaluatedTime = StartTime;
    TargetTime = GetLastKey();
    Direction = EPlayDirection::EPD_Forward;
}

void ULatentCurveEvaluator::ReverseFromEnd()
{
    // Can't reactivate when already active.
    if (bIsActive)
    {
        return;
    }

    bIsActive = true;

    StartTime = GetLastKey();
    EvaluatedTime = StartTime;
    TargetTime = 0.0f;
    Direction = EPlayDirection::EPD_Backward;
}

void ULatentCurveEvaluator::Stop(bool bBroadcastLastKey)
{
    // Can't stop if not already active
    if (!bIsActive)
    {
        return;
    }

    bIsActive = false;

    if (bBroadcastLastKey)
    {
        // Move ahead to the target key and sample it's value, to ensure that we end in the same state as it would have if evaluated continously.
        const float EvaluatedValue = ForceEvaluateAt(TargetTime);
        Params.OnEvaluateDelegate.Execute(EvaluatedValue);
        Params.OnFinishedDelegate.ExecuteIfBound();
    }
}

void ULatentCurveEvaluator::SetPlayDirection(EPlayDirection InDirection)
{
    if (Direction == InDirection)
    {
        return;
    }

    Direction = InDirection;

    // Swap start and end time
    float Temp = StartTime;
    StartTime = TargetTime;
    TargetTime = Temp;
}

bool ULatentCurveEvaluator::HasFinishedEvaluating() const
{
    if (!bIsActive)
    {
        return true;
    }

    switch (Direction)
    {
        case EPlayDirection::EPD_Forward: 
        {
            if (EvaluatedTime >= TargetTime)
            {
                return true;
            }

            break;
        }

        case EPlayDirection::EPD_Backward: 
        {
            if (EvaluatedTime <= TargetTime)
            {
                return true;
            }

            break;
        }

        default:
            checkNoEntry(); // Not supported yet.
    }

    return false;
}

void ULatentCurveEvaluator::SetEndTime(float InEndTime)
{
    if(InEndTime == NO_TARGET_TIME)
    {
        // Assume that we want to evaluate until the end of the curve is hit
        TargetTime = GetLastKey();
        TimeScale = 1.0f;
    }
    else
    {
		TargetTime = InEndTime;

        if (Params.bScaleToEndTime)
        {
            TimeScale = GetLastKey() / TargetTime;
        }
    }
}

void ULatentCurveEvaluator::SetLeaderTickObject(FObjectTickFollowers& LeaderTickObject)
{
    FOnLeaderTickSignature TickDelegate;
	TickDelegate.BindUObject(this, &ULatentCurveEvaluator::TickCurve);
    LeaderTickObject.AddTickFollower(TickDelegate);

	bReliesOnLeaderForTick = true;
}

float ULatentCurveEvaluator::EvaluateCurve()
{
    if (!Params.Curve)
    {
        return 0.0f;
    }

    return Params.Curve->GetFloatValue(EvaluatedTime);
}

void ULatentCurveEvaluator::DisableTicking()
{
    bHasDisabledTicking = true;
}

void ULatentCurveEvaluator::SetProperties(const FLatentCurveEvaluatorParams& InParams)
{
    Params = InParams;
    SetEndTime(Params.EndTime);
}

float ULatentCurveEvaluator::GetLastKey() const
{
    if (!Params.Curve || Params.Curve->FloatCurve.IsEmpty())
    {
        return 0.0f;
    }

    return Params.Curve->FloatCurve.GetLastKey().Time;
}

float ULatentCurveEvaluator::ForceEvaluateAt(float InTime)
{
    if (!Params.Curve)
    {
        return 0.0f;
    }

    return Params.Curve->GetFloatValue(InTime);
}

void ULatentCurveEvaluator::SetEvaluatedTime(float InTime)
{
    EvaluatedTime = InTime;

    const bool bClampForward = Direction == EPlayDirection::EPD_Forward && EvaluatedTime > TargetTime;
    const bool bClampBackward = Direction == EPlayDirection::EPD_Backward && EvaluatedTime < TargetTime;
    if (bClampForward || bClampBackward)
    {
        EvaluatedTime = TargetTime;
    }
}


