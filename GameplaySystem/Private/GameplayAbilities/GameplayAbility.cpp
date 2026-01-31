// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


#include "GameplayAbility.h"

#include "GameplaySystemComponent.h"
#include "DevelopmentTypes.h"
#include "Engine/Texture2D.h"
#include "GameplayAbilitySlot.h"

using namespace DebugTypes;

DEFINE_LOG_CATEGORY(LogGameplayAbility)

FActiveGameplayAbility::FActiveGameplayAbility(UGameplayAbility* BaseAbility, FGameplayAbilityHandle SourceHandle)
{
	check(BaseAbility);

	bIsValid = true;

	Ability = BaseAbility;
	Handle = SourceHandle;

	ElapsedTime = 0.0f;

	Duration = BaseAbility->GetDuration();
	Cooldown = BaseAbility->GetCooldown();

	bHasDurationElapsed = false;
	bHasCooldownElapsed = false;

	AbilityTags = BaseAbility->GetAbilityTags();
}

void FActiveGameplayAbility::Tick(float DeltaTime, UGameplaySystemComponent* GameplaySystem)
{
	ElapsedTime += DeltaTime;

	if (!bHasDurationElapsed && bHasActivated)
	{
		if (ElapsedTime >= Duration)
		{
			bHasDurationElapsed = true;
			GameplaySystem->EndAbility(Handle);
		}
	}

	if (!bHasCooldownElapsed && bHasActivated)
	{
		if (ElapsedTime >= Cooldown)
		{
			bHasCooldownElapsed = true;
		}
	}
}

float FActiveGameplayAbility::GetRemainingCooldown() const
{
	if (bHasCooldownElapsed == true)
	{
		return GameplayAbilityConstants::NO_COOLDOWN;
	}

	return FMath::Clamp(Cooldown - ElapsedTime, GameplayAbilityConstants::NO_COOLDOWN, FLT_MAX);
}

float FActiveGameplayAbility::GetRemainingDuration() const
{
	if (bHasDurationElapsed == true)
	{
		return GameplayAbilityConstants::NO_DURATION;
	}

	return FMath::Clamp(Duration - ElapsedTime, GameplayAbilityConstants::NO_DURATION, FLT_MAX);
}

float FActiveGameplayAbility::GetRemainingCooldownAsPercentage() const
{
	return GetRemainingCooldown() / Cooldown;
}

float FActiveGameplayAbility::GetRemainingDurationAsPercentage() const
{
	return GetRemainingDuration() / Duration;
}

void FActiveGameplayAbility::SetDuration(float Value)
{
	Duration = Value;

	ensure(Ability);

	if (Ability->bForceSameDurationAndCooldown)
	{
		Cooldown = Value;
	}
}

void FActiveGameplayAbility::SetCooldown(float Value)
{
	Cooldown = Value;

	ensure(Ability);

	if (Ability->bForceSameDurationAndCooldown)
	{
		Duration = Value;
	}
}

bool FActiveGameplayAbility::IsAbilityActive() const
{
	ensure(Ability);
	return Ability->bIsActive;
}

bool FActiveGameplayAbility::HasActiveState() const
{
	const bool bHasActiveState = IsAbilityActive() || HasCooldown();

	return bHasActiveState;
}

bool FActiveGameplayAbility::ShouldBeRemoved() const
{
	return !HasActiveState() || !bHasActivated;
}

bool FActiveGameplayAbility::IsValid() const
{
	return bIsValid;
}

bool FActiveGameplayAbility::HasCooldown() const
{
	if (!bHasActivated)
	{
		return false;
	}

	return !bHasCooldownElapsed;
}

FString FActiveGameplayAbility::ToString() const
{
	if (!Ability)
	{
		return TEXT("INVALID ABILITY: NULLPTR");
	}

	FString DisplayInfo = Ability->GetDisplayName() + TEXT(":") + ENDL;
	DisplayInfo += FString::Printf(TEXT("Duration Rmng: %.1f"), GetRemainingDuration()) + FString::Printf(TEXT(", Cooldown: %.1f"), GetRemainingCooldown());
	DisplayInfo += FString::Printf(TEXT(", Ended: %s, Cancel: %s"), Ability->bHasEnded ? TEXT("true") : TEXT("false"), Ability->bHasCancelled ? TEXT("true") : TEXT("false")) + ENDL;

	for (const auto& GameplayTag : AbilityTags.GetGameplayTagArray())
	{
		DisplayInfo += GameplayTag.ToString() + ENDL;
	}

	return DisplayInfo;
}

FString FActiveGameplayAbility::ToStringWithDebugTags() const
{
	if (!Ability)
	{
		return TEXT("INVALID ABILITY: NULLPTR");
	}

	FString DisplayInfo = TextTag_Highlight + Ability->GetDisplayName() + TEXT(":") + TextTag_End + ENDL;
	DisplayInfo += FString::Printf(TEXT("Duration Rmng: %.1f"), GetRemainingDuration()) + FString::Printf(TEXT(", Cooldown: %.1f"), GetRemainingCooldown());
	DisplayInfo += FString::Printf(TEXT(", Ended: %s, Cancel: %s"), Ability->bHasEnded ? TEXT("true") : TEXT("false"), Ability->bHasCancelled ? TEXT("true") : TEXT("false")) + ENDL;

	for (const auto& GameplayTag : AbilityTags.GetGameplayTagArray())
	{
		DisplayInfo += GameplayTag.ToString() + ENDL;
	}

	return DisplayInfo;
}

