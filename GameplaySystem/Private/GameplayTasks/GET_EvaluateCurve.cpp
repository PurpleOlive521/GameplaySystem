// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayTasks/GET_EvaluateCurve.h"
#include "LatentCurveEvaluatorBlueprintLibrary.h"

UGET_EvaluateCurve::UGET_EvaluateCurve(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bTickingTask = true;
}

UGET_EvaluateCurve* UGET_EvaluateCurve::EvaluateCurve(EEvaluatorPlayTypePins PlayType, UGameplayEvent* OwningEvent, UCurveFloat* InCurve, bool bAlwaysEvaluateLastKey, bool bScaleToEndTime, float EndTime)
{
	UGET_EvaluateCurve* NewTask = NewEventTask<UGET_EvaluateCurve>(OwningEvent);

	FLatentCurveEvaluatorParams Params;
	Params.Curve = InCurve;
	Params.EndTime = EndTime;
	Params.bScaleToEndTime = bScaleToEndTime;

	NewTask->Params = Params;
	NewTask->bAlwaysEvaluateLastKey = bAlwaysEvaluateLastKey;
	NewTask->PlayType = PlayType;

	return NewTask;
}

void UGET_EvaluateCurve::Activate()
{
	if (!Params.Curve)
	{
		EndTask();
		return;
	}

	Event->OnEventAbortedDelegate.AddUObject(this, &UGET_EvaluateCurve::OnGameplayEventAborted);

	Params.OnEvaluateDelegate.BindDynamic(this, &UGET_EvaluateCurve::OnCurveEvaluated);
	Params.OnFinishedDelegate.BindDynamic(this, &UGET_EvaluateCurve::OnCurveFinished);

	Evaluator = ULatentCurveEvaluatorBlueprintLibrary::CreateLatentCurveEvaluator(this, Params);
	ensure(Evaluator);

	// We will take over ticking to ensure it matches our owning Events tick.
	Evaluator->DisableTicking();

	Evaluator->PlayByType(PlayType);
}

void UGET_EvaluateCurve::TickTask(float DeltaTime)
{
	if (Evaluator)
	{
		const float RelativeDeltaTime = DeltaTime * Event->GetDeltaTimeCoefficient();

		Evaluator->Tick(RelativeDeltaTime);
	}
}

void UGET_EvaluateCurve::ExternalCancel()
{
	Super::ExternalCancel();
}

FString UGET_EvaluateCurve::GetDebugString() const
{
	return FString::Printf(TEXT("EvaluateCurve: %s"), *GetNameSafe(Params.Curve));
}

void UGET_EvaluateCurve::OnCurveEvaluated(float Value)
{
	if (ShouldBroadcastEventTaskDelegates())
	{
		OnCurveEvaluatedDelegate.Broadcast(Value);
	}
}

void UGET_EvaluateCurve::OnCurveFinished()
{
	if (ShouldBroadcastEventTaskDelegates())
	{
		OnCurveFinishedDelegate.Broadcast();
	}

	EndTask();
}

void UGET_EvaluateCurve::OnGameplayEventAborted()
{
	EndTask();
}

void UGET_EvaluateCurve::OnDestroy(bool AbilityEnded)
{
	// Unbind delegates so this doesn't get recursively called
	if (Event)
	{
		Event->OnEventAbortedDelegate.Remove(EventAbortedHandle);
	}

	if (Evaluator)
	{
		bool bEvaluateLastKey = false;

		if (Evaluator->HasFinishedEvaluating())
		{
			if (ShouldBroadcastEventTaskDelegates() && bAlwaysEvaluateLastKey)
			{
				bEvaluateLastKey = true;
			}
		}

		Evaluator->Stop(bEvaluateLastKey);
	}

	Super::OnDestroy(AbilityEnded);
}
