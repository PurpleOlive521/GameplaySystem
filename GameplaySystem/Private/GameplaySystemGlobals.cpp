// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplaySystemGlobals.h"
#include "DevelopmentTypes.h"


void UGlobalGameplaySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
}

void UGlobalGameplaySubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UGlobalGameplaySubsystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

ETickableTickType UGlobalGameplaySubsystem::GetTickableTickType() const
{
    return Super::GetTickableTickType();
}

bool UGlobalGameplaySubsystem::IsTickable() const
{
    FAIL_ON_FAILED_SUPER(IsTickable());

    return true;
}

TStatId UGlobalGameplaySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UGlobalGameplaySubsystem, STATGROUP_Tickables);
}

UWorld* UGlobalGameplaySubsystem::GetTickableGameObjectWorld() const
{
    return GetWorld();
}

bool UGlobalGameplaySubsystem::IsTickableWhenPaused() const
{
    return false;
}

UGlobalGameplaySubsystem* UGlobalGameplaySubsystem::Get(const UObject* WorldContext)
{
    if (WorldContext)
    {
        const UWorld* World = WorldContext->GetWorld();
        if (World)
        {
            UGlobalGameplaySubsystem* GlobalGameplaySubsystem = World->GetSubsystem<UGlobalGameplaySubsystem>();
            ensure(GlobalGameplaySubsystem);

            return GlobalGameplaySubsystem;
        }
    }

    return nullptr;
}

UGameplaySystemComponent* UGlobalGameplaySubsystem::GetGlobalGameplaySystemComponent()
{
    if (!GlobalGameplaySystem)
    {
        if (UWorld* World = GetWorld())
        {
            FActorSpawnParameters Params;
            Params.Name = "GameplaySystemGlobalsActor";
            AActor* ParentActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
            ensure(ParentActor);

            GlobalGameplaySystem = Cast<UGameplaySystemComponent>(ParentActor->AddComponentByClass(UGameplaySystemComponent::StaticClass(), false, FTransform::Identity, false));
            ensure(GlobalGameplaySystem);
        }
    }

    return GlobalGameplaySystem;
}
