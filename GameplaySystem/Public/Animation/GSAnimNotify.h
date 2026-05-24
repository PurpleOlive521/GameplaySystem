// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GSAnimNotify.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class GAMEPLAYSYSTEM_API UGSAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
	
#if WITH_EDITOR
	virtual void OnAnimNotifyCreatedInEditor(FAnimNotifyEvent& ContainingAnimNotifyEvent) override;
#endif //WITH_EDITOR

protected:

	UPROPERTY(BlueprintReadOnly)
	bool bIsPlayingInEditor = false;
};
