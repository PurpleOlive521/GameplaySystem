// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"

// --- Collection of helper macros and types

DECLARE_LOG_CATEGORY_EXTERN(LogGameplaySystem, Log, All)

// Macro for logging in the LogGameplaySystem category
#define GS_LOG(Verbosity, Format, ...)								\
{																	\
	UE_LOG(LogGameplaySystem, Verbosity, Format, ##__VA_ARGS__);	\
}

#define PrintToScreenError(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, text)
#define PrintToScreenSuccess(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, text)
#define PrintToScreen(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Silver, text)

#define ENDL TEXT("\n")

// One tab-in length
#define SPACER TEXT("    ")

#define ensureNoEntry() ensureMsgf(false, TEXT("This code path should never be executed."));

// Early return if Super::Signature call fails.
#define FAIL_ON_FAILED_SUPER(Signature)			\
if(!Super::Signature)						\
{											\
	return false;							\
}											\


// A collection of text tags for use in debug strings, wrapped in the appropriate RichText format.
// Define the colors in a RichTagTable to use the colors for more readable debug text.
namespace DebugTypes
{
	// Wraps the tag in the appropriate format for RichText, e.g '<TagName>'
	#define WRAP_TAG(str) TEXT("<" #str ">")

	static const FString TextTag_Default = WRAP_TAG(Default);
	static const FString TextTag_Italic = WRAP_TAG(Italic);
	static const FString TextTag_Bold = WRAP_TAG(Bold);
	static const FString TextTag_Header = WRAP_TAG(Header);
	static const FString TextTag_Accept = WRAP_TAG(Default_Green);
	static const FString TextTag_Warning = WRAP_TAG(Default_Red);
	static const FString TextTag_Highlight = WRAP_TAG(Default_Yellow);
	static const FString TextTag_End = WRAP_TAG(/ );
}