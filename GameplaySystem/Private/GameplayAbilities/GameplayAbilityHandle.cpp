// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayAbilityHandle.h"

FGameplayAbilityHandle FGameplayAbilityHandle::CreateNew()
{
	FGameplayAbilityHandle Handle = {};
	Handle.GenerateNewHandle();
	return Handle;
}

void FGameplayAbilityHandle::GenerateNewHandle()
{
	static uint64 NEXT_ID = 1U;

	Id = NEXT_ID++;
}

bool FGameplayAbilityHandle::IsValid() const
{
	return Id != INVALID_ABILITY_HANDLE_ID;
}

uint32 GetTypeHash(const FGameplayAbilityHandle& InHandle)
{
	return GetTypeHash(InHandle.Id);
}
