// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "Fraction.h"


float FFraction::GetPercentage() const
{
	if (Numerator == 0 || Denominator == 0)
	{
		return 0.0f;
	}

	return ((float)Numerator / (float)Denominator) * 100.0f;
}

float FFraction::ToCoefficient() const
{
	return FMath::Clamp((float)Numerator / (float)Denominator, 0.0f, 1.0f);
}

bool FFraction::HasZero() const
{
	return Numerator == 0 || Denominator == 0;
}

bool FFraction::IsProper() const
{
	return Numerator < Denominator;
}

bool FFraction::IsProperInclusive() const
{
	return Numerator <= Denominator;
}

bool FFraction::IsOne() const
{
	return Numerator == Denominator;
}
