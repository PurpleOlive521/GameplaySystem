// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "LatentCurveEvaluator.h"

#include "LatentCurveEvaluatorBlueprintLibrary.generated.h"




UCLASS()
class GAMEPLAYSYSTEM_API ULatentCurveEvaluatorBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// Returns the created LatentCurveEvaluator, allowing full control over start & end time.
	// Will use the last key in the CurveFloat if EndTime is left empty.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Owner", DefaultToSelf = "Owner"), Category = "LatentCurveEvaluatorBlueprintLibrary")
	static ULatentCurveEvaluator* CreateLatentCurveEvaluator(UCurveFloat* InCurve, UObject* Owner, const FOnUpdateEvaluationSignature& OnUpdateEvaluationFunction, const FOnFinishedEvaluationSignature& OnFinishedEvaluationFunction,
															float EndTime = 0.0f, bool bEvaluateWhenPaused = false);

	// Immediately starts the LatentCurveEvaluator on creation.
	// Will use the last key in the CurveFloat if EndTime is left empty.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Owner", DefaultToSelf = "Owner"), Category = "LatentCurveEvaluatorBlueprintLibrary")
	static ULatentCurveEvaluator* CreateAndPlayLatentCurveEvaluator(UCurveFloat* InCurve, UObject* Owner, const FOnUpdateEvaluationSignature& OnUpdateEvaluationFunction, const FOnFinishedEvaluationSignature& OnFinishedEvaluationFunction,
																	float EndTime = 0.0f, bool bEvaluateWhenPaused = false);
				
};
