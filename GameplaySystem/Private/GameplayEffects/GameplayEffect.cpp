// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


#include "GameplayEffect.h"

#include "AttributeEffect.h"
#include "GameplaySystemComponent.h"

#include "DevelopmentTypes.h"

const float FGameplayEffectConstants::INFINITE_DURATION = 0.0f;
const float FGameplayEffectConstants::NO_PERIOD = 0.0f;

void UGameplayEffectExecutor::OnGameplayEffectApplied(FGameplayEffectExecutorParams Params, const TArray<FAttributeEffect>& EffectsToApply, const FGameplayTagModifierContainer& TagModifiers) const
{
	PerformDefaultApply(Params, EffectsToApply, TagModifiers);
}

void UGameplayEffectExecutor::OnGameplayEffectRemoved(FGameplayEffectExecutorParams Params) const
{
	PerformDefaultRemove(Params);
}

void UGameplayEffectExecutor::OnGameplayEffectReapplied(FGameplayEffectExecutorParams Params, const TArray<FAttributeEffect>& EffectsToReapply) const
{
	PerformDefaultReapply(Params, EffectsToReapply);
}

void UGameplayEffectExecutor::PerformDefaultApply(FGameplayEffectExecutorParams Params, const TArray<FAttributeEffect>& EffectsToApply, const FGameplayTagModifierContainer& TagModifiers) const
{
	Apply_Internal(Params, EffectsToApply, TagModifiers);
}

void UGameplayEffectExecutor::PerformDefaultRemove(FGameplayEffectExecutorParams Params) const
{
	FActiveGameplayEffect* ActiveGameplayEffect = Params.ActiveGameplayEffect;
	UGameplaySystemComponent* GameplaySystem = Params.GameplaySystem;

	ActiveGameplayEffect->RemoveAppliedModifiers(GameplaySystem, GameplaySystem->GetOwner());
}

void UGameplayEffectExecutor::PerformDefaultReapply(FGameplayEffectExecutorParams Params, const TArray<FAttributeEffect>& EffectsToReapply) const
{
	const UGameplayEffect* Effect = Params.GameplayEffect;
	UGameplaySystemComponent* GameplaySystem = Params.GameplaySystem;

	for (const FAttributeEffect& AttributeEffect : EffectsToReapply)
	{
		// Force non-reversible application since we can't track and undo our reapplications anyway
		GameplaySystem->ApplyAttributeEffect(AttributeEffect, EDurationType::EDT_Instant);
	}
}

inline static void Predicate_Apply(const FAttributeEffect& AttributeEffect, EDurationType DurationType, UGameplaySystemComponent* GameplaySystem)
{
	GameplaySystem->ApplyAttributeEffect(AttributeEffect, DurationType);
};

void UGameplayEffectExecutor::Apply_Internal(FGameplayEffectExecutorParams Params, const TArray<FAttributeEffect>& EffectsToApply, const FGameplayTagModifierContainer& TagModifiers) const
{
	const UGameplayEffect* Effect = Params.GameplayEffect;
	UGameplaySystemComponent* GameplaySystem = Params.GameplaySystem;

	const bool bApplyEffectNow = Effect->PeriodType == EPeriodApplicationType::EPAT_ExecuteOnApplication;
	if (bApplyEffectNow)
	{
		for (const FAttributeEffect& AttributeEffect : EffectsToApply)
		{
			GameplaySystem->ApplyAttributeEffect(AttributeEffect, Effect->DurationType);
		}

		Effect->TagModifierContainer.Apply(GameplaySystem->GetGameplayTagSystem());
	}
}

UGameplayEffect::UGameplayEffect()
{
	GENERATE_IF_INVALID(Id);

	FillEmptyClasses();
}

#if WITH_EDITOR
void UGameplayEffect::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	GENERATE_IF_INVALID(Id);

	FillEmptyClasses();
}
#endif

void UGameplayEffect::PostEditImport()
{
	Super::PostEditImport();

	Id.Reset();
}

void UGameplayEffect::PostDuplicate(bool bDuplicateForPIE)
{
	Super::PostDuplicate(bDuplicateForPIE);

	if (!bDuplicateForPIE)
	{
		Id.Reset();
	}
}

