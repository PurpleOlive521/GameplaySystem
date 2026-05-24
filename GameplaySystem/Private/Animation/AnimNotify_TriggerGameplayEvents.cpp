// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "Animation/AnimNotify_TriggerGameplayEvents.h"
#include "GameplayEventSubsystem.h"

UAnimNotify_TriggerGameplayEvents::UAnimNotify_TriggerGameplayEvents()
{
#if WITH_EDITORONLY_DATA
	bShouldFireInEditor = false;
#endif //WITH_EDITORONLY_DATA
}

TOptional<FHitResult> FGameplayEventTrace::PerformTrace(UWorld* World, USkeletalMeshComponent* MeshComp)
{
	check(MeshComp);
	check(World);

	FHitResult Result;
	AActor* Owner = MeshComp->GetOwner();
	FVector Start = MeshComp->GetComponentLocation();

	if (Socket.IsValid())
	{
		Start = MeshComp->GetSocketLocation(Socket);
	}

	const FVector End = Start + EndOffsets;

	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;

	if (bIgnoreSelf)
	{
		QueryParams.AddIgnoredActor(Owner);
		QueryParams.AddIgnoredComponent(MeshComp);
	}

	if (bDebug) 
	{
		DrawDebugLine(World, Start, End, FColor::Blue, true, 10.0f);
	}

	const bool bBlocked = World->LineTraceSingleByChannel(Result, Start, End, Channel, QueryParams);
	if (not bBlocked && not bTriggerIfNotBlocking)
	{
		return TOptional<FHitResult>();
	}

	if (bDebug)
	{
		DrawDebugSphere(World, Result.Location, 30.0f, 32, FColor::Red, true, 10.0f);
	}

	return TOptional<FHitResult>(Result);
}

void UAnimNotify_TriggerGameplayEvents::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp->GetOwner();

	ensure(Owner);

	UGameplayEventSubsystem* Subsystem = UGameplayEventSubsystem::Get(Owner);
	if (Subsystem)
	{
		TOptional<FHitResult> OptionalResults;

		if (EventTrace.bTrace)
		{
			OptionalResults = EventTrace.PerformTrace(Owner->GetWorld(), MeshComp);
		}
		
		for (const auto& AnimNotifyEvent : EventsToTrigger)
		{
			FGameplayEventActivationData Data = AnimNotifyEvent.ActivationData;
			if (OptionalResults.IsSet()) 
			{
				Data.HitResults = OptionalResults.GetValue();
			}

			Subsystem->TriggerEvent_ActivationData(AnimNotifyEvent.Event, Owner, Data);
		}
	}
}
