// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGameplayPersistence, Log, All)

// Macro for logging in the LogGameplayPersistence category
#define GP_LOG(Verbosity, Format, ...)								\
{																	\
	UE_LOG(LogGameplayPersistence, Verbosity, Format, ##__VA_ARGS__);	\
}
