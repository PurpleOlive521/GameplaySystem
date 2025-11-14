// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "GameplayEffect.h"

#include "AttributeEffect.h"
#include "GameplaySystemComponent.h"

#include "DevelopmentTypes.h"


const float FGameplayEffectConstants::INFINITE_DURATION = 0.0f;
const float FGameplayEffectConstants::NO_PERIOD = 0.0f;

FActiveGameplayEffect::FActiveGameplayEffect()
{
	UGameplayEffect::GenerateGUID(Id);

	this->GameplayEffectDef = UGameplayEffect::StaticClass();
}

FActiveGameplayEffect::FActiveGameplayEffect(const UGameplayEffect* Def)
{
	check(Def);

	this->Name = Def->Name;
	this->AttributeEffects = Def->AttributeEffects;
	this->DurationType = Def->DurationType;
	this->Duration = Def->Duration;
	this->ChanceToApply = Def->ChanceToApply;
	this->PeriodLength = Def->PeriodLength;
	this->PeriodType = Def->PeriodType;
	this->TagsOnEffect = Def->TagsOnEffect;
	this->bIsUnique = Def->bIsUnique;
	this->TagModifierContainer = Def->TagModifierContainer;

	this->Id = Def->Id;
	this->GameplayEffectDef = Def->GetClass();
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


void FActiveGameplayEffect::TickGameplayEffect(float DeltaTime)
{
	TimeSinceLastApplication += DeltaTime;
	Lifetime += DeltaTime;
}

bool FActiveGameplayEffect::OnGameplayEffectRemoved(UGameplaySystemComponent* GameplaySystem, AActor* Actor)
{
	// Apply AttributeEffects if configured to be done on removal
	if (PeriodType == EPeriodApplicationType::EPAT_ExecuteOnRemoval)
	{
		for (FAttributeEffect& AttributeEffect : AttributeEffects)
		{
			if (DurationType == EDurationType::EDT_Instant)
			{
				GameplaySystem->ApplyAttributeEffectNoRemoval(AttributeEffect);
			}
			else
			{
				GameplaySystem->ApplyAttributeEffect(AttributeEffect);
			}
		}
	}

	// First, remove any applied Attribute Effects
	for (FAttributeEffect& AttributeEffect : AttributeEffects)
	{
		GameplaySystem->RemoveAttributeEffect(AttributeEffect);
	}

	// Remove the GameplayTag modifiers
	TagModifierContainer.ReverseApply(GameplaySystem->GetGameplayTagSystem());

	// Inform the class of the removal
	if (UGameplayEffect* Definition = GetDefinition())
	{
		return Definition->RemoveGameplayEffect(GameplaySystem, Actor);
	}

	return true;
}

bool FActiveGameplayEffect::IsPeriodPassed() const
{
	return PeriodLength == FGameplayEffectConstants::NO_PERIOD ? false : TimeSinceLastApplication >= PeriodLength;
}

bool FActiveGameplayEffect::IsExpired() const
{
	// Will never expire
	if (DurationType == EDurationType::EDT_Infinite)
	{
		return false;
	}

	return Lifetime >= Duration;
}

float FActiveGameplayEffect::GetRemainingDuration() const
{
	return Duration == FGameplayEffectConstants::INFINITE_DURATION ? 0.0f : Duration - Lifetime;
}

FString FActiveGameplayEffect::ToString() const
{
	FString Output = Name + TEXT("\n");
	Output += FString::Printf(TEXT("Duration: %.1f"), GetRemainingDuration()) + TEXT("\n");

	for (const FAttributeEffect& Effect : AttributeEffects)
	{
		Output += TEXT("   ") + Effect.ToString() + TEXT("\n");
	}

	return Output;
}

UGameplayEffect::UGameplayEffect()
{
}

#if WITH_EDITOR
void UGameplayEffect::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	GenerateGUID(Id);
}
#endif

void UGameplayEffect::GenerateGUID(FString& IdRef)
{
	bool bHasIdAlready = IdRef != "";

	if (bHasIdAlready)
	{
		return;
	}

	IdRef = FGuid::NewGuid().ToString();
}

bool UGameplayEffect::ApplyGameplayEffect(UGameplaySystemComponent* GameplaySystem, AActor* Actor) const
{
	const float RandomPercentage = FMath::FRandRange(0.0f, 100.0f);

	// Did not pass the chance to apply
	if (RandomPercentage >= ChanceToApply)
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

	// The Gameplay Effect is applied, but might not apply its Attribute Effects on application
	if (PeriodType != EPeriodApplicationType::EPAT_ExecuteOnApplication)
	{
		return true;
	}

	// --- Apply modifiers & effects to the GameplaySystem and owner Actor

	const bool bApplyEffectNow = PeriodType == EPeriodApplicationType::EPAT_ExecuteOnApplication;
	if (bApplyEffectNow)
	{
		// Apply the AttributeEffects
		for (const FAttributeEffect& AttributeEffect : AttributeEffects)
		{
			if (DurationType == EDurationType::EDT_Instant)
			{
				// Doesnt store the AttributeEffect, and instantly does the necessary calculations
				GameplaySystem->ApplyAttributeEffectNoRemoval(AttributeEffect);
			}
			else
			{
				// Stores the AttributeEffect, to allow for look-ups and traceability of the Effects
				GameplaySystem->ApplyAttributeEffect(AttributeEffect);
			}
		}

		// Apply the GameplayTag modifiers
		TagModifierContainer.Apply(GameplaySystem->GetGameplayTagSystem());
	}

	return true;
}

bool UGameplayEffect::RemoveGameplayEffect(UGameplaySystemComponent* GameplaySystem, AActor* Actor) const
{
	check(GameplaySystem);

	return true;
}
