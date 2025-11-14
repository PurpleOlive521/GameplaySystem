// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "GameplayAbility.h"

#include "GameplaySystemComponent.h"
#include "DevelopmentTypes.h"
#include "Engine/Texture2D.h"

using namespace DebugTypes;

UGameplayAbility::UGameplayAbility()
{
	CurateProperties();
}

#if WITH_EDITOR
void UGameplayAbility::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	CurateProperties();
}
#endif

UWorld* UGameplayAbility::GetWorld() const
{	
	if (HasAllFlags(RF_ClassDefaultObject) == false)
	{
		// Get World from Outer
		return GetOuter()->GetWorld();
	}

	// Return nullptr if we are CDO.
	return nullptr;
}

void UGameplayAbility::FinishDestroy()
{
	Super::FinishDestroy();
}

void UGameplayAbility::Init(AActor* Actor, UGameplaySystemComponent* Component)
{
	OwningActor = Actor;
	OwningComponent = Component;
}

AActor* UGameplayAbility::GetOwningActor() const
{
	return OwningActor.Get();
}

AActor* UGameplayAbility::GetOwningActor_Checked() const
{
	check(OwningActor.IsValid())
	return OwningActor.Get();
}

UGameplaySystemComponent* UGameplayAbility::GetOwningComponent() const
{
	return OwningComponent.Get();
}

UGameplaySystemComponent* UGameplayAbility::GetOwningComponent_Checked() const
{
	check(OwningComponent.IsValid())
	return OwningComponent.Get();
}

bool UGameplayAbility::AttemptActivateAbility(FActiveGameplayAbility& OutGameplayAbilityHandle)
{
	UGameplaySystemComponent* ActivatorComponent = GetOwningComponent();
	
	check(ActivatorComponent);

	// TODO: Move over activation tag checks and that slew of logic to the GameplayAbility, so that it's all in one place.
	// We need to add it to the ApplyAbilityEndedModifiers/Remove to allow us to predict ability activation, since 
	// it is currently not possible to do ergonomically. It needs to be done on activation, apply/remove, end and cancel.
	// in short, this is due to the Abilities now having the blocking/cancelling on themselves, and would need to be applied/removed in ApplyAbilityEndedModifiers/remove calls.
	// This would force that part of the logic to be in the abilities, which is just too all over the place, and is a sign that it should all be moved to the abilities rather than 
	// split between both as it is now.

	if (!CheckAbilityRequirements())
	{
		return false;
	}

	const bool bSuccess = ApplyAbilityRequirements();

	if(bSuccess)
	{
		PreActivateAbility(OutGameplayAbilityHandle);
		ActivateAbility(OutGameplayAbilityHandle);

		if (InstancingPolicy != EInstancingPolicy::EIP_NoLifetime)
		{
			ActivatorComponent->AddAbilityHandle(this, OutGameplayAbilityHandle);
		}
	}
	else
	{
		GS_LOG(Error, TEXT("GameplayAbility: Ability not activated successfully, despite passing requirement check. "));
		return false;
	}

	return true;
}

bool UGameplayAbility::CheckAbilityRequirements_Implementation() const
{
	UGameplaySystemComponent* ActivatorComponent = GetOwningComponent_Checked();

	if (ActivatorComponent->HasCooldown(this))
	{
		return false;
	}

	// Check if the activating GameplaySystem has any tags that would block us
	if (ActivatorComponent->GetGameplayTagSystem()->HasAnyTag(ActivationBlockedTags))
	{
		return false;
	}

	// Check if the ability contains a blocked ability tag
	if (AbilityTags.HasAny(ActivatorComponent->GetBlockingAbilityTags()))
	{
		return false;
	}

	return true;
}

bool UGameplayAbility::ApplyAbilityRequirements_Implementation()
{
	return true;
}

