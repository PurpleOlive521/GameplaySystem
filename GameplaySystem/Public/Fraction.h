// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Fraction.generated.h"

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FFraction
{
	GENERATED_BODY()

	FFraction() = default;

	FFraction(int InNumerator, int InDenominator) : Numerator(InNumerator), Denominator(InDenominator) {};

	// Returns an approximate percentage represenation of the fraction in the form of 50.0f = 50%.
	float GetPercentage() const;

	float ToCoefficient() const;

	// A zero in either Numerator or Denominator will result in a value of 0.
	bool HasZero() const;

	// Fractions are proper when Numerator < Denominator where both are positive.
	bool IsProper() const;

	// Like IsProper, but an identical Numerator and Denominator that are above 0 is allowed.
	bool IsProperInclusive() const;

	// Numerator and denominator are equal.
	bool IsOne() const;

	// The upper component of the fraction, e.g. '1' / 2
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	int Numerator = 1;

	// The lower component of the fraction, e.g. 1 / '2'
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	int Denominator = 1;
};
