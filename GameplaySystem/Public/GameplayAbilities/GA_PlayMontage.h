// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbility.h"
#include "GameplaySystemComponent.h"
#include "GA_PlayMontage.generated.h"

class UGameplayEffect;
class UAnimInstance;

/**
 * GameplayAbility that plays an AnimMontage on activation. 
 */
UCLASS(Blueprintable)
class GAMEPLAYSYSTEM_API UGA_PlayMontage : public UGameplayAbility
{
	GENERATED_BODY()
	
public:

	// --- Begin UGameplayAbility Interface
	virtual void ActivateAbility(const FGameplayAbilityActivationData& ActivationData, FActiveGameplayAbility& OutActiveGameplayAbility) override;

	// Ensures that the AnimMontage is stopped if still playing and that OnMontageBlendingOut is called.
	virtual void CancelAbility() override;

	// --- End UGameplayAbility Interface

	// Called when the montage blends out.
	virtual void OnMontageBlendingOut(UAnimMontage* Montage, bool bWasInterrupted);

	// Called when the montage ends.
	virtual void OnMontageEnded(UAnimMontage* Montage, bool bWasInterrupted);

	// Plays a AnimMontage in the owning GameplaySystems AnimInstance. Returns the length of the played montage.
	float PlayMontage(UAnimMontage* MontageToPlay, FPlayMontageParams& PlayParams);

protected:

	// Keeps track of what Animation this Ability is responsible for playing.
	TObjectPtr<UAnimMontage> AnimationMontage;

	float OnCancelBlendoutTime = 0.1f;
};
