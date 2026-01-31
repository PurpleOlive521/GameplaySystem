// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


#include "GameplayEvent.h"
#include "GameplayEventSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "DevelopmentTypes.h"
#include "GameplayEventBlueprintLibrary.h"

using namespace DebugTypes;

UGameplayEvent::UGameplayEvent()
{
}

#if WITH_EDITOR
void UGameplayEvent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	CleanProperties();
}
#endif // WITH_EDITOR

UWorld* UGameplayEvent::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject) == false)
	{
		// Get World from Outer
		return GetOuter()->GetWorld();
	}

	// Return nullptr if we are CDO.
	return nullptr;
}

void UGameplayEvent::FinishDestroy()
{
	Super::FinishDestroy();
}

void UGameplayEvent::Init(UObject* InOwningObject)
{
	check(InOwningObject);

	OwningObject = InOwningObject;

	if(InOwningObject->IsA<AActor>())
	{
		OwningActor = Cast<AActor>(InOwningObject);
	}
}

void UGameplayEvent::Tick(float DeltaTime)
{
	if (!IsActive())
	{
		return;
	}

	if (bShareOwnerLifetime && !OwningObject.IsValid())
	{
		TryAbortEvent();
		return;
	}

	Lifetime += DeltaTime;
	TickFollowers.Tick(DeltaTime);

	if (DurationType == EEventDurationType::EEDT_HasDuration)
	{
		if (Lifetime >= Duration)
		{
			TryEndGameplayEvent();
		}
	}
}

bool UGameplayEvent::TryTriggerGameplayEvent(const FGameplayEventActivationData& ActivationData)
{
	if (InstancingPolicy == EEventInstancingPolicy::EEIP_Static)
	{
		GE_LOG(Warning, TEXT("Tried to trigger a static GameplayEvent as instanced: %s"), *DisplayName);
		return false;
	}

	if (bHasTriggered)
	{
		return false;
	}

	bHasTriggered = true;
	
	const bool bPassedPreTrigger = PreTriggerEvent();
	if (!bPassedPreTrigger)
	{
		GE_LOG(Log, TEXT("Pretrigger for event '%s' failed."), *DisplayName);

		bMarkedForCleanup = true;

		return false;
	}

	GE_LOG(Log, TEXT("Event: %s triggered. Mode: Instanced"), *DisplayName);

	TriggerEvent(ActivationData);

	K2_TriggerEvent(ActivationData);

	return true;
}

void UGameplayEvent::StaticTryTriggerGameplayEvent(UWorld* World, const FGameplayEventActivationData& ActivationData) const
{
	ensure(World);

	GE_LOG(Log, TEXT("Event: %s triggered. Mode: Static"), *DisplayName);

	const bool bPassedPreTrigger = StaticPreTriggerEvent();
	if (!bPassedPreTrigger)
	{
		GE_LOG(Log, TEXT("StaticPretrigger for event '%s' failed."), *DisplayName);

		return;
	}

	StaticTriggerEvent(World, ActivationData);

	K2_StaticTriggerEvent(World, ActivationData);
}

bool UGameplayEvent::TryEndGameplayEvent()
{
	if (bHasEnded)
	{
		return false;
	}

	bHasEnded = true;

	EndEvent();

	K2_EndEvent();

	MarkForCleanup();

	return true;
}

bool UGameplayEvent::TryAbortEvent()
{
	if (bHasAborted)
	{
		return false;
	}

	bHasAborted = true;

	bIsAborting = true;

	AbortEvent();

	K2_AbortEvent();

	bIsAborting = false;

	MarkForCleanup();

	return true;
}

UObject* UGameplayEvent::GetOwningObject() const
{
	return OwningObject.Get();
}

UObject* UGameplayEvent::GetOwningObject_Checked() const
{
	UObject* Out = OwningObject.Get();

	check(Out);

	return Out;
}

bool UGameplayEvent::HasOwningActor() const
{
	return OwningActor.IsValid();
}

AActor* UGameplayEvent::GetOwnerAsActor() const
{
	return OwningActor.Get();
}

AActor* UGameplayEvent::GetOwnerAsActor_Checked() const
{
	AActor* Out = OwningActor.Get();

	check(Out);

	return Out;
}

UGameplayEventSubsystem* UGameplayEvent::GetEventSubsystem() const
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(this);

	if (!GameInstance)
	{
		GE_LOG(Error, TEXT("Event could not access GameInstance! Was it called outside of the valid game scope?"));
		return nullptr;
	}

	UGameplayEventSubsystem* EventSubsystem = GameInstance->GetSubsystem<UGameplayEventSubsystem>();
	ensure(EventSubsystem); // Should not be invalid during the expected GameplayEvent lifetime. Are we operating invalidly?

	return EventSubsystem;
}

bool UGameplayEvent::ShouldCleanup() const
{
	if (bMarkedForCleanup)
	{
		return true;
	}

	if (!OwningObject.IsValid())
	{
		return true;
	}

	return false;
}