UGameplayAbility::UGameplayAbility()
{
	CurateProperties();

	auto IsFunctionImplementedInBlueprint = [](const UFunction* Func) -> bool
		{
			return Func && ensure(Func->GetOuter()) && Func->GetOuter()->IsA(UBlueprintGeneratedClass::StaticClass());
		};

	{
		static FName FuncName = FName(TEXT("K2_CheckAbilityRequirements"));
		UFunction* BlueprintFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintCheckAbilityRequirements = IsFunctionImplementedInBlueprint(BlueprintFunction);
	}

	{
		static FName FuncName = FName(TEXT("K2_ApplyAbilityRequirements"));
		UFunction* BlueprintFunction = GetClass()->FindFunctionByName(FuncName);
		bHasBlueprintApplyAbilityRequirements = IsFunctionImplementedInBlueprint(BlueprintFunction);
	}
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

bool UGameplayAbility::TryCommitActivateAbility(const FGameplayAbilityActivationData& ActivationData, FActiveGameplayAbility& ActiveGameplayAbility)
{
	UGameplaySystemComponent* ActivatorComponent = GetOwningComponent();
	
	check(ActivatorComponent);

	if (!TryCheckAbilityRequirements(ActivationData))
	{
		return false;
	}

	bool bSuccess = true;

	bSuccess |= TryApplyAbilityRequirements(ActivationData);

	if(bSuccess)
	{
		TryActivateAbility(ActivationData, ActiveGameplayAbility);

		GA_LOG(Log, TEXT("GameplayAbility: %s activated."), *GetDisplayName());
	}
	else
	{
		GA_LOG(Error, TEXT("GameplayAbility: Ability not activated successfully, despite passing requirement check."));
		return false;
	}

	return true;
}

bool UGameplayAbility::TryCheckAbilityRequirements(const FGameplayAbilityActivationData& ActivationData) const
{
	UGameplaySystemComponent* ActivatorComponent = GetOwningComponent_Checked();

	if (ActivatorComponent->HasCooldown(GetClass()))
	{
		GA_LOG(Log, TEXT("GameplayAbility: %s not activated due to cooldown."), *GetDisplayName());
		return false;
	}
	
	// Only an error if it is already active while not having a cooldown
	if (bIsActive)
	{
		GA_LOG(Error, TEXT("GameplayAbility: %s tried to activate while already active."), *GetDisplayName());
		return false;
	}

	if (ActivatorComponent->GetGameplayTagSystem()->HasAnyTag(ActivationBlockedTags))
	{
		GA_LOG(Log, TEXT("GameplayAbility: %s not activated due to GameplaySystem's blocking tags."), *GetDisplayName());
		return false;
	}

	if (AbilityTags.HasAny(ActivatorComponent->GetBlockingAbilityTags()))
	{
		GA_LOG(Log, TEXT("GameplayAbility: %s not activated due to Ability's blocking tags."), *GetDisplayName());
		return false;
	}

	// Native implementation check
	if (!CheckAbilityRequirements(ActivationData))
	{
		return false;
	}

	// Blueprint implementation check
	if (bHasBlueprintCheckAbilityRequirements)
	{
		if (!K2_CheckAbilityRequirements(ActivationData))
		{
			return false;
		}
	}

	return true;
}

bool UGameplayAbility::TryApplyAbilityRequirements(const FGameplayAbilityActivationData& ActivationData)
{
	UGameplaySystemComponent* ActivatorComponent = GetOwningComponent_Checked();
	ActivatorComponent->ApplyBlockingAndCancellingTags(BlockAbilitiesWithTag, CancelAbilitiesWithTag, this);

	// Native implementation check
	if (!ApplyAbilityRequirements(ActivationData))
	{
		return false;
	}

	// Blueprint implementation check
	if (bHasBlueprintApplyAbilityRequirements)
	{
		if (!K2_ApplyAbilityRequirements(ActivationData))
		{
			return false;
		}
	}

	return true;
}

void UGameplayAbility::TryActivateAbility(const FGameplayAbilityActivationData& ActivationData, FActiveGameplayAbility& OutActiveGameplayAbility)
{
	if (!IsStaticInstance())
	{
		bHasAppliedAbilityEndedModifiers = false;
		bIsActive = true;
		bHasCancelled = false;
		bHasEnded = false;
	}

	OutActiveGameplayAbility.bHasActivated = true;

	ActivateAbility(ActivationData, OutActiveGameplayAbility);
	K2_ActivateAbility(ActivationData, OutActiveGameplayAbility);
}

bool UGameplayAbility::TryEndAbility()
{
	if (IsStaticInstance())
	{
		GA_LOG(Warning, TEXT("GameplayAbility: Tried to end Ability %s that has a EInstancingPolicy::NoLifetime. Static Abilities can not be ended."), *GetDisplayName());
		return false;
	}

	if (bHasCancelled)
	{
		// We might have been cancelled before ending naturally.
		return false;
	}

	if (bHasEnded)
	{
		GA_LOG(Error, TEXT("GameplayAbility: Attempting to end an already ended ability!"));
		return false;
	}

	if (!bIsActive)
	{
		GA_LOG(Warning, TEXT("GameplayAbility: Attempting to end an ability that has not activated yet!"));
		return false;
	}

	bIsActive = false;
	bHasEnded = true;

	GA_LOG(Log, TEXT("Ability: %s was ended."), *GetDisplayName());

	UGameplaySystemComponent* GameplaySystem = GetOwningComponent_Checked();
	GameplaySystem->InformAbilityEnded(this);

	TryApplyAbilityEndedModifiers();

	EndAbility();
	K2_EndAbility();

	return true;
}

bool UGameplayAbility::TryCancelAbility(bool bIsAuthoritative)
{
	if (IsStaticInstance())
	{
		GA_LOG(Warning, TEXT("GameplayAbility: Tried to cancel Ability %s that has a EInstancingPolicy::NoLifetime. Static Abilities are not cancellable."), *GetDisplayName());
		return false;
	}

	if (bHasCancelled || bHasEnded)
	{
		GA_LOG(Error, TEXT("GameplayAbility: Attempting to cancel an already cancelled or ended ability!"));
		return false;
	}

	// Not allowed to cancel it
	if (!bIsAuthoritative && !IsCancellable())
	{
		return false;
	}

	bIsActive = false;
	bHasCancelled = true;
	
	GA_LOG(Log, TEXT("Ability: %s was cancelled."), *GetDisplayName());

	UGameplaySystemComponent* GameplaySystem = GetOwningComponent_Checked();
	GameplaySystem->InformAbilityEnded(this);

	TryApplyAbilityEndedModifiers();

	if (bRemoveCooldownWhenCancelled)
	{
		FActiveGameplayAbility* ActiveAbility = GameplaySystem->GetActiveAbilityFromInstance_Ptr(this);
		check(ActiveAbility);

		ActiveAbility->SetCooldown(0.0f);
	}

	CancelAbility();
	K2_CancelAbility();

	return true;
}

bool UGameplayAbility::TryApplyAbilityEndedModifiers()
{
	if (bHasAppliedAbilityEndedModifiers)
	{
		return false;
	}

	bHasAppliedAbilityEndedModifiers = true;

	UGameplaySystemComponent* ActivatorComponent = GetOwningComponent_Checked();
	ActivatorComponent->RemoveBlockingTags(BlockAbilitiesWithTag);

	ApplyAbilityEndedModifiers();
	K2_ApplyAbilityEndedModifiers();

	return true;
}

bool UGameplayAbility::TryRemoveAbilityEndedModifiers()
{
	if (!bHasAppliedAbilityEndedModifiers)
	{
		return false;
	}

	bHasAppliedAbilityEndedModifiers = false;

	UGameplaySystemComponent* ActivatorComponent = GetOwningComponent_Checked();
	ActivatorComponent->ApplyBlockingAndCancellingTags(BlockAbilitiesWithTag, FGameplayTagContainer(), this);

	RemoveAbilityEndedModifiers();

	return true;
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

bool UGameplayAbility::CheckAbilityRequirements(const FGameplayAbilityActivationData& ActivationData) const
{
	return true;
}

bool UGameplayAbility::ApplyAbilityRequirements(const FGameplayAbilityActivationData& ActivationData)
{
	return true;
}

void UGameplayAbility::ActivateAbility(const FGameplayAbilityActivationData& ActivationData, FActiveGameplayAbility& OutActiveGameplayAbility)
{

}

void UGameplayAbility::EndAbility()
{

}

void UGameplayAbility::CancelAbility()
{

}

void UGameplayAbility::ApplyAbilityEndedModifiers()
{
	// Any modifiers that might affect another abilities activation that will trigger on the abilities end or cancellation should be applied here instead.
	// This could be modifiers such as attribute changes, GameplayEffects, GameplayTags, blocking tags, etc.
}

void UGameplayAbility::RemoveAbilityEndedModifiers()
{
	// Any modifiers that might affect another abilities activation that will trigger on the abilities end or cancellation should be applied here instead.
	// This could be modifiers such as attribute changes, GameplayEffects, GameplayTags, blocking tags, etc.
}

bool UGameplayAbility::IsStaticInstance() const
{
	return InstancingPolicy == EInstancingPolicy::EIP_NoLifetime;
}

bool UGameplayAbility::IsAnimatingAbility() const
{
	UGameplaySystemComponent* GameplaySystem = GetOwningComponent_Checked();
	return GameplaySystem->GetAnimatingAbility() == this;
}

void UGameplayAbility::CurateProperties()
{
	if (bForceSameDurationAndCooldown)
	{
		Cooldown = Duration;
	}
}