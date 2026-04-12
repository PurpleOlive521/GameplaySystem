// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayEventSubsystem.h"

#include "Engine/GameInstance.h"
#include "GameplayEvent.h"
#include "GameplayEventBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTasksComponent.h"
#include "DevelopmentTypes.h"

void UGameplayEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    FCoreUObjectDelegates::PreGarbageCollectConditionalBeginDestroy.AddUObject(this, &UGameplayEventSubsystem::HandlePreGarbageCollect);
}

void UGameplayEventSubsystem::Deinitialize()
{
    FCoreUObjectDelegates::PreGarbageCollectConditionalBeginDestroy.RemoveAll(this);
    
    Super::Deinitialize();
}

void UGameplayEventSubsystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    const float GlobalTimeDilation = GetWorld()->GetWorldSettings()->GetEffectiveTimeDilation();

    const float AbsoluteDeltaTime = DeltaTime / GlobalTimeDilation;
    const float GlobalDeltaTime = DeltaTime;

    FGameplayEventMapLock ActiveLock(*this);

    for (auto& [Handle, Event] : EventMap)
    {
        TickEvent(Event, DeltaTime, AbsoluteDeltaTime, GlobalDeltaTime);
    }

    for (auto QueuedEvent : EventQueue)
    {
        TickEvent(QueuedEvent.Event, DeltaTime, AbsoluteDeltaTime, GlobalDeltaTime);
    }
}

ETickableTickType UGameplayEventSubsystem::GetTickableTickType() const
{
    return Super::GetTickableTickType();
}

bool UGameplayEventSubsystem::IsTickable() const
{
    FAIL_ON_FAILED_SUPER(IsTickable());

    return true;
}

TStatId UGameplayEventSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UGameplayEventSubsystem, STATGROUP_Tickables);
}

UWorld* UGameplayEventSubsystem::GetTickableGameObjectWorld() const
{
    return GetWorld();
}

bool UGameplayEventSubsystem::IsTickableWhenPaused() const
{
    // We allow some GameplayEvents to tick when paused
    return true;
}

UGameplayEventSubsystem* UGameplayEventSubsystem::Get(const UObject* WorldContext)
{
    if (WorldContext)
    {
        const UWorld* World = WorldContext->GetWorld();
        if (World)
        {
            UGameplayEventSubsystem* EventSubsystem = World->GetSubsystem<UGameplayEventSubsystem>();
            ensure(EventSubsystem);
            
            return EventSubsystem;
        }
    }

    return nullptr;
}

FGameplayEventHandle UGameplayEventSubsystem::TriggerEvent(TSubclassOf<UGameplayEvent> EventClass, UObject* Owner)
{
    FGameplayEventActivationData ActivationData = {};
	return TriggerEvent_Internal(EventClass, Owner, ActivationData);
}

FGameplayEventHandle UGameplayEventSubsystem::TriggerEvent_ActivationData(TSubclassOf<UGameplayEvent> EventClass, UObject* Owner, const FGameplayEventActivationData& ActivationData)
{
    return TriggerEvent_Internal(EventClass, Owner, ActivationData);
}

UGameplayEvent* UGameplayEventSubsystem::GetEventFromHandle(const FGameplayEventHandle& Handle)
{
    UGameplayEvent* Event = EventMap.FindRef(Handle);
    if (Event)
    {
        return EventMap.FindRef(Handle);
    }

    FQueuedEvent* QueuedEventPtr = EventQueue.FindByPredicate([Handle](const FQueuedEvent& Item)
        {
            return Item.Handle == Handle;
        });

    if (QueuedEventPtr)
    {
        return QueuedEventPtr->Event;
    }

    return nullptr;
}

void UGameplayEventSubsystem::GetEventsFromHandles(const TArray<FGameplayEventHandle>& Handles, TArray<UGameplayEvent*> OutEvents)
{
    OutEvents.Empty();

    for (const FGameplayEventHandle& Handle : Handles)
    {
        OutEvents.Emplace(GetEventFromHandle(Handle));
    }
}

inline static bool Predicate_ByType(TSubclassOf<UGameplayEvent> EventClass, const UGameplayEvent* Event)
{
    return Event->GetClass() == EventClass;
}

