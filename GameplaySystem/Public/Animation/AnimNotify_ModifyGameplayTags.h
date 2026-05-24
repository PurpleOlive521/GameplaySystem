// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "GSAnimNotify.h"
#include "GameplayTagTypes.h"

#include "AnimNotify_ModifyGameplayTags.generated.h"

/**
 * Modifies one or more GameplayTags on the Actor once this AnimNotify is hit.
 */
UCLASS()
class GAMEPLAYSYSTEM_API UAnimNotify_ModifyGameplayTags : public UGSAnimNotify
{
	GENERATED_BODY()

public:

	UAnimNotify_ModifyGameplayTags();

	// --- Begin UAnimNotify Interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	// --- End UAnimNotify Interface

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ModifyGameplayTags")
	TArray<FGameplayTagModifier> TagModifiers;
};

