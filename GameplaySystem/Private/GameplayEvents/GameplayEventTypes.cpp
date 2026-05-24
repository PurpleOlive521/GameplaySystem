// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayEventTypes.h"

DEFINE_LOG_CATEGORY(LogGameplayEvent)

FTickFollowerHandle FTickFollowerHandle::CreateNew()
{
	FTickFollowerHandle Handle = {};
	Handle.GenerateNewHandle();
	return Handle;
}

void FTickFollowerHandle::GenerateNewHandle()
{
	static uint64 NEXT_ID = 1U;

	Id = NEXT_ID++;
}

bool FTickFollowerHandle::IsValid() const
{
	return Id != INVALID_TICK_FOLLOWER_HANDLE_ID;
}

uint32 GetTypeHash(const FTickFollowerHandle& InHandle)
{
	return GetTypeHash(InHandle.Id);
}

FTickFollowerHandle FObjectTickFollowers::AddTickFollower(const FOnLeaderTickSignature& Delegate)
{
	FTickFollowerHandle NewHandle = FTickFollowerHandle::CreateNew();
	FollowerDelegates.Add(NewHandle, Delegate);

	return NewHandle;
}

void FObjectTickFollowers::RemoveTickFollower(const FTickFollowerHandle& Handle)
{
	FollowerDelegates.Remove(Handle);
}

void FObjectTickFollowers::Tick(float DeltaTime)
{
	for (const auto& [Handle, Follower] : FollowerDelegates)
	{
		// We want to assert here if someone forgets to unbind.
		Follower.Execute(DeltaTime);
	}
}

void FObjectTickFollowers::Clear()
{
	FollowerDelegates.Empty();
}
