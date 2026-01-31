// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#pragma once

#include "CoreMinimal.h"


DECLARE_LOG_CATEGORY_EXTERN(LogGameplaySystemEditor, Log, All)

// Macro for logging in the LogGameplaySystemEditor category
#define GSED_LOG(Verbosity, Format, ...)								\
{																	\
	UE_LOG(LogGameplaySystemEditor, Verbosity, Format, ##__VA_ARGS__);	\
}