void UGameplayAbility::PreActivateAbility_Implementation(FActiveGameplayAbility& OutActiveGameplayAbility)
{
	// Set up handle with generic Ability values.
	SetupAbilityHandle(OutActiveGameplayAbility);

	UGameplaySystemComponent* ActivatorComponent = GetOwningComponent_Checked();

	// By the time we receive this call we should be sure that the ability can activate, and can now start cancelling abilities
	ActivatorComponent->ApplyBlockingAndCancellingTags(BlockAbilitiesWithTag, CancelAbilitiesWithTag);

	bHasAppliedAbilityEndedModifiers = false;
}

void UGameplayAbility::ActivateAbility_Implementation(FActiveGameplayAbility& OutActiveGameplayAbility)
{
	// Very sad and empty...
}

void UGameplayAbility::EndAbility_Implementation()
{
	TryApplyAbilityEndedModifiers();
}

void UGameplayAbility::CancelAbility_Implementation()
{
	TryApplyAbilityEndedModifiers();
}

bool UGameplayAbility::TryApplyAbilityEndedModifiers()
{
	if (bHasAppliedAbilityEndedModifiers)
	{
		return false;
	}

	bHasAppliedAbilityEndedModifiers = true;
	ApplyAbilityEndedModifiers();

	return true;
}

void UGameplayAbility::ApplyAbilityEndedModifiers_Implementation()
{
	UGameplaySystemComponent* ActivatorComponent = GetOwningComponent_Checked();
	
	ActivatorComponent->RemoveBlockingTags(BlockAbilitiesWithTag);

	// Any modifiers that might affect another abilities activation that will trigger on the abilities end or cancellation should be applied here instead.
	// This could be modifiers such as attribute changes, GameplayEffects, GameplayTags, blocking tags, etc.
}

bool UGameplayAbility::TryRemoveAbilityEndedModifiers()
{
	if (!bHasAppliedAbilityEndedModifiers)
	{
		return false;
	}

	bHasAppliedAbilityEndedModifiers = false;
	RemoveAbilityEndedModifiers();

	return true;
}

void UGameplayAbility::RemoveAbilityEndedModifiers_Implementation()
{
	UGameplaySystemComponent* ActivatorComponent = GetOwningComponent_Checked();

	ActivatorComponent->ApplyBlockingAndCancellingTags(BlockAbilitiesWithTag, FGameplayTagContainer());

	// Any modifiers that might affect another abilities activation that will trigger on the abilities end or cancellation should be applied here instead.
	// This could be modifiers such as attribute changes, GameplayEffects, GameplayTags, blocking tags, etc.
}

void UGameplayAbility::SetupAbilityHandle(FActiveGameplayAbility& OutActiveGameplayAbility)
{
	OutActiveGameplayAbility = FActiveGameplayAbility(this);
}

float UGameplayAbility::GetCooldown() const
{
	return Cooldown;
}

float UGameplayAbility::GetDuration() const
{
	return Duration;
}

bool UGameplayAbility::IsCancellable() const
{
	return bIsCancellable;
}

void UGameplayAbility::SetIsCancellable(bool bInIsCancellable)
{
	if (bIsCancellable == bInIsCancellable)
	{
		return;
	}

	bIsCancellable = bInIsCancellable;
}

const FGameplayTagContainer& UGameplayAbility::GetAbilityTags() const
{
	return AbilityTags;
}

EInstancingPolicy UGameplayAbility::GetInstancingPolicy() const
{
	return InstancingPolicy;
}

FString UGameplayAbility::ToString() const
{
	FString DisplayInfo = TEXT("Ability: ") + GetDisplayName() + ENDL;
	DisplayInfo += FString::Printf(TEXT("Duration: %.2f, Cooldown : %.2f"), Duration, Cooldown);

	return DisplayInfo;
}

FString UGameplayAbility::ToStringWithDebugTags() const
{
	FString DisplayInfo = TEXT("Ability: ") + TextTag_Highlight + GetDisplayName() + TextTag_End + ENDL;
	DisplayInfo += FString::Printf(TEXT("Duration: %.2f, Cooldown : %.2f"), Duration, Cooldown);
	
	return DisplayInfo;
}

FString UGameplayAbility::GetDisplayName() const
{
	return DisplayName;
}



void UGameplayAbility::CurateProperties()
{
	if (bSameDurationAndCooldown)
	{
		Cooldown = Duration;
	}
}

