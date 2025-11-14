// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#include "LatentCurveEvaluatorBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

ULatentCurveEvaluator* ULatentCurveEvaluatorBlueprintLibrary::CreateLatentCurveEvaluator(UCurveFloat* InCurve, UObject* Owner, const FOnUpdateEvaluationSignature& OnUpdateEvaluationFunction, const FOnFinishedEvaluationSignature& OnFinishedEvaluationFunction, float EndTime, bool bEvaluateWhenPaused)
{
	ULatentCurveEvaluator* CurveEvaluator = StaticCast<ULatentCurveEvaluator*>(UGameplayStatics::SpawnObject(ULatentCurveEvaluator::StaticClass(), Owner));

	CurveEvaluator->AssignCurve(InCurve);
	CurveEvaluator->SetUpdateDelegate(OnUpdateEvaluationFunction);
	CurveEvaluator->SetFinishDelegate(OnFinishedEvaluationFunction);
	CurveEvaluator->SetEndTime(EndTime);
	CurveEvaluator->SetUpdatingPolicy(bEvaluateWhenPaused);

	return CurveEvaluator;
}

ULatentCurveEvaluator* ULatentCurveEvaluatorBlueprintLibrary::CreateAndPlayLatentCurveEvaluator(UCurveFloat* InCurve, UObject* Owner, const FOnUpdateEvaluationSignature& OnUpdateEvaluationFunction, const FOnFinishedEvaluationSignature& OnFinishedEvaluationFunction, float EndTime, bool bEvaluateWhenPaused)
{
	ULatentCurveEvaluator* CreatedCurve = CreateLatentCurveEvaluator(InCurve, Owner,OnUpdateEvaluationFunction, OnFinishedEvaluationFunction, EndTime, bEvaluateWhenPaused);

	CreatedCurve->Play();

	return CreatedCurve;
}

