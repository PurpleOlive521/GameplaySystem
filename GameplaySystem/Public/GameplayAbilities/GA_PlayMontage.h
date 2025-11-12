// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility.h"
#include "GA_PlayMontage.generated.h"

class UGameplayEffect;
class UAnimInstance;

/**
 * GameplayAbility that plays an AnimMontage on activation. 
 * 
 * Ensures that the AnimMontage is stopped if the Ability is cancelled, and that OnMontageBlendingOut is called properly.
 * This is due to AnimNotify's being processed before Montage Events, meaning that AnimNotify's can cause OnMontageBlendingOut to 
 * be called twice - once if the AnimNotify cancels a Ability that plays AnimMontages, and once when the AnimMontage's BlendOutStarted Event is triggered due to the cancelled Ability.
 */
UCLASS(Blueprintable)
class GAMEPLAYSYSTEM_API UGA_PlayMontage : public UGameplayAbility
{
	GENERATED_BODY()
	
public:

	// --- Begin UGameplayAbility interface
	virtual void ActivateAbility_Implementation(FActiveGameplayAbility& OutActiveGameplayAbility) override;

	// Ensures that the AnimMontage is stopped if still playing and that OnMontageBlendingOut is called.
	virtual void CancelAbility_Implementation() override;

	// --- End UGameplayAbility interface

	// Override in derived class to act when the montage blends out. Is guaranteed to only be called once per animation.
	virtual void OnMontageBlendingOut(UAnimMontage* Montage, bool bWasInterrupted);

	// Override in derived class to act when the montage ends.
	virtual void OnMontageEnded(UAnimMontage* Montage, bool bWasInterrupted);

	// Plays a AnimMontage in the owning GameplaySystems AnimInstance. Returns the length of the played montage.
	// StartSection will play the AnimMontage at the start of the Section, and EndSection at the end of the Section. 
	// If both StartSection and EndSection is set, StartSection will take priority.
	float PlayMontage(UAnimMontage* MontageToPlay, float PlayRate = 1.0f, FName StartSection = FName(), FName EndSection = FName());

protected:

	// Keeps track of what Animation this Ability is responsible for playing.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GA_PlayMontage")
	TObjectPtr<UAnimMontage> AnimationMontage;
};
