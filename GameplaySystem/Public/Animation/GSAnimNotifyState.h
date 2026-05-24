// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GSAnimNotifyState.generated.h"

/**
 * 
 */
UCLASS(Blueprintable)
class GAMEPLAYSYSTEM_API UGSAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

#if WITH_EDITOR
	virtual void OnAnimNotifyCreatedInEditor(FAnimNotifyEvent& ContainingAnimNotifyEvent) override;
#endif //WITH_EDITOR

protected:

#if WITH_EDITORONLY_DATA

	UPROPERTY(BlueprintReadOnly)
	bool bIsPlayingInEditor = false;

#endif //WITH_EDITORONLY_DATA

};
