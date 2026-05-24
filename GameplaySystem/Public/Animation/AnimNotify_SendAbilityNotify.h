// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/GSAnimNotify.h"
#include "AnimNotify_SendAbilityNotify.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UAnimNotify_SendAbilityNotify : public UGSAnimNotify
{
	GENERATED_BODY()
	
public:

	UAnimNotify_SendAbilityNotify();

	// --- Begin UAnimNotify Interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	// --- End UAnimNotify Interface

protected:

	// The notify to send.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "SendAbilityNotify")
	FName NotifyName;
};
