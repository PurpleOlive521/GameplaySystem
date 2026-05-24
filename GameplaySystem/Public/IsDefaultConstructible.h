// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreTypes.h"

template <typename T>
struct TIsDefaultConstructible
{
	enum { Value = __is_constructible(T) };
};

class TIsDefaultConstructible_NonConstructible
{
	TIsDefaultConstructible_NonConstructible() = delete;
};

static_assert(TIsDefaultConstructible<bool>::Value, "GameplaySystemModule Platform TIsDefaultConstructible test failed.");
static_assert(TIsDefaultConstructible<TIsDefaultConstructible_NonConstructible>::Value == false, "GameplaySystemModule Platform TIsDefaultConstructible test failed.");

