// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GA_PlayMontage.h"

#include "DevelopmentTypes.h"


void UGA_PlayMontage::ActivateAbility(const FGameplayAbilityActivationData& ActivationData, FActiveGameplayAbility& OutActiveGameplayAbility)
{

}

void UGA_PlayMontage::CancelAbility()
{
	UGameplaySystemComponent* GameplaySystem = GetOwningComponent_Checked();

	const FGameplaySystemActorInfo* ActorInfo = GameplaySystem->GetActorInfo();

	if (UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance(); AnimInstance->Montage_IsPlaying(AnimationMontage))
	{
		// We know that the montage is playing and are explicitly calling the BlendOut callbacks ourselves, hence we unbind any existing delegates to avoid them being called twice.
		// Calling it ourselves ensures it happens this frame, instead of waiting for the montage to tick and call it next frame.
		if (FOnMontageBlendingOutStarted* BlendOutDelegate = AnimInstance->Montage_GetBlendingOutDelegate(AnimationMontage))
		{
			BlendOutDelegate->Unbind();
		}

		AnimInstance->Montage_Stop(OnCancelBlendoutTime, AnimationMontage);

		OnMontageBlendingOut(AnimationMontage, true);
	}

	Super::CancelAbility();
}

void UGA_PlayMontage::OnMontageBlendingOut(UAnimMontage* Montage, bool bWasInterrupted)
{

}

void UGA_PlayMontage::OnMontageEnded(UAnimMontage* Montage, bool bWasInterrupted)
{

}

float UGA_PlayMontage::PlayMontage(UAnimMontage* MontageToPlay, FPlayMontageParams& PlayParams)
{
	PlayParams.MontageBlendOutDelegate.BindUObject(this, &UGA_PlayMontage::OnMontageBlendingOut);
	PlayParams.MontageEndedDelegate.BindUObject(this, &UGA_PlayMontage::OnMontageEnded);

	// Delegating the call to GameplaySystem to allow them to track what ability played the montage.
	const float AnimDuration = GetOwningComponent_Checked()->PlayMontage(this, MontageToPlay, PlayParams);
	AnimationMontage = MontageToPlay;

	return AnimDuration;
}
