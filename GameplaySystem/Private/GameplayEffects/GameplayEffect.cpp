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

bool UGameplayEffect::ApplyGameplayEffect(UGameplaySystemComponent* GameplaySystem, FActiveGameplayEffect& ActiveGameplayEffect, FGameplayEffectHandle& OutHandle) const
{
	check(GameplaySystem);

	ensure(not ActiveGameplayEffect.IsPendingRemove());

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

	// If we are a stacking effect and the effect is already applied, add a stack to it instead.
	if (IsStackingEffect())
	{
		FGameplayEffectHandle ExistingEffect = GameplaySystem->GetGameplayEffectByClass(GetClass());

		if (ExistingEffect.IsValid())
		{
			FGameplayEffectStackModifier StackModifier = FGameplayEffectStackModifier(false /* bAddGameplayEffectIfNotApplied */, 1, 0.0f);

			const bool bApplied = GameplaySystem->ApplyGameplayEffectStackModifier(GetClass(), StackModifier);
			ensure(bApplied);
			return true;
		}
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

	// Remove any matching GameplayEffects
	TArray<FGameplayEffectHandle> HandlesToRemove;
	GameplaySystem->GetMatchingGameplayEffects(RemoveMatchingGameplayEffects, HandlesToRemove);
	const int RemovedCount = GameplaySystem->RemoveGameplayEffectsByHandles(HandlesToRemove);

	if (DurationType == EDurationType::EDT_Instant)
	{
		return true;
	}

	OutHandle = FGameplayEffectHandle::CreateNew();
	GameplaySystem->RegisterGameplayEffect(OutHandle, ActiveGameplayEffect);

	return true;
}

bool UGameplayEffect::RemoveGameplayEffect(UGameplaySystemComponent* GameplaySystem, FActiveGameplayEffect& ActiveGameplayEffect, const FGameplayEffectHandle& Handle) const
{
	check(GameplaySystem);

	ensure(not ActiveGameplayEffect.IsPendingRemove());

	ActiveGameplayEffect.bIsPendingRemove = true;

	FGameplayEffectExecutorParams Params = { this, &ActiveGameplayEffect, GameplaySystem };
	FireExecutorPipeline(Params, EGameplayEffectStage::EGES_Remove);

	return GameplaySystem->RemoveGameplayEffect_Internal(Handle);
}

void UGameplayEffect::ApplyGameplayEffectStackModifier(UGameplaySystemComponent* GameplaySystem, FActiveGameplayEffect& ActiveGameplayEffect, const FGameplayEffectStackModifier& Modifier) const
{
	check(GameplaySystem);

	FGameplayEffectStackerParams Params = { this, &ActiveGameplayEffect, GameplaySystem };
	FireStackerPipeline(Params, Modifier);
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

void UGameplayEffect::FireStackerPipeline(const FGameplayEffectStackerParams& Params, const FGameplayEffectStackModifier& StackModifier) const
{
	FGameplayEffectStackModifier MutableStackModifier = StackModifier;

	for (auto& Module : StackerModules)
	{
		if (!Module)
		{
			GS_LOG(Warning, TEXT("Invalid Condition Module found in GameplayEffect: %s"), *Name);
			continue;
		}

		Module->PreApplyStackModifier(Params, MutableStackModifier);
	}

	UGameplayEffectStacker::ApplyStackModifier(Params, MutableStackModifier);

	for (auto& Module : StackerModules)
	{
		if (!Module)
		{
			GS_LOG(Warning, TEXT("Invalid Condition Module found in GameplayEffect: %s"), *Name);
			continue;
		}

		Module->PostApplyStackModifier(Params, MutableStackModifier);
	}
}

bool UGameplayEffect::IsAppliedOnTick() const
{
	return PeriodLength != FGameplayEffectConstants::NO_PERIOD;
}

bool UGameplayEffect::IsStackingEffect() const
{
	return StackingPolicy == EStackingPolicy::ESP_CanStack;
}

bool UGameplayEffect::HasDuration() const
{
	return DurationType == EDurationType::EDT_HasDuration && Duration > 0.0f;
}

bool UGameplayEffect::HasStackProgressDuration() const
{
	return StackProgressDurationType == EStackProgressDurationType::ESP_Duration && StackProgressDuration > 0.0f;
}

bool UGameplayEffect::HasPeriod() const
{
	return PeriodLength > 0.0f;
}

bool UGameplayEffect::HasMaxStacks() const
{
	return MaxStacks != FGameplayEffectConstants::NO_MAX_STACKS;
}

float UGameplayEffect::EvaluateScalingCurve(float InTime) const
{
	float TimeScale = 1.0f;

	if (ScalingCurve)
	{
		// Only stretch ScalingCurve if we have a finite duration.
		if (DurationType == EDurationType::EDT_HasDuration && Duration != FGameplayEffectConstants::NO_DURATION)
		{
			TimeScale = ScalingCurve->FloatCurve.GetLastKey().Time / Duration;
		}

		const float EvaluatedTime = InTime * TimeScale;
		return ScalingCurve->GetFloatValue(EvaluatedTime);
	}

	return 1.0f;
}

FActiveGameplayEffect::FActiveGameplayEffect()
{
	Definition = UGameplayEffect::StaticClass();
}

FActiveGameplayEffect::FActiveGameplayEffect(const UGameplayEffect* InDefinition)
{
	check(InDefinition);

	AttributeEffects = InDefinition->AttributeEffects;
	Definition = InDefinition->GetClass();
	Id = InDefinition->Id;
	StackProgressLimit = InDefinition->StackProgressLimit;
}

FActiveGameplayEffect::FActiveGameplayEffect(const TSubclassOf<UGameplayEffect> InDefinition, AActor* InInstigator)
	: FActiveGameplayEffect(InDefinition ? InDefinition->GetDefaultObject<UGameplayEffect>() : UGameplayEffect::StaticClass()->GetDefaultObject<UGameplayEffect>())
{
	if (InInstigator)
	{
		Instigator = MakeWeakObjectPtr(InInstigator);
	}
}

int FActiveGameplayEffect::GetAttributeEffect(FAttributeEffect Effect) const
{
	const int Index = AttributeEffects.Find(Effect);

	return Index;
}

bool FActiveGameplayEffect::IsFullyApplied() const
{
	if (GetDefinition()->IsStackingEffect())
	{
		return AppliedStacks >= 1;
	}

	return true;
}

bool FActiveGameplayEffect::IsAtFullStacks() const
{
	if (not GetDefinition()->IsStackingEffect())
	{
		return true;
	}

	return AppliedStacks >= GetDefinition()->MaxStacks;
}

bool FActiveGameplayEffect::IsPendingRemove() const
{
	return bIsPendingRemove;
}

// Only applies when the GameplayEffect is stacking, with StackProgressDuration enabled and the GameplayEffect is not fully applied.
bool FActiveGameplayEffect::IsStackProgressDurationInControl() const
{
	return GetDefinition()->IsStackingEffect() && GetDefinition()->HasStackProgressDuration() && not IsFullyApplied();
}

bool FActiveGameplayEffect::IsRemoveExternal() const
{
	if (not GetDefinition()->HasDuration())
	{
		return true;
	}

	return bIsPendingRemove && not IsExpired();
}

float FActiveGameplayEffect::GetRemainingStackProgressDuration() const
{
	const UGameplayEffect* GameplayEffect = GetDefinition();

	return GameplayEffect->StackProgressDuration - StackProgressDurationCounter;
}

UGameplayEffect* FActiveGameplayEffect::GetDefinition() const
{
	return Definition ? Definition->GetDefaultObject<UGameplayEffect>() : UGameplayEffect::StaticClass()->GetDefaultObject<UGameplayEffect>();
}

void FActiveGameplayEffect::Tick(float DeltaTime, UGameplaySystemComponent* GameplaySystem)
{
	const UGameplayEffect* GameplayEffect = GetDefinition();
	const float PreviousScalar = GameplayEffect->EvaluateScalingCurve(DurationCounter);

	Lifetime += DeltaTime;

	if (GameplayEffect->HasPeriod())
	{
		PeriodCounter += DeltaTime;
	}

	if (not IsStackProgressDurationInControl())
	{
		DurationCounter += DeltaTime;
	}
	else if(GetDefinition()->IsStackingEffect())
	{
		StackProgressDurationCounter += DeltaTime;
	}

	if (IsExpired())
	{
		if (GameplayEffect->StackingExpirationRule == EStackingExpirationRule::ESP_RemoveSingleStack)
		{
			// Before the GameplayEffect is checked for removal, remove a stack instead and reset the duration counter
			if (AppliedStacks > 0)
			{
				ApplyStack(-1, GameplaySystem);
				DurationCounter = 0.0f;
			}
		}
	}

	const float CurrentScalar = GameplayEffect->EvaluateScalingCurve(DurationCounter);
	if (PreviousScalar != CurrentScalar)
	{
		ScaleModifiers(PreviousScalar, CurrentScalar, GameplaySystem);
	}

	while (IsPeriodPassed())
	{
		if (IsFullyApplied())
		{
			FGameplayEffectConditionParams ConditionParams = { GameplayEffect, GameplaySystem };
			if (GameplayEffect->FireConditionPipeline(ConditionParams, EGameplayEffectStage::EGES_Reapply))
			{
				FGameplayEffectExecutorParams ExecutorParams = { GameplayEffect, this, GameplaySystem };
				GameplayEffect->FireExecutorPipeline(ExecutorParams, EGameplayEffectStage::EGES_Reapply);
			}
		}

		// Compensate for missed applications because of large DeltaTime or small PeriodLength
		if (PeriodCounter > GameplayEffect->PeriodLength)
		{
			PeriodCounter -= GameplayEffect->PeriodLength;
		}
		else
		{
			PeriodCounter = 0.0f;
		}
	}
}

void FActiveGameplayEffect::ScaleModifiers(float LastScalar, float NewScalar, UGameplaySystemComponent* GameplaySystem)
{
	if (LastScalar == NewScalar)
	{
		return;
	}

	TArray<EAttributeType> AffectedAttributes;
	for (auto& AttributeEffect : AttributeEffects)
	{
		AttributeEffect.Value /= LastScalar;
		AttributeEffect.Value *= NewScalar;
		AffectedAttributes.AddUnique(AttributeEffect.Attribute);
	}

	GameplaySystem->MarkAttributesDirty(AffectedAttributes);
}

void FActiveGameplayEffect::ApplyStackProgress(float AddedStackProgress, UGameplaySystemComponent* GameplaySystem)
{
	if (not GetDefinition()->IsStackingEffect())
	{
		return;
	}

	const float PreviousStackProgress = StackProgress;

	StackProgress += AddedStackProgress;

	int32 StackDelta = 0;

	// Add stacks when above progress limit
	while (StackProgress >= StackProgressLimit)
	{
		StackProgress -= StackProgressLimit;

		StackDelta++;
	}

	// Take off stacks for negative progress values
	while (StackProgress < 0.0f)
	{
		StackProgress += StackProgressLimit;

		StackDelta--;
	}

	ApplyStack(StackDelta, GameplaySystem);

	// Can't gather progress past the limit
	if (IsAtFullStacks())
	{
		StackProgress = 0.0f;
	}

	// Reset StackProgress-based duration
	if (GetDefinition()->HasStackProgressDuration())
	{
		StackProgressDurationCounter = 0.0f;
	}

	// StackProgress was modified
	if (StackProgress != PreviousStackProgress)
	{
		OnStackProgressChangedSignature.Broadcast(StackProgress);
	}
}

void FActiveGameplayEffect::ApplyStack(int32 AddedStacks, UGameplaySystemComponent* GameplaySystem)
{
	if (not GetDefinition()->IsStackingEffect())
	{
		return;
	}

	// Note: 0 stacks is treated as a scaling modifier of 1, since otherwise it makes any modifiers unrecoverable in subsequent modifications
	//		 We scale it back to it's original value by scaling by 1, and disable its application instead of scaling modifiers by 0.

	const int32 PreviousTrueStackCount = AppliedStacks;
	int32 PreviousStackCount = AppliedStacks;

	if (PreviousStackCount == 0)
	{
		PreviousStackCount = 1;
	}

	AppliedStacks += AddedStacks;

	if (GetDefinition()->HasMaxStacks())
	{
		const int32 MaxStacks = GetDefinition()->MaxStacks;
		if (AppliedStacks > MaxStacks)
		{
			AppliedStacks = MaxStacks;
			StackProgress = 0.0f;
		}
	}

	if (AppliedStacks < 0)
	{
		AppliedStacks = 0;
	}

	const EStackingDurationRule DurationRule = GetDefinition()->StackingDurationRule;
	const EStackingPeriodRule PeriodRule = GetDefinition()->StackingPeriodRule;

	// We added stacks
	if (AppliedStacks > 0)
	{
		if (GetDefinition()->HasDuration())
		{
			if (DurationRule == EStackingDurationRule::ESD_AddResetsDuration || DurationRule == EStackingDurationRule::ESD_AddAndRemoveResetsDuration)
			{
				DurationCounter = 0.0f;
			}
		}

		if (GetDefinition()->HasPeriod())
		{
			if (PeriodRule == EStackingPeriodRule::ESP_AddResetsPeriod || PeriodRule == EStackingPeriodRule::ESP_AddAndRemoveResetsPeriod)
			{
				PeriodCounter = 0.0f;
			}
		}
	}
	else // We removed stacks
	{
		if (GetDefinition()->HasDuration())
		{
			if (DurationRule == EStackingDurationRule::ESD_RemoveResetsDuration || DurationRule == EStackingDurationRule::ESD_AddAndRemoveResetsDuration)
			{
				DurationCounter = 0.0f;
			}
		}

		if (GetDefinition()->HasPeriod())
		{
			if (PeriodRule == EStackingPeriodRule::ESP_RemoveResetsPeriod || PeriodRule == EStackingPeriodRule::ESP_AddAndRemoveResetsPeriod)
			{
				PeriodCounter = 0.0f;
			}
		}
	}

	if (AppliedStacks == 0)
	{
		ScaleModifiers(PreviousStackCount, 1, GameplaySystem);
	}
	else
	{
		ScaleModifiers(PreviousStackCount, AppliedStacks, GameplaySystem);
	}

	// Reset StackProgress-based duration
	if (GetDefinition()->HasStackProgressDuration())
	{
		StackProgressDurationCounter = 0.0f;
	}

	if (AppliedStacks != PreviousTrueStackCount)
	{
		GameplaySystem->MarkAttributesDirty(*this);

		OnStackChangedSignature.Broadcast(AppliedStacks);
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

void FActiveGameplayEffect::SetStackProgressLimit(float Limit, UGameplaySystemComponent* GameplaySystem)
{
	StackProgressLimit = Limit;

	// Check if StackProgress is past the new limit
	// TODO: Make this call more explicit, rather than passing a dummy value to run the required logic?
	ApplyStackProgress(0.0f, GameplaySystem);
}

bool FActiveGameplayEffect::IsPeriodPassed() const
{
	const UGameplayEffect* GameplayEffect = GetDefinition();

	const bool bHasPassedPeriod = PeriodCounter >= GameplayEffect->PeriodLength;
	const bool bHasPeriod = GameplayEffect->PeriodLength != FGameplayEffectConstants::NO_PERIOD;
	return bHasPeriod ? bHasPassedPeriod : false;
}

bool FActiveGameplayEffect::IsExpired() const
{
	const UGameplayEffect* GameplayEffect = GetDefinition();

	if (IsStackProgressDurationInControl())
	{
		if (StackProgressDurationCounter >= GameplayEffect->StackProgressDuration)
		{
			return true;
		}
		else
		{
			return false;
		}	
	}

	// Will never expire
	if (GameplayEffect->DurationType == EDurationType::EDT_Infinite)
	{
		return false;
	}

	return DurationCounter >= GameplayEffect->Duration;
}

float FActiveGameplayEffect::GetRemainingDuration() const
{
	const UGameplayEffect* GameplayEffect = GetDefinition();

	return GameplayEffect->Duration == FGameplayEffectConstants::INFINITE_DURATION ? 0.0f : GameplayEffect->Duration - DurationCounter;
}

FString FActiveGameplayEffect::ToString() const
{
	const UGameplayEffect* GameplayEffect = GetDefinition();

	FString Output = GameplayEffect->Name + ENDL;
	Output += FString::Printf(TEXT("Duration: %.1f"), GetRemainingDuration()) + ENDL;
	Output += FString::Printf(TEXT("Period: %.1f | %.1f"), GameplayEffect->PeriodLength, PeriodCounter) + ENDL;

	if (GameplayEffect->IsStackingEffect())
	{
		Output += FString::Printf(TEXT("Stacks: %d   | %.1f of %.1f"), AppliedStacks, StackProgress, StackProgressLimit) + ENDL;
	}

	for (const FAttributeEffect& Effect : AttributeEffects)
	{
		Output += TEXT("   ") + Effect.ToString() + ENDL;
	}

	return Output;
}

FGameplayEffectHandle FGameplayEffectHandle::CreateNew()
{
	FGameplayEffectHandle Handle;
	Handle.bWasInitialized = true;

	return Handle;
}

void FGameplayEffectHandle::Regenerate()
{
	Id = FGuidTag();

	bWasInitialized = true;
}

bool FGameplayEffectHandle::IsValid() const
{
	return bWasInitialized && Id.IsValid();
}


uint32 GetTypeHash(const FGameplayEffectHandle& InHandle)
{
	return GetTypeHash(InHandle.Id);
}