bool UGameplayEffect::ApplyGameplayEffect(UGameplaySystemComponent* GameplaySystem, AActor* Actor, FActiveGameplayEffect& ActiveGameplayEffect, FGameplayEffectHandle& OutHandle) const
{
	const float RandomPercentage = FMath::FRandRange(0.0f, 100.0f);

	// Did not pass the chance to apply
	if (RandomPercentage >= ChanceToApply)
	{
		return false;
	}
	
	const bool bPassedCustomRequirements = GetActivationRequirementsClass()->CanApply(this, GameplaySystem);
	if (!bPassedCustomRequirements)
	{
		return false;
	}

	// Check if the Effect is unique, and if it is, check if it is already applied
	if (bIsUnique)
	{
		FGameplayEffectHandle Handle;
		const bool bAlreadyExists = GameplaySystem->HasGameplayEffectOfInstance(this, Handle);
		
		if (bAlreadyExists)
		{
			// Replace the existing effect with a new one
			if (bOverwriteOnUnique)
			{
				GameplaySystem->RemoveGameplayEffectByHandle(Handle);
			}
			else // This effect is already applied, do not overwrite it
			{
				return false;
			}
		}
	}

	// We do not modify on application, so we only register and exit here 
	if (PeriodType != EPeriodApplicationType::EPAT_ExecuteOnApplication)
	{
		FGameplayEffectHandle GeneratedHandle = {};

		OutHandle = GeneratedHandle;
		GameplaySystem->RegisterGameplayEffect(GeneratedHandle, ActiveGameplayEffect);
		return true;
	}

	const UGameplayEffectExecutor* GameplayEffectExecutor = GetExecutorClass();
	FGameplayEffectExecutorParams Params = { this, &ActiveGameplayEffect, GameplaySystem };
	GameplayEffectExecutor->OnGameplayEffectApplied(Params, AttributeEffects, TagModifierContainer);

	// Remove any matching GameplayEffects
	TArray<FGameplayEffectHandle> HandlesToRemove;
	GameplaySystem->GetMatchingGameplayEffects(RemoveMatchingGameplayEffects, HandlesToRemove);
	const int RemovedCount = GameplaySystem->RemoveGameplayEffectsByHandles(HandlesToRemove);

	if (DurationType != EDurationType::EDT_Instant)
	{
		FGameplayEffectHandle GeneratedHandle = {};

		OutHandle = GeneratedHandle;
		GameplaySystem->RegisterGameplayEffect(GeneratedHandle, ActiveGameplayEffect);
	}

	return true;
}

bool UGameplayEffect::RemoveGameplayEffect(UGameplaySystemComponent* GameplaySystem, AActor* Actor, FActiveGameplayEffect& ActiveGameplayEffect, const FGameplayEffectHandle& Handle) const
{
	check(GameplaySystem);

	const UGameplayEffectExecutor* GameplayEffectExecutor = GetExecutorClass();
	FGameplayEffectExecutorParams Params = { this, &ActiveGameplayEffect, GameplaySystem };
	GameplayEffectExecutor->OnGameplayEffectRemoved(Params);

	return GameplaySystem->RemoveGameplayEffect_Internal(Handle);
}

const UGameplayEffectExecutor* UGameplayEffect::GetExecutorClass() const
{
	check(ExecutorClass);

	return ExecutorClass->GetDefaultObject<UGameplayEffectExecutor>();
}

const UGameplayEffectApplicationRequirements* UGameplayEffect::GetActivationRequirementsClass() const
{
	check(ActivationRequirementsClass);

	return ActivationRequirementsClass->GetDefaultObject<UGameplayEffectApplicationRequirements>();
}

void UGameplayEffect::FillEmptyClasses()
{
	// Ensure we always have classes to operate on.
	if (!ExecutorClass)
	{
		ExecutorClass = UGameplayEffectExecutor::StaticClass();
		check(ExecutorClass)
	}

	if (!ActivationRequirementsClass)
	{
		ActivationRequirementsClass = UGameplayEffectApplicationRequirements::StaticClass();
		check(ActivationRequirementsClass)
	}
}

FActiveGameplayEffect::FActiveGameplayEffect()
{
	this->GameplayEffectDef = UGameplayEffect::StaticClass();
}

