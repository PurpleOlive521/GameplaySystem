// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

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

	// --- Begin UAnimNotify Interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

#if WITH_EDITOR
	virtual bool ShouldFireInEditor() override;
#endif

	// --- End UAnimNotify Interface



protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ModifyGameplayTags")
	TArray<FGameplayTagModifier> TagModifiers;
};