void UGameplayEventSubsystem::GetEventsByType(TSubclassOf<UGameplayEvent> EventClass, TArray<FGameplayEventHandle>& OutEvents, const UGameplayEvent* Ignore)
{
    ensure(EventClass);

    GetEventsByPredicate(std::bind(&Predicate_ByType, EventClass, std::placeholders::_1), NO_ACTOR, Ignore, OutEvents);
}

void UGameplayEventSubsystem::GetEventsByTypeFromActor(TSubclassOf<UGameplayEvent> EventClass, TArray<FGameplayEventHandle>& OutEvents, AActor* Actor, const UGameplayEvent* Ignore)
{
    ensure(EventClass);

    GetEventsByPredicate(std::bind(&Predicate_ByType, EventClass, std::placeholders::_1), Actor, Ignore, OutEvents);
}

inline static bool Predicate_ByGameplayTagQuery(const FGameplayTagQuery& Query, const UGameplayEvent* Event)
{
    return Query.Matches(Event->EventTags);
}

void UGameplayEventSubsystem::GetEventsByGameplayTagQuery(const FGameplayTagQuery& Query, TArray<FGameplayEventHandle>& OutEvents, const UGameplayEvent* Ignore)
{
    ensure(!Query.IsEmpty());

    GetEventsByPredicate(std::bind(&Predicate_ByGameplayTagQuery, Query, std::placeholders::_1), NO_ACTOR, Ignore, OutEvents);
}

void UGameplayEventSubsystem::GetEventsByGameplayTagQueryFromActor(const FGameplayTagQuery& Query, TArray<FGameplayEventHandle>& OutEvents, AActor* Actor, const UGameplayEvent* Ignore)
{
    ensure(!Query.IsEmpty());

    GetEventsByPredicate(std::bind(&Predicate_ByGameplayTagQuery, Query, std::placeholders::_1), Actor, Ignore, OutEvents);
}

bool UGameplayEventSubsystem::EndEventByHandle(const FGameplayEventHandle& Handle)
{
    TObjectPtr<UGameplayEvent> Event = GetEventFromHandle(Handle);
    if (!Event)
    {
        return false;
    }

    const bool bEndedSuccessfully = Event->TryAbortEvent();

    return bEndedSuccessfully;
}

bool UGameplayEventSubsystem::EndEventsByHandles(const TArray<FGameplayEventHandle>& Handles)
{
    bool bEndedSuccessfully = true;

    for (const FGameplayEventHandle& Handle : Handles)
    {
        bEndedSuccessfully |= EndEventByHandle(Handle);
    }

    return bEndedSuccessfully;
}

inline static bool FilterByPredicate(std::function<bool(const UGameplayEvent*)> Predicate, const UGameplayEvent* Ignore, TObjectPtr<UGameplayEvent> Event)
{
    // Not yet removed internally, continue
    if (!Event)
    {
        return false;
    }

    // Not active
    if (!Event->IsActive())
    {
        return false;
    }

    // Ignored
    if (Event == Ignore)
    {
        return false;
    }

    return Predicate(Event);
}

FGameplayEventHandle UGameplayEventSubsystem::TriggerEvent_Internal(TSubclassOf<UGameplayEvent> EventClass, UObject* Owner, const FGameplayEventActivationData& ActivationData)
{
    check(EventClass);

    ensure(Owner);

    const UGameplayEvent* EventCDO = EventClass->GetDefaultObject<UGameplayEvent>();

    const bool CanActivate = CanTrigger(EventCDO, Owner);

    if (!CanActivate)
    {
        return FGameplayEventHandle();
    }

    // No state is required, and we do not store the GameplayEvent for later callbacks.
    if (EventCDO->InstancingPolicy == EEventInstancingPolicy::EEIP_Static)
    {
        EventCDO->StaticTryTriggerGameplayEvent(Owner, ActivationData);
        return FGameplayEventHandle();
    }

    TObjectPtr<UGameplayEvent> Event = NewObject<UGameplayEvent>(this, EventClass);
    Event->Init(Owner);

    FGameplayEventHandle EventHandle = FGameplayEventHandle::CreateNew();

    if (Event->HasOwningActor())
    {
        TWeakObjectPtr<AActor> EventActor = MakeWeakObjectPtr(Event->GetOwnerAsActor_Checked());
        FActorGameplayEventContainer& EventContainer = PerActorEventMap.FindOrAdd(EventActor);

        EventContainer.GameplayEvents.Add(EventHandle);
    }

    if (IsLockActive())
    {
        EnqueueEvent(FQueuedEvent(Event, EventHandle));
    }
    else
    {
        EventMap.Add(EventHandle, Event);
    }

    Event->TryTriggerGameplayEvent(ActivationData);

    return EventHandle;
}

