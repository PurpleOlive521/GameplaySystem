// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "LatentCurveEvaluatorBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

ULatentCurveEvaluator* ULatentCurveEvaluatorBlueprintLibrary::CreateLatentCurveEvaluator(UCurveFloat* InCurve, UObject* Owner, const FOnEvaluateSignature& OnUpdateEvaluationFunction, const FOnFinishedSignature& OnFinishedEvaluationFunction, float EndTime, bool bEvaluateWhenPaused)
{
	ULatentCurveEvaluator* CurveEvaluator = NewObject<ULatentCurveEvaluator>(Owner, ULatentCurveEvaluator::StaticClass());

	CurveEvaluator->AssignCurve(InCurve);
	CurveEvaluator->SetUpdateDelegate(OnUpdateEvaluationFunction);
	CurveEvaluator->SetFinishDelegate(OnFinishedEvaluationFunction);
	CurveEvaluator->SetEndTime(EndTime);
	CurveEvaluator->SetUpdatingPolicy(bEvaluateWhenPaused);

	return CurveEvaluator;
}

ULatentCurveEvaluator* ULatentCurveEvaluatorBlueprintLibrary::CreateAndPlayLatentCurveEvaluator(EEvaluatorPlayTypePins PlayType, UCurveFloat* InCurve, UObject* Owner, const FOnEvaluateSignature& OnUpdateEvaluationFunction, const FOnFinishedSignature& OnFinishedEvaluationFunction, float EndTime, bool bEvaluateWhenPaused)
{
	ULatentCurveEvaluator* CreatedCurve = CreateLatentCurveEvaluator(InCurve, Owner, OnUpdateEvaluationFunction, OnFinishedEvaluationFunction, EndTime, bEvaluateWhenPaused);
	ensure(CreatedCurve);

	CreatedCurve->PlayByType(PlayType);

	return CreatedCurve;
}