FActiveGameplayEffect::FActiveGameplayEffect(const UGameplayEffect* Def)
{
	check(Def);

	AttributeEffects = Def->AttributeEffects;
	GameplayEffectDef = Def->GetClass();
	Id = Def->Id;
}

int FActiveGameplayEffect::GetAttributeEffect(FAttributeEffect Effect) const
{
	const int Index = AttributeEffects.Find(Effect);

	return Index;
}

UGameplayEffect* FActiveGameplayEffect::GetDefinition() const
{
	return GameplayEffectDef ? GameplayEffectDef->GetDefaultObject<UGameplayEffect>() : nullptr;
}

void FActiveGameplayEffect::Tick(float DeltaTime, UGameplaySystemComponent* GameplaySystem)
{
	TimeSinceLastApplication += DeltaTime;
	Lifetime += DeltaTime;

	const UGameplayEffect* GameplayEffect = GetDefinition();

	while (IsPeriodPassed())
	{
		const UGameplayEffectExecutor* GameplayEffectExecutor = GameplayEffect->GetExecutorClass();
		FGameplayEffectExecutorParams Params = { GameplayEffect, this, GameplaySystem };
		GameplayEffectExecutor->OnGameplayEffectReapplied(Params, AttributeEffects);

		// Compensate for missed applications because of large DeltaTime or small PeriodLength
		if (TimeSinceLastApplication > GameplayEffect->PeriodLength)
		{
			TimeSinceLastApplication -= GameplayEffect->PeriodLength;
		}
		else
		{
			TimeSinceLastApplication = 0.0f;
		}
	}
}

void FActiveGameplayEffect::RemoveAppliedModifiers(UGameplaySystemComponent* GameplaySystem, AActor* Actor)
{
	const UGameplayEffect* GameplayEffect = GetDefinition();

	// Apply AttributeEffects if configured to be done on removal
	if (GameplayEffect->PeriodType == EPeriodApplicationType::EPAT_ExecuteOnRemoval)
	{
		for (FAttributeEffect& AttributeEffect : AttributeEffects)
		{
			GameplaySystem->ApplyAttributeEffect(AttributeEffect, GameplayEffect->DurationType);
		}
	}

	// First, remove any applied Attribute Effects
	for (FAttributeEffect& AttributeEffect : AttributeEffects)
	{
		GameplaySystem->RemoveAttributeEffect(AttributeEffect);
	}

	// Remove the GameplayTag modifiers
	GameplayEffect->TagModifierContainer.ReverseApply(GameplaySystem->GetGameplayTagSystem());
}

volatile bool FActiveGameplayEffect::IsPeriodPassed() const
{
	const UGameplayEffect* GameplayEffect = GetDefinition();

	const bool bHasPassedPeriod = TimeSinceLastApplication >= GameplayEffect->PeriodLength;
	const bool bHasPeriod = GameplayEffect->PeriodLength != FGameplayEffectConstants::NO_PERIOD;
	return bHasPeriod ? bHasPassedPeriod : false;
}

bool FActiveGameplayEffect::IsExpired() const
{
	const UGameplayEffect* GameplayEffect = GetDefinition();

	// Will never expire
	if (GameplayEffect->DurationType == EDurationType::EDT_Infinite)
	{
		return false;
	}

	return Lifetime >= GameplayEffect->Duration;
}

float FActiveGameplayEffect::GetRemainingDuration() const
{
	const UGameplayEffect* GameplayEffect = GetDefinition();

	return GameplayEffect->Duration == FGameplayEffectConstants::INFINITE_DURATION ? 0.0f : GameplayEffect->Duration - Lifetime;
}

FString FActiveGameplayEffect::ToString() const
{
	const UGameplayEffect* GameplayEffect = GetDefinition();

	FString Output = GameplayEffect->Name + TEXT("\n");
	Output += FString::Printf(TEXT("Duration: %.1f"), GetRemainingDuration()) + TEXT("\n");
	Output += FString::Printf(TEXT("Period: %.1f"), GameplayEffect->PeriodLength) + TEXT(" | ") + FString::Printf(TEXT("%.1f"), TimeSinceLastApplication) + TEXT("\n");

	for (const FAttributeEffect& Effect : AttributeEffects)
	{
		Output += TEXT("   ") + Effect.ToString() + TEXT("\n");
	}

	return Output;
}