void UGameplayEventSubsystem::TickEvent(UGameplayEvent* Event, float DeltaTime, float AbsoluteDeltaTime, float GlobalDeltaTime)
{
    switch (Event->TickSource)
    {
    case(ETickSource::ETS_GlobalDeltaTime):
    {
        Event->Tick(GlobalDeltaTime);
        break;
    }

    case(ETickSource::ETS_SourceDeltaTime):
    {
        AActor* Actor = Event->GetOwnerAsActor();

        if (!Actor)
        {
            GE_LOG(Warning, TEXT("Custom delta time source set, but no Actor can be retrieved for the GameplayEvent!"));
            Event->Tick(GlobalDeltaTime);
            break;
        }

        const float CustomDeltaTime = DeltaTime * Actor->CustomTimeDilation;
        Event->Tick(CustomDeltaTime);
        break;
    }

    case(ETickSource::ETS_AbsoluteDeltaTime):
    {
        Event->Tick(AbsoluteDeltaTime);
        break;
    }

    default:
        checkNoEntry(); // Enum not supported yet.
    }
}

bool UGameplayEventSubsystem::CanTrigger(const UGameplayEvent* EventCDO, UObject* Owner)
{
    FGameplayEventMapLock ActiveLock(*this);

    if (bBlockAllEvents)
    {
        return false;
    }

    AActor* OwningActor = Cast<AActor>(Owner);

    // Emulate state for when cancellation is requested during TriggerEvent

    TArray<FGameplayEventHandle> HandlesToEmulate;

    if (EventCDO->EndQueryPolicy == EQueryPolicy::EQP_Global)
    {
        GetEventsByGameplayTagQuery(EventCDO->EndMatchingEvents, HandlesToEmulate);
    }
    else if (EventCDO->EndQueryPolicy == EQueryPolicy::EQP_PerActor)
    {
        if (!OwningActor)
        {
            GE_LOG(Warning, TEXT("Could not access Owner as Actor on GameplayEvent: %s! Do not use Global policies on GameplayEvents that trigger on non-actor objects."), *EventCDO->DisplayName);
            return false;
        }
        GetEventsByGameplayTagQueryFromActor(EventCDO->EndMatchingEvents, HandlesToEmulate, OwningActor);
    }

	TArray<UGameplayEvent*> EventsToEmulate;
	GetEventsFromHandles(HandlesToEmulate, EventsToEmulate);

    for(UGameplayEvent* EventToEmulate : EventsToEmulate)
    {
        if (!EventToEmulate)
        {
            continue;
        }

        EventToEmulate->ApplyBlockingQuery();
	}

    bool bCanTrigger = true;

    if (EventCDO->EventTags.HasAny(GloballyBlockedEventTags.GetGameplayTagContainer()))
    {
        bCanTrigger = false;
	}

    if (bCanTrigger)
    {
        FActorGameplayEventContainer* ActorEventContainer = PerActorEventMap.Find(OwningActor);
        if (ActorEventContainer)
        {
            if (EventCDO->EventTags.HasAny(ActorEventContainer->BlockedEventTags.GetGameplayTagContainer()))
            {
                bCanTrigger = false;
            }
        }
    }

    for (UGameplayEvent* EventToEmulate : EventsToEmulate)
    {
        if (!EventToEmulate)
        {
            continue;
        }

        EventToEmulate->RemoveBlockingQuery();
    }

    return bCanTrigger;
}