bool UGameplayEvent::IsActive() const
{
	if (!bHasTriggered)
	{
		return false;
	}

	if (bHasEnded || bHasAborted || bMarkedForCleanup || bIsAborting)
	{
		return false;
	}

	return true;
}

bool UGameplayEvent::ApplyBlockingQuery()
{
	if (bAppliedBlockQuery)
	{
		return false;
	}

	UGameplayEventSubsystem* EventSubsystem = GetEventSubsystem();

	if (BlockQueryPolicy == EQueryPolicy::EQP_Global)
	{
		EventSubsystem->GloballyBlockedEventTags.AppendTags(BlockEventsWithTag);
	}
	else if (BlockQueryPolicy == EQueryPolicy::EQP_PerActor)
	{
		if (!GetOwnerAsActor())
		{
			GE_LOG(Error, TEXT("Could not access Owner as Actor on GameplayEvent: %s! Do not use Global policies on GameplayEvents that trigger on non-actor objects."), *DisplayName);
			return false;
		}

		FActorGameplayEventContainer& EventContainer = EventSubsystem->PerActorEventMap.FindOrAdd(OwningActor);
		EventContainer.BlockedEventTags.AppendTags(BlockEventsWithTag);
	}

	bAppliedBlockQuery = true;
	return true;
}

bool UGameplayEvent::RemoveBlockingQuery()
{
	if (!bAppliedBlockQuery)
	{
		return false;
	}

	UGameplayEventSubsystem* EventSubsystem = GetEventSubsystem();

	if (BlockQueryPolicy == EQueryPolicy::EQP_Global)
	{
		EventSubsystem->GloballyBlockedEventTags.RemoveTags(BlockEventsWithTag);
	}
	else if (BlockQueryPolicy == EQueryPolicy::EQP_PerActor)
	{
		if (!GetOwnerAsActor())
		{
			GE_LOG(Error, TEXT("Could not access Owner as Actor on GameplayEvent: %s! Do not use Global policies on GameplayEvents that trigger on non-actor objects."), *DisplayName);
			return false;
		}

		FActorGameplayEventContainer* EventContainer = EventSubsystem->PerActorEventMap.Find(OwningActor);

		if(!EventContainer)
		{
			GE_LOG(Error, TEXT("Could not find Actor Event Container when removing blocking query on GameplayEvent: %s"), *DisplayName);
			return false;
		}

		EventContainer->BlockedEventTags.RemoveTags(BlockEventsWithTag);
	}

	bAppliedBlockQuery = false;
	return true;
}

FString UGameplayEvent::ToString() const
{
	FString DisplayInfo = TEXT("Event: ") + DisplayName + ENDL;
	DisplayInfo += UGameplayEventBlueprintLibrary::ConvertTickSourceToDisplayName(TickSource);
	DisplayInfo += FString::Printf(TEXT(" Duration: %.2f, Lifetime : %.2f"), Duration, Lifetime);

	if (UObject* Owner = GetOwningObject())
	{
		DisplayInfo += TEXT(" Owner: ") + Owner->GetClass()->GetName();
	}

	return DisplayInfo;
}

FString UGameplayEvent::ToStringWithDebugTags() const
{
	FString DisplayInfo = TEXT("Event: ") + TextTag_Highlight + DisplayName + TextTag_End + ENDL;
	DisplayInfo += UGameplayEventBlueprintLibrary::ConvertTickSourceToDisplayName(TickSource);
	DisplayInfo += FString::Printf(TEXT(" Duration: %.2f, Lifetime : %.2f"), Duration, Lifetime);

	if (UObject* Owner = GetOwningObject())
	{
		DisplayInfo += TEXT(" Owner: ") + TextTag_Highlight + Owner->GetClass()->GetName() + TextTag_End;
	}

	return DisplayInfo;
}

