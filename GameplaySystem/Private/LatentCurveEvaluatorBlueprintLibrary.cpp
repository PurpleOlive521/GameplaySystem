// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.

#include "LatentCurveEvaluatorBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

ULatentCurveEvaluator* ULatentCurveEvaluatorBlueprintLibrary::CreateLatentCurveEvaluator(UCurveFloat* InCurve, UObject* Owner, const FOnUpdateEvaluationSignature& OnUpdateEvaluationFunction, const FOnFinishedEvaluationSignature& OnFinishedEvaluationFunction, float EndTime, bool bEvaluateWhenPaused)
{
	ULatentCurveEvaluator* CurveEvaluator = NewObject<ULatentCurveEvaluator>(Owner, ULatentCurveEvaluator::StaticClass());

	CurveEvaluator->AssignCurve(InCurve);
	CurveEvaluator->SetUpdateDelegate(OnUpdateEvaluationFunction);
	CurveEvaluator->SetFinishDelegate(OnFinishedEvaluationFunction);
	CurveEvaluator->SetEndTime(EndTime);
	CurveEvaluator->SetUpdatingPolicy(bEvaluateWhenPaused);

	return CurveEvaluator;
}

ULatentCurveEvaluator* ULatentCurveEvaluatorBlueprintLibrary::CreateAndPlayLatentCurveEvaluator(EPlayTypePins PlayType, UCurveFloat* InCurve, UObject* Owner, const FOnUpdateEvaluationSignature& OnUpdateEvaluationFunction, const FOnFinishedEvaluationSignature& OnFinishedEvaluationFunction, float EndTime, bool bEvaluateWhenPaused)
{
	ULatentCurveEvaluator* CreatedCurve = CreateLatentCurveEvaluator(InCurve, Owner, OnUpdateEvaluationFunction, OnFinishedEvaluationFunction, EndTime, bEvaluateWhenPaused);

	switch (PlayType)
	{
		case(EPlayTypePins::Play):
		{
			CreatedCurve->Play();
			break;
		}

		case(EPlayTypePins::ReverseFromEnd): 
		{
			CreatedCurve->ReverseFromEnd();
			break;
		}
	}

	return CreatedCurve;
}

