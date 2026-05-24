// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/GSAnimNotifyState.h"
#include "GameplayEffect.h"
#include "AnimNotifyState_GameplayEffects.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UAnimNotifyState_GameplayEffects : public UGSAnimNotifyState
{
	GENERATED_BODY()
	
public:

	UAnimNotifyState_GameplayEffects();

	// --- Begin UAnimNotifyState Interface
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	// --- End UAnimNotifyState Interface

protected:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayEffects")
	TArray<TSubclassOf<UGameplayEffect>> EffectsOnBegin;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayEffects")
	TArray<TSubclassOf<UGameplayEffect>> EffectsOnEnd;

	// Removes any GameplayEffecs that where applied on NotifyBegin on NotifyEnd.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayEffects")
	bool bRemoveAppliedBeginEffectsOnEnd = false;

	TArray<FGameplayEffectHandle> AppliedEffects;
};
