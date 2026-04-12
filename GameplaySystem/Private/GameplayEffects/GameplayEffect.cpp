// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayEffect.h"

#include "AttributeEffect.h"
#include "GameplaySystemComponent.h"

#include "DevelopmentTypes.h"

UGameplayEffect::UGameplayEffect()
{
	GENERATE_IF_INVALID(Id);
}

#if WITH_EDITOR
void UGameplayEffect::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	GENERATE_IF_INVALID(Id);
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
	
	FGameplayEffectConditionParams ConditionParams = { this, GameplaySystem };
	const bool bPassedConditions = FireConditionPipeline(ConditionParams, EGameplayEffectStage::EGES_Apply);
	if (!bPassedConditions)
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

	FGameplayEffectExecutorParams ExecutorParams = { this, &ActiveGameplayEffect, GameplaySystem };
	FireExecutorPipeline(ExecutorParams, EGameplayEffectStage::EGES_Apply);

	// We do not modify on application, so we only register and exit here 
	if (PeriodType != EPeriodApplicationType::EPAT_ExecuteOnApplication)
	{
		FGameplayEffectHandle GeneratedHandle = {};

		OutHandle = GeneratedHandle;
		GameplaySystem->RegisterGameplayEffect(GeneratedHandle, ActiveGameplayEffect);
		return true;
	}

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

	FGameplayEffectExecutorParams Params = { this, &ActiveGameplayEffect, GameplaySystem };
	FireExecutorPipeline(Params, EGameplayEffectStage::EGES_Remove);

	return GameplaySystem->RemoveGameplayEffect_Internal(Handle);
}

void UGameplayEffect::FireExecutorPipeline(const FGameplayEffectExecutorParams& Params, EGameplayEffectStage Stage) const
{
	const FGameplaySystemSnapshot Snapshot = Params.GameplaySystem->GetSnapshot();

	for (auto& Module : ExecutorModules)
	{
		if (!Module)
		{
			GS_LOG(Warning, TEXT("Invalid Executor Module found in GameplayEffect: %s"), *Name);
			continue;
		}

		switch (Stage)
		{
		case EGameplayEffectStage::EGES_Apply:
			Module->PreApply(Params);
			break;

		case EGameplayEffectStage::EGES_Reapply:
			Module->PreReapply(Params);
			break;

		case EGameplayEffectStage::EGES_Remove:
			Module->PreRemove(Params);
			break;
		}

	}

	switch (Stage)
	{
	case EGameplayEffectStage::EGES_Apply:
		UGameplayEffectExecutor::Apply(Params);
		break;

	case EGameplayEffectStage::EGES_Reapply:
		UGameplayEffectExecutor::Reapply(Params);
		break;

	case EGameplayEffectStage::EGES_Remove:
		UGameplayEffectExecutor::Remove(Params);
		break;
	}

	for (auto& Module : ExecutorModules)
	{
		if (!Module)
		{
			GS_LOG(Warning, TEXT("Invalid Executor Module found in GameplayEffect: %s"), *Name);
			continue;
		}

		switch (Stage)
		{
		case EGameplayEffectStage::EGES_Apply:
			Module->PostApply(Params, Snapshot);
			break;

		case EGameplayEffectStage::EGES_Reapply:
			Module->PostReapply(Params, Snapshot);
			break;

		case EGameplayEffectStage::EGES_Remove:
			Module->PostRemove(Params, Snapshot);
			break;
		}

	}
}

bool UGameplayEffect::FireConditionPipeline(const FGameplayEffectConditionParams& Params, EGameplayEffectStage Stage) const
{
	for (auto& Module : ConditionModules)
	{
		if (!Module)
		{
			GS_LOG(Warning, TEXT("Invalid Condition Module found in GameplayEffect: %s"), *Name);
			continue;
		}

		bool bResult = true;

		switch (Stage)
		{
		case EGameplayEffectStage::EGES_Apply:
			bResult = Module->CanApply(Params);
			break;

		case EGameplayEffectStage::EGES_Reapply:
			bResult = Module->CanReapply(Params);
			break;

		case EGameplayEffectStage::EGES_Remove:
			checkNoEntry(); // Not supported yet.
			break;
		}

		if (!bResult)
		{
			return false;
		}
	}

	return true;
}

bool UGameplayEffect::IsAppliedOnTick() const
{
	return PeriodLength != FGameplayEffectConstants::NO_PERIOD;
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
		FGameplayEffectConditionParams ConditionParams = { GameplayEffect, GameplaySystem };
		if (GameplayEffect->FireConditionPipeline(ConditionParams, EGameplayEffectStage::EGES_Reapply))
		{
			FGameplayEffectExecutorParams ExecutorParams = { GameplayEffect, this, GameplaySystem };
			GameplayEffect->FireExecutorPipeline(ExecutorParams, EGameplayEffectStage::EGES_Reapply);
		}

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

	if (GameplayEffect->bUndoModifiersOnRemoval)
	{
		for (FAttributeEffect& AttributeEffect : AttributeEffects)
		{
			GameplaySystem->RemoveAttributeEffect(AttributeEffect);
		}

		GameplayEffect->TagModifierContainer.ReverseApply(GameplaySystem->GetGameplayTagSystem());
	}
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
