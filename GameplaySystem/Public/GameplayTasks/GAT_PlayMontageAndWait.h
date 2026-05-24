// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTasks/GameplayAbilityTask.h"
#include "GameplaySystemComponent.h"
#include "GAT_PlayMontageAndWait.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMontageWaitSimpleDelegate);

// Stripped down specialization of FPlayMontageParams for this task.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FPlayMontageAndWaitParams
{
	GENERATED_BODY()

	FPlayMontageAndWaitParams() = default;

	FPlayMontageParams MakePlayMontageParams() const;

	UPROPERTY(BlueprintReadWrite, EditAnywhere);
	float PlayRate = 1.0f;

	// Starts the Montage at the beginning of this section.
	UPROPERTY(BlueprintReadWrite, EditAnywhere);
	FName StartSection;

	// Starts the Montage at the end of the section instead.
	UPROPERTY(BlueprintReadWrite, EditAnywhere);
	bool bUseEndOfSection = false;
};

/**
 * 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UGAT_PlayMontageAndWait : public UGameplayAbilityTask
{
	GENERATED_BODY()

public:

	UGAT_PlayMontageAndWait(const FObjectInitializer& ObjectInitializer);

	// Starts playing an AnimMontage on the activating Ability's Actor and waits for it to finish.
	// If StopWhenAbilityEnds is true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "GameplayAbility|Tasks")
	static UGAT_PlayMontageAndWait* CreatePlayMontageAndWait(UGameplayAbility* OwningAbility, FName TaskName, UAnimMontage* MontageToPlay, const FPlayMontageAndWaitParams& Params,
		bool bStopWhenAbilityEnds = true, bool bAllowInterruptAfterBlendOut = false);

	virtual void Activate() override;

	virtual void ExternalCancel() override;

	virtual FString GetDebugString() const override;

	UFUNCTION()
	void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	UFUNCTION()
	void OnGameplayAbilityCancelled();

	UFUNCTION()
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:

	virtual void OnDestroy(bool AbilityEnded) override;

	// Checks if the ability is playing a montage and stops that montage, returns true if a montage was stopped, false if not.
	bool StopPlayingMontage();

	FDelegateHandle InterruptedHandle;

	UAnimMontage* MontageToPlay = nullptr;

	FPlayMontageParams Params;

	bool bStopWhenAbilityEnds = false;

	bool bAllowInterruptAfterBlendOut = false;

public:
	// --- Delegates

	// OnCompleted is called when the AnimMontage is completely done playing.
	UPROPERTY(BlueprintAssignable)
	FMontageWaitSimpleDelegate	OnCompleted;

	// OnBlendOut is called when the montage begins blending out.
	UPROPERTY(BlueprintAssignable)
	FMontageWaitSimpleDelegate	OnBlendOut;

	// OnInterrupted is called if another montage overwrites this, e.g. by playing another AnimMontage.
	UPROPERTY(BlueprintAssignable)
	FMontageWaitSimpleDelegate	OnInterrupted;

	// OnCancelled is called if the ability or task is cancelled.
	UPROPERTY(BlueprintAssignable)
	FMontageWaitSimpleDelegate	OnCancelled;
};
