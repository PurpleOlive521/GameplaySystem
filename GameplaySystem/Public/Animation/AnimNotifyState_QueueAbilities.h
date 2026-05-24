// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "GSAnimNotifyState.h"

#include "AnimNotifyState_QueueAbilities.generated.h"

struct FGameplayTagSystem;

// When do we want to try to activate the queued ability?
UENUM(BlueprintType)
enum class EQueueTriggers: uint8
{
	EQT_AnimNotifyEnd	UMETA(DisplayName = "AnimNotify End"),
	EQT_AnimNotifyTick	UMETA(DisplayName = "AnimNotify Tick"),
};

/**
 * Queues any abilities that are unsuccessfully performed within this state, and attempts to activate it once the AnimNotifyState has ended.
 */
UCLASS(Blueprintable)
class GAMEPLAYSYSTEM_API UAnimNotifyState_QueueAbilities : public UGSAnimNotifyState
{
	GENERATED_BODY()

public:

	UAnimNotifyState_QueueAbilities();

	// --- Begin UAnimNotifyState Interface
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	// --- End UAnimNotifyState Interface

	void TryTriggerQueuedAbility(USkeletalMeshComponent* MeshComp);

protected:
	
	// Determines at what AnimNotify events we want to try to activate any queued ability.
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EQueueTriggers TriggerPolicy = EQueueTriggers::EQT_AnimNotifyEnd;


private:
	FGameplayTagSystem* GameplayTagSystem;
};