FActiveGameplayAbility::FActiveGameplayAbility(UGameplayAbility* BaseAbility)
{
	// Set up handle
	GameplayAbility = BaseAbility;
	ElapsedTime = 0.0f;

	Duration = BaseAbility->GetDuration();
	Cooldown = BaseAbility->GetCooldown();

	bHasDurationElapsed = false;
	bHasCooldownElapsed = false;

	AbilityTags = BaseAbility->GetAbilityTags();
}

void FActiveGameplayAbility::Tick(float DeltaTime)
{
	ElapsedTime += DeltaTime;
	if (ElapsedTime >= Duration)
	{
		bHasDurationElapsed = true;
	}

	if (ElapsedTime >= Cooldown)
	{
		bHasCooldownElapsed = true;
	}
}

float FActiveGameplayAbility::GetCurrentCooldown() const
{
	if (bHasCooldownElapsed == true)
	{
		return 0.0f;
	}

	return FMath::Abs(ElapsedTime - Cooldown);
}

float FActiveGameplayAbility::GetCurrentDuration() const
{
	if (bHasDurationElapsed == true)
	{
		return 0.0f;
	}

	return FMath::Abs(ElapsedTime - Duration);
}

float FActiveGameplayAbility::GetRemainingDuration() const
{
	if (bHasDurationElapsed == true)
	{
		return 0.0f;
	}

	return Duration - ElapsedTime;
}

bool FActiveGameplayAbility::IsAbilityActive() const
{
	return !bHasCooldownElapsed || !bHasDurationElapsed;
}

void FActiveGameplayAbility::CancelAbility()
{
	if (GameplayAbility)
	{
		if (!bHasBeenEnded)
		{
			GameplayAbility->CancelAbility();
			bHasBeenEnded = true;
		}
	}
	else
	{
		GS_LOG(Error, TEXT("FActiveGameplayAbility::CancelAbility: Attempting to cancel an ability with no valid instance."));
	}
}

void FActiveGameplayAbility::EndAbility()
{
	if (GameplayAbility)
	{
		if (!bHasBeenEnded)
		{
			GameplayAbility->EndAbility();
			bHasBeenEnded = true;
		}
	}
	else
	{
		GS_LOG(Error, TEXT("FActiveGameplayAbility::CancelAbility: Attempting to cancel an ability with no valid instance."));
	}
}

void FActiveGameplayAbility::Reset()
{
	GameplayAbility = nullptr;

	bHasDurationElapsed = false;
	bHasCooldownElapsed = false;
	bHasBeenEnded = false;

	Duration = 0;
	Cooldown = 0;
	ElapsedTime = 0;
}

FString FActiveGameplayAbility::ToString() const
{
	FString DisplayInfo = (IsValid(GameplayAbility) ? GameplayAbility->GetDisplayName() : TEXT("Invalid Ability")) + TEXT(":") + ENDL;
	DisplayInfo += FString::Printf(TEXT("Duration Rmng: %.1f"), GetRemainingDuration()) + FString::Printf(TEXT(", Cooldown: %.1f"), GetCurrentCooldown());
	DisplayInfo += FString::Printf(TEXT(", RcvEnd: %s"), bHasBeenEnded ? TEXT("true") : TEXT("false")) + ENDL;

	return DisplayInfo;
}

FString FActiveGameplayAbility::ToStringWithDebugTags() const
{	
	FString DisplayInfo = TextTag_Highlight + (IsValid(GameplayAbility) ? GameplayAbility->GetDisplayName() : TEXT("Invalid Ability")) + TEXT(":") + TextTag_End + ENDL;
	DisplayInfo += FString::Printf(TEXT("Duration Rmng: %.1f"), GetRemainingDuration()) + FString::Printf(TEXT(", Cooldown: %.1f"), GetCurrentCooldown());
	DisplayInfo += FString::Printf(TEXT(", RcvEnd: %s"), bHasBeenEnded ? TEXT("true") : TEXT("false")) + ENDL;

	return DisplayInfo;
}
