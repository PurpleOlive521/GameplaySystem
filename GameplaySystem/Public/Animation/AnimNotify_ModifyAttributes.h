// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/GSAnimNotify.h"
#include "AttributeEffect.h"
#include "AnimNotify_ModifyAttributes.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UAnimNotify_ModifyAttributes : public UGSAnimNotify
{
	GENERATED_BODY()
	
public:

	UAnimNotify_ModifyAttributes();

	// --- Begin UAnimNotify Interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	// --- End UAnimNotify Interface

	UPROPERTY(EditAnywhere)
	TArray<FAttributeEffect> AttributeModifiers;
};
