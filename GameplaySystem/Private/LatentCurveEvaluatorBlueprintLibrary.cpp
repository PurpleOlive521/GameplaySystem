// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "LatentCurveEvaluatorBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"

ULatentCurveEvaluator* ULatentCurveEvaluatorBlueprintLibrary::CreateLatentCurveEvaluator(UObject* Owner, const FLatentCurveEvaluatorParams& Params)
{
	ULatentCurveEvaluator* CurveEvaluator = NewObject<ULatentCurveEvaluator>(Owner, ULatentCurveEvaluator::StaticClass());

	CurveEvaluator->SetProperties(Params);

	return CurveEvaluator;
}

ULatentCurveEvaluator* ULatentCurveEvaluatorBlueprintLibrary::CreateAndPlayLatentCurveEvaluator(EEvaluatorPlayTypePins PlayType, UObject* Owner, const FLatentCurveEvaluatorParams& Params)
{
	ULatentCurveEvaluator* CreatedCurve = CreateLatentCurveEvaluator(Owner, Params);
	ensure(CreatedCurve);

	CreatedCurve->PlayByType(PlayType);

	return CreatedCurve;
}

