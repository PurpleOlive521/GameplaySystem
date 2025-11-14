// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/GSAnimNotify.h"
#include "AnimNotify_ModifyAbility.generated.h"

/**
 * Modifies select properties on the GameplayAbility that started this AnimMontage. 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UAnimNotify_ModifyAbility : public UGSAnimNotify
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
	
	// If true, enables cancelling of the ability that started this AnimMontage.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ModifyAbility")
	bool bEnableAbilityCancelling = true;
};
