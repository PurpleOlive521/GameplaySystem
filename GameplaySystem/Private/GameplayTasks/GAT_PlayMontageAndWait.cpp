// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayTasks/GAT_PlayMontageAndWait.h"
#include "DevelopmentTypes.h"

FPlayMontageParams FPlayMontageAndWaitParams::MakePlayMontageParams() const
{
	FPlayMontageParams Params = FPlayMontageParams();

	Params.PlayRate = PlayRate;
	Params.StartSection = StartSection;
	Params.bUseEndOfSection = bUseEndOfSection;

	return Params;
}

UGAT_PlayMontageAndWait::UGAT_PlayMontageAndWait(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGAT_PlayMontageAndWait* UGAT_PlayMontageAndWait::CreatePlayMontageAndWait(UGameplayAbility* OwningAbility, FName TaskName, UAnimMontage* MontageToPlay, const FPlayMontageAndWaitParams& Params, bool bStopWhenAbilityEnds, bool bAllowInterruptAfterBlendOut)
{
	UGAT_PlayMontageAndWait* NewTask = NewAbilityTask<UGAT_PlayMontageAndWait>(OwningAbility, TaskName);
	NewTask->MontageToPlay = MontageToPlay;
	NewTask->Params = Params.MakePlayMontageParams();
	NewTask->bStopWhenAbilityEnds = bStopWhenAbilityEnds;
	NewTask->bAllowInterruptAfterBlendOut = bAllowInterruptAfterBlendOut;

	return NewTask;
}

void UGAT_PlayMontageAndWait::Activate()
{
	if (!MontageToPlay)
	{
		EndTask();
		return;
	}

	bool bPlayedMontage = false;

	UGameplaySystemComponent* GS = Ability->GetOwningComponent();
	if (!GS)
	{
		GS_LOG(Warning, TEXT("UGAT_PlayMontageAndWait called on invalid GameplaySystemComponent!"));
		EndTask();
		return;
	}

	const FGameplaySystemActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	if (!AnimInstance)
	{
		GS_LOG(Warning, TEXT("UGAT_PlayMontageAndWait no AnimInstance found!"));
		EndTask();
		return;
	}

	InterruptedHandle = Ability->OnAbilityCancelledDelegate.AddUObject(this, &UGAT_PlayMontageAndWait::OnGameplayAbilityCancelled);

	Params.MontageBlendOutDelegate.Unbind();
	Params.MontageBlendOutDelegate.BindUObject(this, &UGAT_PlayMontageAndWait::OnMontageBlendingOut);

	Params.MontageEndedDelegate.Unbind();
	Params.MontageEndedDelegate.BindUObject(this, &UGAT_PlayMontageAndWait::OnMontageEnded);

	const float Duration = GS->PlayMontage(Ability, MontageToPlay, Params);
	if (Duration != 0.0f)
	{
		// Playing a montage could potentially fire off a callback into game code which could kill this task! 
		if (ShouldBroadcastAbilityTaskDelegates() == false)
		{
			return;
		}

		bPlayedMontage = true;
	}

	if (!bPlayedMontage)
	{
		GS_LOG(Warning, TEXT("UGAT_PlayMontageAndWait called in Ability %s failed to play montage %s; Task Instance Name %s."), *Ability->GetName(), *GetNameSafe(MontageToPlay), *InstanceName.ToString());
		EndTask();
		return;
	}

	SetWaitingOnAvatar();
}

void UGAT_PlayMontageAndWait::ExternalCancel()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnCancelled.Broadcast();
	}

	Super::ExternalCancel();
}

FString UGAT_PlayMontageAndWait::GetDebugString() const
{
	UAnimMontage* PlayingMontage = nullptr;
	if (Ability)
	{
		const FGameplaySystemActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
		UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();

		if (AnimInstance != nullptr)
		{
			PlayingMontage = AnimInstance->Montage_IsActive(MontageToPlay) ? MontageToPlay : AnimInstance->GetCurrentActiveMontage();
		}
	}

	return FString::Printf(TEXT("PlayMontageAndWait: %s"), *GetNameSafe(MontageToPlay));
}

void UGAT_PlayMontageAndWait::OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		if (bInterrupted)
		{
			OnInterrupted.Broadcast();

			EndTask();
		}
		else
		{
			OnBlendOut.Broadcast();
		}
	}
}

void UGAT_PlayMontageAndWait::OnGameplayAbilityCancelled()
{
	if (StopPlayingMontage() || bAllowInterruptAfterBlendOut)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnInterrupted.Broadcast();
		}
	}

	EndTask();
}

void UGAT_PlayMontageAndWait::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (!bInterrupted)
	{
		if (ShouldBroadcastAbilityTaskDelegates())
		{
			OnCompleted.Broadcast();
		}
	}

	EndTask();
}

void UGAT_PlayMontageAndWait::OnDestroy(bool AbilityEnded)
{
	// We don't need to unbind from the AnimInstance
	// We do need to on the Ability since it's multicast
	if (Ability)
	{
		Ability->OnAbilityCancelledDelegate.Remove(InterruptedHandle);
		if (AbilityEnded && bStopWhenAbilityEnds)
		{
			StopPlayingMontage();
		}
	}

	Super::OnDestroy(AbilityEnded);
}

bool UGAT_PlayMontageAndWait::StopPlayingMontage()
{
	if (Ability == nullptr)
	{
		return false;
	}

	const FGameplaySystemActorInfo* ActorInfo = Ability->GetCurrentActorInfo();
	if (!ActorInfo)
	{
		return false;
	}

	UAnimInstance* AnimInstance = ActorInfo->GetAnimInstance();
	if (!AnimInstance)
	{
		return false;
	}

	// Check if the montage is still playing
	// The ability would have been interrupted, in which case we should automatically stop the montage
	UGameplaySystemComponent* GS = GameplaySystem.Get();
	if (GS && Ability)
	{
		if (GS->GetAnimatingAbility() == Ability && GS->GetCurrentAnimMontage() == MontageToPlay)
		{
			// Unbind delegates so they don't get called as well
			FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveInstanceForMontage(MontageToPlay);
			if (MontageInstance)
			{
				MontageInstance->OnMontageBlendingOutStarted.Unbind();
				MontageInstance->OnMontageEnded.Unbind();
			}

			GS->StopCurrentMontage();
			return true;
		}
	}

	return false;
}
