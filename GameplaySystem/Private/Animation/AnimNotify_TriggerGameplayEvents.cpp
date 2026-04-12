// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "Animation/AnimNotify_TriggerGameplayEvents.h"
#include "GameplayEventSubsystem.h"

void UAnimNotify_TriggerGameplayEvents::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	AActor* Owner = MeshComp->GetOwner();

	ensure(Owner);

	UGameplayEventSubsystem* Subsystem = UGameplayEventSubsystem::Get(Owner);
	if (Subsystem)
	{
		for(const auto& EventClass : EventsToTrigger)
		Subsystem->TriggerEvent(EventClass, Owner);
	}
}

#if WITH_EDITOR
bool UAnimNotify_TriggerGameplayEvents::ShouldFireInEditor()
{
	return false;
}
#endif
