// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/GSAnimNotify.h"
#include "GameplayEventTypes.h"
#include "AnimNotify_TriggerGameplayEvents.generated.h"

class UGameplayEvent;

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FAnimNotifyGameplayEvent
{
	GENERATED_BODY()

	FAnimNotifyGameplayEvent() = default;

	UPROPERTY(EditAnywhere)
	FGameplayEventActivationData ActivationData;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEvent> Event = nullptr;
};

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayEventTrace
{
	GENERATED_BODY()

	FGameplayEventTrace() = default;

	TOptional<FHitResult> PerformTrace(UWorld* World, USkeletalMeshComponent* MeshComp);

	// Any results will be sent through ActivationData to the GameplayEvent.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayEventTrace")
	bool bTrace = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayEventTrace")
	TEnumAsByte<ECollisionChannel> Channel = ECollisionChannel::ECC_Visibility;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayEventTrace")
	FVector EndOffsets = FVector::ZeroVector;

	// Optional socket to trace from.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayEventTrace")
	FName Socket;

	// Whether or not we still trigger the GameplayEvents if we are not blocked with the trace.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayEventTrace")
	bool bTriggerIfNotBlocking = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayEventTrace")
	bool bIgnoreSelf = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayEventTrace")
	bool bDebug = false;
};

/**
 * 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UAnimNotify_TriggerGameplayEvents : public UGSAnimNotify
{
	GENERATED_BODY()
	
public:

	UAnimNotify_TriggerGameplayEvents();

	// --- Begin UAnimNotify Interface
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	// --- End UAnimNotify Interface

protected:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TriggerGameplayEvent")
	TArray<FAnimNotifyGameplayEvent> EventsToTrigger;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "TriggerGameplayEvent")
	FGameplayEventTrace EventTrace;
};
