// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayEventHandle.h"

FGameplayEventHandle FGameplayEventHandle::CreateNew()
{
	FGameplayEventHandle Handle = {};
	Handle.GenerateNewHandle();
	return Handle;
}

void FGameplayEventHandle::GenerateNewHandle()
{
	static uint32 NEXT_ID = 1U;

	Id = NEXT_ID++;

	bWasInitialized = true;
}

bool FGameplayEventHandle::IsValid() const
{
	return bWasInitialized && Id != INVALID_EVENT_HANDLE_ID;
}

uint32 GetTypeHash(const FGameplayEventHandle& InHandle)
{
	return GetTypeHash(InHandle.Id);
}
