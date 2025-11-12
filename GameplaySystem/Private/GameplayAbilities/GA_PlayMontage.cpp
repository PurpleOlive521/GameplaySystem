// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "GA_PlayMontage.h"

#include "GameplaySystemComponent.h"
#include "DevelopmentTypes.h"



void UGA_PlayMontage::ActivateAbility_Implementation(FActiveGameplayAbility& OutActiveGameplayAbility)
{
	const float MontageDuration = PlayMontage(AnimationMontage);
	OutActiveGameplayAbility.Duration = MontageDuration;
}

void UGA_PlayMontage::CancelAbility_Implementation()
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

		AnimInstance->Montage_Stop(0.1f, AnimationMontage);

		OnMontageBlendingOut(AnimationMontage, true);
	}

	Super::CancelAbility_Implementation();
}

void UGA_PlayMontage::OnMontageBlendingOut(UAnimMontage* Montage, bool bWasInterrupted)
{

}

void UGA_PlayMontage::OnMontageEnded(UAnimMontage* Montage, bool bWasInterrupted)
{

}

float UGA_PlayMontage::PlayMontage(UAnimMontage* MontageToPlay, float PlayRate, FName StartSection, FName EndSection)
{
	// Delegating the call to GameplaySystem to allow them to track what ability played what montage.
	const float AnimDuration = GetOwningComponent_Checked()->PlayMontage(this, MontageToPlay, PlayRate, StartSection, EndSection);

	FOnMontageBlendingOutStarted MontageBlendOutDelegate;
	MontageBlendOutDelegate.BindUObject(this, &UGA_PlayMontage::OnMontageBlendingOut);

	FOnMontageEnded MontageEndedDelegate;
	MontageEndedDelegate.BindUObject(this, &UGA_PlayMontage::OnMontageEnded);

	const FGameplaySystemActorInfo* ActorInfo = GetOwningComponent_Checked()->GetActorInfo();
	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	check(AnimInstance)

	AnimInstance->Montage_SetBlendingOutDelegate(MontageBlendOutDelegate, AnimationMontage);
	AnimInstance->Montage_SetEndDelegate(MontageEndedDelegate, AnimationMontage);

	return AnimDuration;
}