bool UGameplayEvent::PreTriggerEvent()
{
	UGameplayEventSubsystem* EventSubsystem = GetEventSubsystem();

	// --- Enforce End Queries

	if (EndQueryPolicy == EQueryPolicy::EQP_Global)
	{
		TArray<FGameplayEventHandle> HandlesToEnd;
		EventSubsystem->GetEventsByGameplayTagQuery(EndMatchingEvents, HandlesToEnd, this);

		EventSubsystem->EndEventsByHandles(HandlesToEnd);
	}
	else if (EndQueryPolicy == EQueryPolicy::EQP_PerActor)
	{
		AActor* Owner = GetOwnerAsActor();
		if (!Owner)
		{
			GE_LOG(Error, TEXT("Could not access Owner as Actor on GameplayEvent: %s! Do not use Global policies on GameplayEvents that trigger on non-actor objects."), *DisplayName);
			return false;
		}

		TArray<FGameplayEventHandle> HandlesToEnd;
		EventSubsystem->GetEventsByGameplayTagQueryFromActor(EndMatchingEvents, HandlesToEnd, Owner, this);

		EventSubsystem->EndEventsByHandles(HandlesToEnd);
	}

	// --- Enforce Uniqueness

	if (bGloballyUnique)
	{		
		TArray<FGameplayEventHandle> OutEvents;
		EventSubsystem->GetEventsByType(GetClass(), OutEvents, this);

		if (OutEvents.Num() > 0)
		{
			if (bReplaceOnUnique)
			{
				EventSubsystem->EndEventsByHandles(OutEvents);
			}
			else
			{
				return false;
			}
		}
	}

	if (bActorUnique)
	{
		AActor* Owner = GetOwnerAsActor();
		if (!Owner)
		{
			GE_LOG(Fatal, TEXT("Could not access Owner as Actor in PreTrigger! Do not use bActorUnique on GameplayEvents that trigger on non-actor objects."));
			return false;
		}

		TArray<FGameplayEventHandle> OutEvents;
		EventSubsystem->GetEventsByTypeFromActor(GetClass(), OutEvents, Owner, this);

		if (OutEvents.Num() > 0)
		{
			if (bReplaceOnUnique)
			{
				EventSubsystem->EndEventsByHandles(OutEvents);
			}
			else
			{
				return false;
			}
		}
	}

	return true;
}

bool UGameplayEvent::StaticPreTriggerEvent() const
{
	UGameplayEventSubsystem* EventSubsystem = GetEventSubsystem();

	// --- Enforce End Queries

	if (EndQueryPolicy == EQueryPolicy::EQP_Global)
	{
		TArray<FGameplayEventHandle> HandlesToEnd;
		EventSubsystem->GetEventsByGameplayTagQuery(EndMatchingEvents, HandlesToEnd, this);

		EventSubsystem->EndEventsByHandles(HandlesToEnd);
	}
	else if (EndQueryPolicy == EQueryPolicy::EQP_PerActor)
	{
		AActor* Owner = GetOwnerAsActor();
		if (Owner)
		{
			GE_LOG(Error, TEXT("Could not access Owner as Actor on GameplayEvent: %s! Do not use Global policies on GameplayEvents that trigger on non-actor objects."), *DisplayName);
			return false;
		}

		TArray<FGameplayEventHandle> HandlesToEnd;
		EventSubsystem->GetEventsByGameplayTagQueryFromActor(EndMatchingEvents, HandlesToEnd, Owner, this);

		EventSubsystem->EndEventsByHandles(HandlesToEnd);
	}

	return true;
}

void UGameplayEvent::TriggerEvent(const FGameplayEventActivationData& ActivationData)
{
	// Write your custom native trigger logic here.
}

void UGameplayEvent::StaticTriggerEvent(UWorld* World, const FGameplayEventActivationData& ActivationData) const
{
	// Write your custom native trigger logic here.
	// Only required when EventInstancingPolicy is set to Static.
}

void UGameplayEvent::EndEvent()
{
	// Write your custom native end logic here.
}

void UGameplayEvent::AbortEvent()
{
	// Write your custom native abort logic here.
}

void UGameplayEvent::FinishEvent(bool bTriggerEndCallback)
{
	if (bTriggerEndCallback)
	{
		TryEndGameplayEvent();
	}
}

void UGameplayEvent::FinishAbortWithEndEvent()
{
	if (!bHasAborted)
	{
		GE_LOG(Warning, TEXT("FinishAbortWithEndEvent called when not aborted!"));
		return;
	}

	if (!bIsAborting)
	{
		GE_LOG(Warning, TEXT("FinishAbortWithEndEvent called when not aborting!"));
		return;
	}
	
	TryEndGameplayEvent();
}

void UGameplayEvent::ForceTerminate()
{
	if (!bHasAborted)
	{
		const FString OwnerName = GetOwningObject() ? GetOwningObject()->GetClass()->GetName() : "NO OWNER";
		GE_LOG(Error, TEXT("Event forced to terminate to avoid raising errors in non-crucial GameplayEvent: '%s' by '%s'."), *DisplayName, *OwnerName);

		TryAbortEvent();
		
		MarkForCleanup();
		// We want to get GC'ed ASAP, since we might have created objects in Blueprints that we want removed.
		GetEventSubsystem()->RequestCleanup();
	}
}

void UGameplayEvent::StaticForceTerminate() const
{
	const FString OwnerName = GetOwningObject() ? GetOwningObject()->GetClass()->GetName() : "NO OWNER";
	GE_LOG(Error, TEXT("Static Event forced to terminate to avoid raising errors in non-crucial GameplayEvent: '%s' by '%s'."), *DisplayName, *OwnerName);
}

void UGameplayEvent::MarkForCleanup()
{
	TickFollowers.Clear();
	bMarkedForCleanup = true;
}

void UGameplayEvent::CleanProperties()
{
	if (bGloballyUnique)
	{
		// Redundant flag
		bActorUnique = false;
	}
}
