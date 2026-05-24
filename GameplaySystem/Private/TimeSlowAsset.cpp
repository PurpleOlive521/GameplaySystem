// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "TimeSlowAsset.h"

FPrimaryAssetId UTimeSlowAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("TimeSlowAssetItems", GetFName());
}

UCurveFloat* UTimeSlowAsset::GetEasingCurve(const FModularCurve& ModularCurve)
{
	return ModularCurve.Curve;
}

bool UTimeSlowAsset::ContainsValidEasing(const FModularCurve& ModularCurve)
{
	return ModularCurve.IsValidCurve();
}

EEvaluatorPlayTypePins UTimeSlowAsset::GetEasingPlayType(const FModularCurve& ModularCurve)
{
	if (ModularCurve.bMirrorCurve)
	{
		return EEvaluatorPlayTypePins::ReverseFromEnd;
	}
	else
	{
		return EEvaluatorPlayTypePins::PlayFromStart;
	}
}

bool FModularCurve::IsValidCurve() const
{
	return Curve && Time > 0.0f;
}
