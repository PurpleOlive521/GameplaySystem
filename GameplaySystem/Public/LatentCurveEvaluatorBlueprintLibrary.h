// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "LatentCurveEvaluator.h"

#include "LatentCurveEvaluatorBlueprintLibrary.generated.h"

class UGameplayEvent;

UCLASS()
class GAMEPLAYSYSTEM_API ULatentCurveEvaluatorBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// Returns the created LatentCurveEvaluator.
	// Will play until the last key in the CurveFloat if EndTime is left empty.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Owner", DefaultToSelf = "Owner"), Category = "LatentCurveEvaluatorBlueprintLibrary")
	static ULatentCurveEvaluator* CreateLatentCurveEvaluator(UObject* Owner, const FLatentCurveEvaluatorParams& Params);

	// Immediately starts the LatentCurveEvaluator on creation.
	// Will play until the the last key in the CurveFloat if EndTime is left empty.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "Owner", DefaultToSelf = "Owner", ExpandEnumAsExecs = "PlayType"), Category = "LatentCurveEvaluatorBlueprintLibrary")
	static ULatentCurveEvaluator* CreateAndPlayLatentCurveEvaluator(EEvaluatorPlayTypePins PlayType, UObject* Owner, const FLatentCurveEvaluatorParams& Params);
				
};
