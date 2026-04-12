// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/GSAnimNotify.h"
#include "AnimNotify_TriggerGameplayEvents.generated.h"

class UGameplayEvent;

/**
 * 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UAnimNotify_TriggerGameplayEvents : public UGSAnimNotify
{
	GENERATED_BODY()
	
public:

	// --- Begin UAnimNotify Interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

#if WITH_EDITOR
	virtual bool ShouldFireInEditor() override;
#endif

	// --- End UAnimNotify Interface

protected:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TriggerGameplayEvent")
	TArray<TSubclassOf<UGameplayEvent>> EventsToTrigger;
};
