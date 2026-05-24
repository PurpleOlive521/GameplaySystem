// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GSAnimNotifyState.h"

#include "GameplayTagTypes.h"

#include "AnimNotifyState_TimedGameplayTag.generated.h"

struct FGameplayTagSystem;

/**
 * Modifies one or more GameplayTags on the Actor, with the chance to modify again once the AnimNotifyState has ended.
 * Guaranteed to operate on the same GameplayTagSystem on both Begin and End.
 */
UCLASS()
class GAMEPLAYSYSTEM_API UAnimNotifyState_TimedGameplayTag : public UGSAnimNotifyState
{
	GENERATED_BODY()

public:

	UAnimNotifyState_TimedGameplayTag();

	// --- Begin UAnimNotifyState Interface
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	// --- End UAnimNotifyState Interface

protected:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TimedGameplayTag")
	TArray<FGameplayTagModifier> TagModifiersOnBegin;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TimedGameplayTag", meta = (EditCondition = "bUndoOnEnd == false"))
	TArray<FGameplayTagModifier> TagModifiersOnEnd;

	// Inverts the values of TagModifiersOnBegin, undoing any changes this has applied at the end. If enabled also disables TagModifiersOnEnd.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TimedGameplayTag")
	bool bUndoOnEnd = false;

private:

	// Cached so we operate on the same instance throughout the notify's lifetime.
	FGameplayTagSystem* GameplayTagSystem;
};
