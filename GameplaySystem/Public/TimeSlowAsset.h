// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "LatentCurveEvaluator.h"
#include "TimeSlowAsset.generated.h"

class UCurveFloat;

UENUM(blueprintType)
enum class ETimeDilationType : uint8
{
	// Applies to Global TimeDilation
	ETDT_Global		UMETA(DisplayName = "Global"),

	// Applies to activating Actors Custom TimeDilation
	ETDT_Source		UMETA(DisplayName = "Source"),
};

USTRUCT(BlueprintType)
struct FModularCurve
{
	GENERATED_BODY()

	bool IsValidCurve() const;

	// The time we want to evaluate for.
	// The Curve will be stretched or shrunk to match this time, regardless of it's own length.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ModularCurve")
	float Time = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ModularCurve")
	TObjectPtr<UCurveFloat> Curve = nullptr;

	// Mirrors the Curve horizontally, evaluating it from end to beginning.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "ModularCurve")
	bool bMirrorCurve = false;
};

/**

 */
UCLASS()
class GAMEPLAYSYSTEM_API UTimeSlowAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// --- Begin UPrimaryDataAsset Interface
	FPrimaryAssetId GetPrimaryAssetId() const override;
	// --- End UPrimaryDataAsset Interface

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TimeSlowAsset")
	static UCurveFloat* GetEasingCurve(const FModularCurve& ModularCurve);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TimeSlowAsset")
	static bool ContainsValidEasing(const FModularCurve& ModularCurve);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TimeSlowAsset")
	static EEvaluatorPlayTypePins GetEasingPlayType(const FModularCurve& ModularCurve);
	
	// The target TimeDilation we want to achieve.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "TimeSlowAsset")
	float TimeDilation = 0.0f;
	
	// This disables easing out since we won't ever trigger it ourselves. Instead snaps to the expected TimeDilation when ended.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "TimeSlowAsset")
	bool bIsInfiniteDuration = false;

	// Duration we wait between EaseIn and EaseOut. Not the same as the total length of the TimeSlow!
	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (EditCondition = "bIsInfiniteDuration == false"), Category = "TimeSlowAsset")
	float Duration = 0.0f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "TimeSlowAsset|Easing")
	FModularCurve EaseInCurve;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, meta = (EditCondition = "bIsInfiniteDuration == false"), Category = "TimeSlowAsset|Easing")
	FModularCurve EaseOutCurve;
};