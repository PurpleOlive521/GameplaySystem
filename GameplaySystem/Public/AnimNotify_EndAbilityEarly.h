// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/GSAnimNotify.h"
#include "AnimNotify_EndAbilityEarly.generated.h"

/**
 * A bit of a misnomer as the Ability is not actually ENDED early but instead is marked for being cancelled by other activating abilities, allowing them to replace this ability once activated.
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