void UGameplayEventSubsystem::GetEventsByPredicate(std::function<bool(const UGameplayEvent*)> Predicate, TWeakObjectPtr<AActor> Target, const UGameplayEvent* Ignore, TArray<FGameplayEventHandle>& OutEvents)
{
    if (Target != nullptr)
    {
        FActorGameplayEventContainer* EventContainer = PerActorEventMap.Find(Target);

        // No events owned by this Actor are found.
        if (!EventContainer)
        {
            return;
        }

        for (const FGameplayEventHandle& Handle : EventContainer->GameplayEvents)
        {
            UGameplayEvent* Event = GetEventFromHandle(Handle);
            if (!Event)
            {
                checkNoEntry(); // We are storing a per-actor handle that should have been removed.
                return;
            }

            if (FilterByPredicate(Predicate, Ignore, Event))
            {
                OutEvents.Add(Handle);
            }
        }

        return;
    }

    for (auto& [Handle, Event] : EventMap)
    {
        if (FilterByPredicate(Predicate, Ignore, Event))
        {
            OutEvents.Add(Handle);
        }
    }

    for (auto QueuedEvent : EventQueue)
    {
        if (FilterByPredicate(Predicate, Ignore, QueuedEvent.Event))
        {
            OutEvents.Add(QueuedEvent.Handle);
        }
    }
}

void UGameplayEventSubsystem::CleanupGameplayEvents()
{
    FGameplayEventMapLock ActiveLock(*this);

    TArray<FGameplayEventHandle> HandlesToRemove;

    for (const auto& [Handle, Event] : EventMap)
    {
        ensure(Event);

        if (Event->ShouldCleanup())
        {
            HandlesToRemove.Emplace(Handle);

            // --- Remove handle from per-actor map

            AActor* Owner = Event->GetOwnerAsActor();

            if (!Owner)
            {
                continue;
            }

            FActorGameplayEventContainer* EventContainer = PerActorEventMap.Find(Owner);
            
            if (!EventContainer)
            {
                continue;
            }

            EventContainer->GameplayEvents.Remove(Handle);
        }
    }

    for (const FGameplayEventHandle& Handle : HandlesToRemove)
    {
        EventMap.Remove(Handle);
    }

    GE_LOG(Log, TEXT("Pre-GC cleanup: %d GameplayEvent(s) removed."), HandlesToRemove.Num());
}

void UGameplayEventSubsystem::ClearAllGameplayEvents()
{
    FGameplayEventMapLock ActiveLock (*this);

    // We try to gracefully abort any active GameplayEvents.
    for (const auto& [Handle, SharedEvent] : EventMap)
    {
        UGameplayEvent* RawEvent = SharedEvent.Get();

        if (!RawEvent)
        {
            continue;
        }

        RawEvent->TryAbortEvent();
    }

    EventMap.Empty();
    PerActorEventMap.Empty();
}

void UGameplayEventSubsystem::RequestCleanup()
{
    if (IsLockActive())
    {
        bHasQueuedCleanup = true;
    }
    else
    {
        CleanupGameplayEvents();
    }
}

void UGameplayEventSubsystem::SetBlockAllEvents(bool InState)
{
    bBlockAllEvents = InState;
}

bool UGameplayEventSubsystem::GetBlockAllEvents() const
{
    return (bool)bBlockAllEvents;
}

FGameplayEventHandle UGameplayEventSubsystem::GetHandleForEvent(const UGameplayEvent* Event) const
{
    for (const auto& [Handle, ContainedEvent] : EventMap)
    {
        if (ContainedEvent == Event)
        {
            return Handle;
        }
    }

    return FGameplayEventHandle();
}

void UGameplayEventSubsystem::HandlePreGarbageCollect()
{
    RequestCleanup();
}

void UGameplayEventSubsystem::ProcessQueuedEvents()
{
    for (const FQueuedEvent& Event : EventQueue)
    {
        EventMap.Add(Event.Handle, Event.Event);
    }

    EventQueue.Empty();
}

void UGameplayEventSubsystem::EnqueueEvent(const FQueuedEvent& Event)
{
    EventQueue.Emplace(Event);
}

inline bool UGameplayEventSubsystem::IsLockActive() const
{
    return EventMapLockCount > 0;
}
