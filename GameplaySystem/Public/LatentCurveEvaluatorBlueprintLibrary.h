// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "LatentCurveEvaluator.h"

#include "LatentCurveEvaluatorBlueprintLibrary.generated.h"

class UGameplayEvent;

UENUM()
enum class EPlayTypePins : uint8
{
	Play,
	ReverseFromEnd,
};

UCLASS()
class GAMEPLAYSYSTEM_API ULatentCurveEvaluatorBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// Returns the created LatentCurveEvaluator.
	// Will use the last key in the CurveFloat if EndTime is left empty.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Owner", DefaultToSelf = "Owner"), Category = "LatentCurveEvaluatorBlueprintLibrary")
	static ULatentCurveEvaluator* CreateLatentCurveEvaluator(UCurveFloat* InCurve, UObject* Owner, const FOnUpdateEvaluationSignature& OnUpdateEvaluationFunction, const FOnFinishedEvaluationSignature& OnFinishedEvaluationFunction,
															float EndTime = 0.0f, bool bEvaluateWhenPaused = false);

	// Immediately starts the LatentCurveEvaluator on creation.
	// Will use the last key in the CurveFloat if EndTime is left empty.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Owner", DefaultToSelf = "Owner", ExpandEnumAsExecs = "PlayType"), Category = "LatentCurveEvaluatorBlueprintLibrary")
	static ULatentCurveEvaluator* CreateAndPlayLatentCurveEvaluator(EPlayTypePins PlayType, UCurveFloat* InCurve, UObject* Owner, const FOnUpdateEvaluationSignature& OnUpdateEvaluationFunction, const FOnFinishedEvaluationSignature& OnFinishedEvaluationFunction,
																	float EndTime = 0.0f, bool bEvaluateWhenPaused = false);
				
};
