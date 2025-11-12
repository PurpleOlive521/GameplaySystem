// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/GSAnimNotify.h"
#include "AnimNotify_EndAbilityEarly.generated.h"

/**
 * Ends the ability that triggered this AnimMontage prematurely. Not intended for "natural" endings, that should be controlled by setting the Ability's duration.
 */
UCLASS()
class GAMEPLAYSYSTEM_API UAnimNotify_EndAbilityEarly : public UGSAnimNotify
{
	GENERATED_BODY()
	
public:

	// --- Begin UAnimNotify Interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

#if WITH_EDITOR
	virtual bool ShouldFireInEditor() override;
#endif

	// --- End UAnimNotify Interface
};
