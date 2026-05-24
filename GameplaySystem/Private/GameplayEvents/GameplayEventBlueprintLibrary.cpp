// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayEventBlueprintLibrary.h"
#include "GameplayEventSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "DevelopmentTypes.h"

FGameplayEventHandle UGameplayEventBlueprintLibrary::TriggerEvent(TSubclassOf<UGameplayEvent> EventClass, UObject* Owner)
{
	UGameplayEventSubsystem* EventSubsystem = GetGameplayEventSubsystem(Owner);
    return EventSubsystem->TriggerEvent(EventClass, Owner);
}

FGameplayEventHandle UGameplayEventBlueprintLibrary::TriggerEvent_ActivationData(TSubclassOf<UGameplayEvent> EventClass, UObject* Owner, const FGameplayEventActivationData& ActivationData)
{
	UGameplayEventSubsystem* EventSubsystem = GetGameplayEventSubsystem(Owner);
	return EventSubsystem->TriggerEvent_ActivationData(EventClass, Owner, ActivationData);
}

UGameplayEvent* UGameplayEventBlueprintLibrary::GetEventFromHandle(const UObject* WorldObject, const FGameplayEventHandle& Handle)
{
	UGameplayEventSubsystem* EventSubsystem = GetGameplayEventSubsystem(WorldObject);
	return EventSubsystem->GetEventFromHandle(Handle);
}

void UGameplayEventBlueprintLibrary::GetEventsFromHandles(const UObject* WorldObject, const TArray<FGameplayEventHandle>& Handles, TArray<UGameplayEvent*> OutEvents)
{
	UGameplayEventSubsystem* EventSubsystem = GetGameplayEventSubsystem(WorldObject);
	EventSubsystem->GetEventsFromHandles(Handles, OutEvents);
}

bool UGameplayEventBlueprintLibrary::AbortEvent(const UObject* WorldObject, const FGameplayEventHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return false;
	}

	UGameplayEventSubsystem* EventSubsystem = GetGameplayEventSubsystem(WorldObject);

	return EventSubsystem->EndEventByHandle(Handle);
}

bool UGameplayEventBlueprintLibrary::IsHandleValid(const FGameplayEventHandle& Handle)
{
	return Handle.IsValid();
}

bool UGameplayEventBlueprintLibrary::IsEventActive(const FGameplayEventHandle& Handle, UObject* WorldObject)
{
	UGameplayEventSubsystem* EventSubsystem = GetGameplayEventSubsystem(WorldObject);
	UGameplayEvent* Event = EventSubsystem->GetEventFromHandle(Handle);

	if (Event)
	{
		return Event->IsActive();
	}

	return false;
}

FString UGameplayEventBlueprintLibrary::ConvertTickSourceToDisplayName(ETickSource TickSourceType)
{
	switch (TickSourceType)
	{
	case ETickSource::ETS_SourceDeltaTime:
		return TEXT("Source DT");
	case ETickSource::ETS_GlobalDeltaTime:
		return TEXT("Global DT");
	case ETickSource::ETS_AbsoluteDeltaTime:
		return TEXT("Absolute DT");
	default:
		ensureNoEntry(); // There should always be a corresponding type
		return FString();
	}
}

APlayerController* UGameplayEventBlueprintLibrary::GetPlayerControllerForEvent(const UObject* WorldContextObject, int32 PlayerIndex)
{
	return UGameplayStatics::GetPlayerController(WorldContextObject, PlayerIndex);
}

UGameInstance* UGameplayEventBlueprintLibrary::GetGameInstanceForEvent(const UObject* WorldContextObject)
{
	return UGameplayStatics::GetGameInstance(WorldContextObject);
}

UNiagaraComponent* UGameplayEventBlueprintLibrary::SpawnSystemAtLocation(const UObject* WorldContextObject, class UNiagaraSystem* SystemTemplate, FVector Location, FRotator Rotation, FVector Scale, bool bAutoDestroy, bool bAutoActivate, ENCPoolMethod PoolingMethod, bool bPreCullCheck)
{
	if (WorldContextObject)
	{
		if (UWorld* World = WorldContextObject->GetWorld()) 
		{
			return UNiagaraFunctionLibrary::SpawnSystemAtLocation(WorldContextObject, SystemTemplate, Location, Rotation, Scale, bAutoDestroy, bAutoActivate, PoolingMethod, bPreCullCheck);
		}
	}

	return nullptr;
}

inline UGameplayEventSubsystem* UGameplayEventBlueprintLibrary::GetGameplayEventSubsystem(const UObject* WorldContextObject)
{
	return UGameplayEventSubsystem::Get(WorldContextObject);
}

