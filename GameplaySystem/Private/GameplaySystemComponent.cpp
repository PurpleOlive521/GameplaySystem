// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


#include "GameplaySystemComponent.h"

#include "AttributeDataSet.h"
#include "AttributeEffect.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplaySaveGameTypes.h"
#include "GameplayTagDefines.h"
#include "DevelopmentTypes.h"
#include "GameplaySystemDeveloperSettings.h"

UGameplaySystemComponent::UGameplaySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGameplaySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	GameplaySystemSettings = GetDefault<UGameplaySystemDeveloperSettings>();

	InitializeAttributes();

	InitializeLevelSystem();

	InitializeGameplayTags();

	InitializeGameplaySystemActorInfo(); // Call before GameplayAbilities in case they need ActorInfo for their own initialization

	// Applying the movement value to the Component at start
	if (UCharacterMovementComponent* MovementComponent = GetOwner()->FindComponentByClass<UCharacterMovementComponent>())
	{
		MovementComponent->MaxWalkSpeed = GetAttributeValue(EAttributeType::EAT_MovementSpeed, EAttributeValue::EAV_CurrentValue);
	}
}

void UGameplaySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UGameplaySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsGameplayEffectsPaused)
	{
		TArray<FGameplayEffectHandle> EffectsToRemove;

		for (auto It = ActiveGameplayEffects.CreateIterator(); It; ++It)
		{
			It->Value.Tick(DeltaTime, this);

			if (It->Value.IsExpired())
			{
				EffectsToRemove.Emplace(It.Key());
			}
		}

		// Remove outside the for-loop
		for (FGameplayEffectHandle& Effect : EffectsToRemove)
		{
			RemoveGameplayEffectByHandle(Effect);
		}
	}

	TArray<FGameplayAbilityHandle> ExpiredAbilities;
	for (auto& [Handle, ActiveAbility] : ActiveAbilityMap)
	{
		ActiveAbility.Tick(DeltaTime, this);

		if (ActiveAbility.ShouldBeRemoved())
		{
			ExpiredAbilities.Emplace(Handle);
		}
	}

	// Clean up expired abilities
	for (const FGameplayAbilityHandle& Handle : ExpiredAbilities)
	{
		ActiveAbilityMap.Remove(Handle);

		// We do not remove instances that are instanced per actor here, as they are reused
		if (UGameplayAbility* Instance = GetAbilityInstanceFromHandle(Handle))
		{
			if (Instance->GetInstancingPolicy() == EInstancingPolicy::EIP_InstancedPerActor)
			{
				continue;
			}
		}

		AbilityInstanceMap.Remove(Handle);
	}
}

void UGameplaySystemComponent::AddTag(const FGameplayTag& TagToAdd)
{
	GameplayTagSystem.AddTag(TagToAdd);
}

void UGameplaySystemComponent::RemoveTag(const FGameplayTag& TagToRemove)
{
	GameplayTagSystem.RemoveTag(TagToRemove);
}

void UGameplaySystemComponent::ClearTag(const FGameplayTag& TagToClear)
{
	GameplayTagSystem.ClearTag(TagToClear);
}

void UGameplaySystemComponent::AppendTags(FGameplayTagContainer const& Other)
{
	GameplayTagSystem.AppendTags(Other);
}

bool UGameplaySystemComponent::HasTag(const FGameplayTag& TagToCheck)
{
	return GameplayTagSystem.HasTag(TagToCheck);
}

bool UGameplaySystemComponent::HasAllTags(const FGameplayTagContainer& TagsToCheckAgainst)
{
	return GameplayTagSystem.HasAllTags(TagsToCheckAgainst);
}

int UGameplaySystemComponent::GetTagCount(const FGameplayTag& TagToCheck)
{
	return GameplayTagSystem.GetTagCount(TagToCheck);
}

int UGameplaySystemComponent::GetTotalTagCount()
{
	return GameplayTagSystem.GetTotalTagCount();
}

void UGameplaySystemComponent::EvaluateAttributes()
{
	// Assume that no Attribute has an Active Effect, and reset the CurrentValue
	for (auto& [Type, Attribute] : Attributes)
	{
		Attribute.CurrentValue = Attribute.BaseValue;
	}

	// Sort such that the Effects are in ascending order per EEffectApplicationType
	ActiveAttributeEffects.Sort([](const FAttributeEffect& A, const FAttributeEffect& B)
		{
			return A.ApplicationType < B.ApplicationType;
		});

	// Apply each AttributeEffect to the corresponding Attribute
	for (FAttributeEffect& Effect : ActiveAttributeEffects)
	{
		EAttributeType EffectAttribute = Effect.Attribute;
		FAttribute* AffectedAttribute = Attributes.Find(EffectAttribute);

		if (!AffectedAttribute)
		{
			continue;
		}
		
		Effect.ApplyAttributeEffect(*AffectedAttribute);
	}

	// Apply AttributeConfigs
	for (auto& [Type, Attribute] : Attributes)
	{
		const FAttributeConfiguration AttributeConfig = GameplaySystemSettings->AttributeSettings.FindRef(Type);

		if (!AttributeConfig.bIsUnsigned)
		{
			if (Attribute.CurrentValue < 0.0f)
			{
				Attribute.CurrentValue = 0.0f;
			}

			if (Attribute.BaseValue < 0.0f)
			{
				Attribute.BaseValue = 0.0f;
			}
		}

		if (AttributeConfig.MaxValueReference != EAttributeType::EAT_NONE)
		{
			// We can't call the GetAttributeValue functions since bIsDirty is still flagged
			float MaxValue = Attributes.FindRef(AttributeConfig.MaxValueReference).CurrentValue;
			if (Attribute.CurrentValue > MaxValue)
			{
				Attribute.CurrentValue = MaxValue;
			}

			if (Attribute.BaseValue > MaxValue)
			{
				Attribute.BaseValue = MaxValue;
			}
		}

	}

	bIsDirty = false;
}

FGameplaySystemSaveObject UGameplaySystemComponent::SaveToObject() const
{
	FGameplaySystemSaveObject SaveData;
	SaveData.ValidityKey.MakeValid();

	GetAttributes(SaveData.Attributes);
	GetActiveAttributeEffects(SaveData.ActiveEffects);
	GetActiveGameplayEffects(SaveData.ActiveGameplayEffects);
	SaveData.EntityLevel = GetEntityLevel();
	SaveData.Experience = EntityExperience;
	SaveData.GameplayTags = GameplayTagSystem.GetSaveData();

	return SaveData;
}

void UGameplaySystemComponent::LoadFromObject(const FGameplaySystemSaveObject& GameplaySystemSaveObject)
{
	CHECK_VALIDITY_EARLY_RETURN(GameplaySystemSaveObject);

	// Apply first so that any redundant stat changes are overwritten by the saved attributes.
	SetEntityLevel(GameplaySystemSaveObject.EntityLevel);
	SetExperience(GameplaySystemSaveObject.Experience);

	SetAttributes(GameplaySystemSaveObject.Attributes);
	SetActiveAttributeEffects(GameplaySystemSaveObject.ActiveEffects);

	SetActiveGameplayEffects(GameplaySystemSaveObject.ActiveGameplayEffects);
	GameplayTagSystem.LoadFromData(GameplaySystemSaveObject.GameplayTags);
}

void UGameplaySystemComponent::InitializeAttributes()
{
	if (AttributeDataSet == nullptr)
	{
		GS_LOG(Error, TEXT("AttributeComponent: Error - Initialized with null dataset"));
		return;
	}

	Attributes.Empty();

	// Parsing the values to FAttribute and adding them to TMap
	for (const auto& EditorAttribute : AttributeDataSet->Attributes)
	{
		FAttribute ParsedAttribute = { EditorAttribute.Type, EditorAttribute.BaseValue, EditorAttribute.CurrentValue };

		Attributes.Emplace(ParsedAttribute.AttributeType, ParsedAttribute);
	}

	bIsDirty = true;

	return;
}

void UGameplaySystemComponent::InitializeGameplayTags()
{
	GameplayTagSystem.AppendTags(StartUpTags);
}

void UGameplaySystemComponent::InitializeGameplaySystemActorInfo()
{


	GameplaySystemActorInfo = MakeShared<FGameplaySystemActorInfo>();
	GameplaySystemActorInfo->Init(GetOwner(), this);
}

float UGameplaySystemComponent::GetAttributeValue(EAttributeType AttributeType, EAttributeValue TargetValue)
{
	if (bIsDirty)
	{
		EvaluateAttributes();
	}

	switch (TargetValue)
	{
	case EAttributeValue::EAV_BaseValue:
		return Attributes.FindRef(AttributeType).BaseValue;

	case EAttributeValue::EAV_CurrentValue:
		return Attributes.FindRef(AttributeType).CurrentValue;

	default:
		checkNoEntry(); // Value not supported yet.
	}

	return 0.0f;
}

void UGameplaySystemComponent::ModifyAttributeValue(EAttributeType AttributeType, EAttributeValue TargetValue, float ValueChange)
{
	FAttribute* Attribute = Attributes.Find(AttributeType);

	if (Attribute)
	{
		switch (TargetValue)
		{
		case EAttributeValue::EAV_BaseValue:
			Attribute->BaseValue += ValueChange;
			break;

		case EAttributeValue::EAV_CurrentValue:
			Attribute->CurrentValue += ValueChange;
			break;

		default:
			checkNoEntry(); // Value not supported yet.
		}

		bIsDirty = true;

		OnAttributeChangedDelegateCollection.Broadcast(AttributeType);
	}
}

void UGameplaySystemComponent::SetAttributeValue(EAttributeType AttributeType, EAttributeValue TargetValue, float NewValue)
{
	FAttribute* Attribute = Attributes.Find(AttributeType);

	if (Attribute)
	{
		switch (TargetValue)
		{
		case EAttributeValue::EAV_BaseValue:
			Attribute->BaseValue = NewValue;
			break;

		case EAttributeValue::EAV_CurrentValue:
			Attribute->CurrentValue = NewValue;
			break;

		default:
			checkNoEntry(); // Value not supported yet.
		}

		bIsDirty = true;

		OnAttributeChangedDelegateCollection.Broadcast(AttributeType);
	}
}

void UGameplaySystemComponent::ClampAttributeValue(EAttributeType AttributeType, EAttributeValue TargetValue, float Min, float Max)
{
	FAttribute* Attribute = Attributes.Find(AttributeType);

	if (Attribute)
	{

		switch (TargetValue)
		{
		case EAttributeValue::EAV_BaseValue: 
			{
				const float ValueBefore = Attribute->BaseValue;
				Attribute->BaseValue = FMath::Clamp(Attribute->BaseValue, Min, Max);

				// No change in value, no need to broadcast or recalculate
				if (ValueBefore == Attribute->BaseValue)
				{
					return;
				}

				break;
			}
		case EAttributeValue::EAV_CurrentValue:
			{
				const float ValueBefore = Attribute->CurrentValue;
				Attribute->CurrentValue = FMath::Clamp(Attribute->CurrentValue, Min, Max);

				// No change in value, no need to broadcast or recalculate
				if (ValueBefore == Attribute->CurrentValue)
				{
					return;
				}

				break;
			}
		default:
			checkNoEntry(); // Value not supported yet.
		}

		bIsDirty = true;

		OnAttributeChangedDelegateCollection.Broadcast(AttributeType);
	}
}

bool UGameplaySystemComponent::HasAttributeType(EAttributeType AttributeType)
{
	return Attributes.Contains(AttributeType);
}

void UGameplaySystemComponent::ApplyAttributeEffect(FAttributeEffect EffectToApply, EDurationType Type)
{
	if (Type == EDurationType::EDT_Instant)
	{
		ApplyAttributeEffect_Internal_NoRemoval(EffectToApply);
	}
	else
	{
		ApplyAttributeEffect_Internal(EffectToApply);
	}
}

void UGameplaySystemComponent::RemoveAttributeEffect(FAttributeEffect& EffectToRemove)
{
	int Index = ActiveAttributeEffects.Find(EffectToRemove);

	//Nothing to remove
	if (Index == INDEX_NONE)
	{
		return;
	}

	FAttribute* AffectedAttribute = Attributes.Find(EffectToRemove.Attribute);

	// Correctly remove the Effect by reversing its change to the Attribute
	if (AffectedAttribute)
	{
		EffectToRemove.RemoveAttributeEffect(*AffectedAttribute);
	}

	ActiveAttributeEffects.RemoveAt(Index);


	// If the affected Attribute exists, prompt a recalculation
	if (HasAttributeType(EffectToRemove.Attribute))
	{
		bIsDirty = true;

		OnAttributeChangedDelegateCollection.Broadcast(EffectToRemove.Attribute);
	}
}

int UGameplaySystemComponent::GetActiveEffectsCount()
{
	return ActiveAttributeEffects.Num();
}

void UGameplaySystemComponent::SetAttributeDataSet(UAttributeDataSet* InAttributeDataSet)
{
	AttributeDataSet = InAttributeDataSet;
}

void UGameplaySystemComponent::GetAttributes(TMap<EAttributeType, FAttribute>& AttributesOut) const
{
	AttributesOut = Attributes;
}

void UGameplaySystemComponent::SetAttributes(const TMap<EAttributeType, FAttribute>& AttributesIn)
{
	Attributes = AttributesIn;
	bIsDirty = true;
}

void UGameplaySystemComponent::GetActiveAttributeEffects(TArray<FAttributeEffect>& ActiveEffectsOut) const
{
	ActiveEffectsOut = ActiveAttributeEffects;
}

void UGameplaySystemComponent::SetActiveAttributeEffects(const TArray<FAttributeEffect>& AttributeEffectsIn)
{
	ActiveAttributeEffects = AttributeEffectsIn;
	bIsDirty = true;
}

TMap<EAttributeType, FAttribute>::TConstIterator UGameplaySystemComponent::GetConstAttributeIterator() const
{
	return TMap<EAttributeType, FAttribute>::TConstIterator(Attributes);
}

void UGameplaySystemComponent::AddAttribute(FAttribute Attribute)
{
	Attributes.Add(Attribute.AttributeType, Attribute);
	bIsDirty = true;

	OnAttributeChangedDelegateCollection.Broadcast(Attribute.AttributeType);
}

void UGameplaySystemComponent::SimulateAttributes(const TArray<FAttributeEffect>& AttributeEffectsToSimulate, TMap<EAttributeType, FAttribute>& GeneratedAttributesOut)
{
	// We dont want other systems to try and react to these changes

	OnAttributeChangedDelegateCollection.bIsSilenced = true;

	TMap<EAttributeType, FAttribute> OriginalAttributes;
	TArray<FAttributeEffect> OriginalActiveEffects;

	GetAttributes(OriginalAttributes);
	GetActiveAttributeEffects(OriginalActiveEffects);

	// Apply the AttributeEffectsToSimulate
	for (const FAttributeEffect& Effect : AttributeEffectsToSimulate)
	{
		ApplyAttributeEffect(Effect, EDurationType::EDT_Infinite);
	}

	// Get all affected Attributes
	for (const FAttributeEffect& Effect : AttributeEffectsToSimulate)
	{
		FAttribute ChangedAttribute = { 
			Effect.Attribute, 
			GetAttributeValue(Effect.Attribute, EAttributeValue::EAV_BaseValue), 
			GetAttributeValue(Effect.Attribute, EAttributeValue::EAV_CurrentValue) 
		};

		GeneratedAttributesOut.Add(Effect.Attribute, ChangedAttribute);
	}

	// Restore state
	SetAttributes(OriginalAttributes);
	SetActiveAttributeEffects(OriginalActiveEffects);

	OnAttributeChangedDelegateCollection.bIsSilenced = false;
}

float UGameplaySystemComponent::CalculateCoefficientAttribute(const FCoefficientAttribute& CoefficientAttr)
{
	if (CoefficientAttr.Attribute == EAttributeType::EAT_NONE)
	{
		return CoefficientAttr.Coefficient;
	}

	if (!HasAttributeType(CoefficientAttr.Attribute))
	{
		return 0.0f;
	}

	const float AttributeValue = GetAttributeValue(CoefficientAttr.Attribute, CoefficientAttr.Target);

	return CoefficientAttr.Coefficient * AttributeValue;
}

void UGameplaySystemComponent::ForceEvaluateAttributes()
{
	EvaluateAttributes();
}

int UGameplaySystemComponent::GetEntityLevel() const
{
	return EntityLevel;
}

void UGameplaySystemComponent::SetEntityLevel(const int& NewLevel, bool bDoSilently)
{
	if (!LevelScalingCurveTable)
	{
		GS_LOG(Error, TEXT("GameplaySystemComponent: No assigned CurveTable for leveling!")); 
		return;
	}

	if (NewLevel == EntityLevel)
	{
		return;
	}

	// Indicates that the Entity does not want to use the Level system
	if (EntityLevel == GameplaySystemConstants::NO_LEVEL || NewLevel == GameplaySystemConstants::NO_LEVEL)
	{
		return;
	}

	if (NewLevel < 0)
	{
		GS_LOG(Warning, TEXT("GameplaySystemComponent: New Level cannot be negative!"));
		return;
	}

	FName RowName = FName("ExpForNextLevel");
	FSimpleCurve* ExperienceCurve = LevelScalingCurveTable->FindSimpleCurve(RowName, TEXT("GetExperienceForLevel"));
	if (ExperienceCurve == nullptr)
	{
		GS_LOG(Error, TEXT("GameplaySystemComponent: No curve found for LevelSystem!"));
		return;
	}

	TMap<EAttributeType, float> AttributeDeltaMap;
	const int Direction = NewLevel > EntityLevel ? 1 : -1;
	const int LevelsToTraverse = abs(NewLevel - EntityLevel);

	for (auto& [Type, Attribute] : Attributes)
	{
		FString EnumString = UEnum::GetValueAsString(Type);
		FString LeftString, RightString;
		EnumString.Split(TEXT("::EAT_"), &LeftString, &RightString);
		FName EnumName = FName(*RightString);

		FSimpleCurve* ScalingCurve = LevelScalingCurveTable->FindSimpleCurve(EnumName, TEXT("GetAttributeScalingValue"));
		if (ScalingCurve == nullptr)
		{
			FString Message = TEXT("GameplaySystemComponent: No curve found for ") + EnumString;
			GS_LOG(Error, TEXT("%s"), *Message);
			continue;
		}

		for (int i = 1; i <= LevelsToTraverse; i++)
		{
			const int EvaluatedLevel = EntityLevel + (i * Direction);

			const float Delta = ScalingCurve->Eval(EvaluatedLevel, 0.0f);

			AttributeDeltaMap.FindOrAdd(Type) += Delta * Direction;
		}
	}

	for (const auto& [Type, Delta] : AttributeDeltaMap)
	{
		FAttribute* Attribute = Attributes.Find(Type);
		check(Attribute);

		Attribute->BaseValue += Delta;

		if (const FAttributeConfiguration* Config = GameplaySystemSettings->AttributeSettings.Find(Type))
		{
			if (Config->bScaleWithMaxValue)
			{
				const float ScalingDelta = AttributeDeltaMap.FindRef(Config->MaxValueReference);
				Attribute->BaseValue += ScalingDelta;
			}
		}
	}

	for (const auto& [Type, Delta] : AttributeDeltaMap)
	{
		OnAttributeChangedDelegateCollection.Broadcast(Type);
	}

	bIsDirty = true;
	const int PreviousLevel = EntityLevel;
	EntityLevel = NewLevel;
	ExpRequiredForNextLevel = ExperienceCurve->Eval(EntityLevel);

	if (bDoSilently == false)
	{
		OnLeveledUpDelegate.Broadcast(PreviousLevel, EntityLevel, ExpRequiredForNextLevel);
	}
}

void UGameplaySystemComponent::InitializeLevelSystem()
{
	// Treats this Entity as if it went from level 1 to the specified level, appying all the stat changes required appropriately
	int LevelTarget = EntityLevel;
	EntityLevel = 1;
	SetEntityLevel(LevelTarget);
}

// Recursively adds experience whenever a level-up occurs.
void UGameplaySystemComponent::AddExperience(float Experience)
{
	int LevelBefore = EntityLevel;
	bool bWasLevelUpTriggered = false;

	// Recursively adds experience and triggering level ups when above or equal the required amount
	do
	{

		// Is the total experience added enough to pass the levelup threshold?
		float TotalExperience = EntityExperience + Experience;
		if (TotalExperience >= ExpRequiredForNextLevel)
		{
			Experience -= ExpRequiredForNextLevel;

			EntityExperience = 0;
			SetEntityLevel(EntityLevel + 1);
			bWasLevelUpTriggered = true;
		}
		else
		{
			EntityExperience += Experience;
			Experience = 0.0f;
			break;
		}
	} while (Experience >= 0.0f);

	if (bWasLevelUpTriggered)
	{
		OnLeveledUpDelegate.Broadcast(LevelBefore /*Level before levelups*/, EntityLevel /*Current Level*/, ExpRequiredForNextLevel /*Exp required for next level*/);
	}
}

void UGameplaySystemComponent::SetExperience(float Experience)
{
	float Delta = EntityExperience - Experience;

	// Let AddExperience check if we have enough experience to level up
	if (Delta >= 0)
	{
		AddExperience(Delta);
	}
	else
	{
		EntityExperience = Experience;

		if (EntityExperience < 0)
		{
			EntityExperience = 0;
		}
	}

}

float UGameplaySystemComponent::GetEntityExperience() const
{
	return EntityExperience;
}

float UGameplaySystemComponent::GetRequiredExperienceForNextLevel() const
{
	return ExpRequiredForNextLevel;
}

float UGameplaySystemComponent::GetExperienceRemainingForNextLevel() const
{
	return ExpRequiredForNextLevel - EntityExperience;
}

bool UGameplaySystemComponent::AddGameplayEffect(UGameplayEffect* EffectToApply, FGameplayEffectHandle& OutHandle)
{
	check(EffectToApply);
	FActiveGameplayEffect ActiveGameplayEffect(EffectToApply);

	return AddGameplayEffect_Internal(ActiveGameplayEffect, OutHandle);
}

bool UGameplaySystemComponent::RemoveGameplayEffect(const UGameplayEffect* EffectToRemove)
{
	check(EffectToRemove);

	for (const auto& [Handle, ActiveEffect] : ActiveGameplayEffects)
	{
		if (ActiveEffect.GetDefinition() == EffectToRemove)
		{
			return RemoveGameplayEffectByHandle(Handle);
		}
	}

	return false;
}

bool UGameplaySystemComponent::AddGameplayEffectFromType(TSubclassOf<UGameplayEffect> EffectToApply, FGameplayEffectHandle& OutHandle)
{
	check(EffectToApply);
	FActiveGameplayEffect ActiveGameplayEffect(EffectToApply);

	return AddGameplayEffect_Internal(ActiveGameplayEffect, OutHandle);
}

bool UGameplaySystemComponent::RemoveGameplayEffectFromType(TSubclassOf<UGameplayEffect> EffectToRemove)
{
	check(EffectToRemove);

	for (const auto& [Handle, ActiveEffect] : ActiveGameplayEffects)
	{
		if (ActiveEffect.GetDefinition()->IsA(EffectToRemove))
		{
			return RemoveGameplayEffectByHandle(Handle);
		}
	}

	return false;
}

bool UGameplaySystemComponent::AddGameplayEffect_Internal(FActiveGameplayEffect& EffectToAdd, FGameplayEffectHandle& OutHandle)
{
	const UGameplayEffect* GameplayEffectCDO = EffectToAdd.GetDefinition();
	check(GameplayEffectCDO);

	const bool bAppliedSuccessfully = GameplayEffectCDO->ApplyGameplayEffect(this, GetOwner(), EffectToAdd, OutHandle);

	return bAppliedSuccessfully;
}

bool UGameplaySystemComponent::RemoveGameplayEffect_Internal(const FGameplayEffectHandle& EffectToRemove)
{
	return ActiveGameplayEffects.Remove(EffectToRemove) > 0;
}

void UGameplaySystemComponent::ApplyAttributeEffect_Internal(FAttributeEffect EffectToApply)
{
	ActiveAttributeEffects.Add(EffectToApply);

	//If it has the affected Attribute, prompt a recalculation
	if (HasAttributeType(EffectToApply.Attribute))
	{
		bIsDirty = true;

		OnAttributeChangedDelegateCollection.Broadcast(EffectToApply.Attribute);
	}
}

void UGameplaySystemComponent::ApplyAttributeEffect_Internal_NoRemoval(FAttributeEffect EffectToApply)
{
	FAttribute* AffectedAttribute = Attributes.Find(EffectToApply.Attribute);

	// No corresponding Attribute found to apply to, skip
	if (!AffectedAttribute)
	{
		return;
	}

	EffectToApply.ApplyAttributeEffect(*AffectedAttribute);
	bIsDirty = true;

	OnAttributeChangedDelegateCollection.Broadcast(EffectToApply.Attribute);
}

void UGameplaySystemComponent::RegisterGameplayEffect(const FGameplayEffectHandle& Handle, FActiveGameplayEffect& ActiveEffect)
{
	ActiveGameplayEffects.Add(Handle, ActiveEffect);
}

bool UGameplaySystemComponent::RemoveGameplayEffectByHandle(const FGameplayEffectHandle& EffectToRemove)
{
	FActiveGameplayEffect* FoundEffect = GetActiveGameplayEffectByHandle(EffectToRemove);

	if (FoundEffect)
	{
		UGameplayEffect* GameplayEffectCDO = FoundEffect->GetDefinition();
		check(GameplayEffectCDO);

		return GameplayEffectCDO->RemoveGameplayEffect(this, GetOwner(), *FoundEffect, EffectToRemove);
	}
	
	return false;
}

int UGameplaySystemComponent::RemoveGameplayEffectsByHandles(const TArray<FGameplayEffectHandle>& EffectsToRemove)
{
	int Count = 0;
	for (const FGameplayEffectHandle& Handle : EffectsToRemove)
	{
		Count += RemoveGameplayEffectByHandle(Handle);
	}

	return Count;
}

int UGameplaySystemComponent::RemoveAllGameplayEffectsByPredicate(std::function<bool(const FActiveGameplayEffect&)> Predicate)
{
	TArray<FGameplayEffectHandle> EffectsToRemove;

	// Collect all Effects that match the Name
	for (auto It = ActiveGameplayEffects.CreateConstIterator(); It; ++It)
	{
		if (Predicate(It->Value))
		{
			EffectsToRemove.Add(It->Key);
		}
	}

	// Remove all collected keys separately, to avoid invalidating the iterator
	int RemovedCount = 0;
	for (const FGameplayEffectHandle& EffectHandle : EffectsToRemove)
	{
		bool bResult = RemoveGameplayEffectByHandle(EffectHandle);

		if (bResult == false)
		{
			RemovedCount++;
		}
	}

	return RemovedCount;
}

bool UGameplaySystemComponent::RemoveAllGameplayEffectsWithName(FString Name)
{
	auto CompareNames = [Name](const FActiveGameplayEffect& Effect)
	{
		const UGameplayEffect* Def = Effect.GetDefinition();
		return Def->Name == Name;
	};

	const int RemovedCount = RemoveAllGameplayEffectsByPredicate(CompareNames);

	return RemovedCount > 0;
}

bool UGameplaySystemComponent::RemoveAllGameplayEffectsWithTag(const FGameplayTag& TagToRemove)
{
	auto HasTag = [TagToRemove](const FActiveGameplayEffect& Effect)
	{
		const UGameplayEffect* Def = Effect.GetDefinition();
		return Def->TagsOnEffect.HasTag(TagToRemove);
	};

	const int RemovedCount = RemoveAllGameplayEffectsByPredicate(HasTag);

	return RemovedCount > 0;
}

void UGameplaySystemComponent::GetActiveGameplayEffects(TMap<FGameplayEffectHandle, FActiveGameplayEffect>& EffectsOut) const
{
	EffectsOut = ActiveGameplayEffects;
}

void UGameplaySystemComponent::SetActiveGameplayEffects(const TMap<FGameplayEffectHandle, FActiveGameplayEffect>& EffectsIn)
{
	ActiveGameplayEffects = EffectsIn;
}

int UGameplaySystemComponent::GetActiveGameplayEffectsCount() const
{
	return ActiveGameplayEffects.Num();
}

FActiveGameplayEffect* UGameplaySystemComponent::GetActiveGameplayEffectByHandle(const FGameplayEffectHandle& Handle)
{
	return ActiveGameplayEffects.Find(Handle);
}

void UGameplaySystemComponent::GetMatchingGameplayEffects(const FGameplayTagQuery& TagQuery, TArray<FGameplayEffectHandle>& OutHandles) const
{
	if (TagQuery.IsEmpty())
	{
		return;
	}

	for (auto& [Handle, Effect] : ActiveGameplayEffects)
	{
		if(TagQuery.Matches(Effect.GetDefinition()->TagsOnEffect))
		{
			OutHandles.Emplace(Handle);
		}
	}
}

bool UGameplaySystemComponent::HasGameplayEffectOfInstance(const UGameplayEffect* EffectToCheck, FGameplayEffectHandle& OutHandle) const
{
	for (const auto& [Handle, ActiveEffect] : ActiveGameplayEffects)
	{
		if (ActiveEffect.GetDefinition() == EffectToCheck)
		{
			OutHandle = Handle;
			return true;
		}
	}

	return false;
}

bool UGameplaySystemComponent::HasGameplayEffect(const FGameplayEffectHandle& Handle) const
{
	return ActiveGameplayEffects.Find(Handle) != nullptr;
}

void UGameplaySystemComponent::PauseAllGameplayEffects()
{
	bIsGameplayEffectsPaused = true;
}

void UGameplaySystemComponent::UnpauseAllGameplayEffects()
{
	bIsGameplayEffectsPaused = false;
}

void UGameplaySystemComponent::K2_GetGameplayTagSystem(FGameplayTagSystem& OutGameplayTagSystem) const
{
	OutGameplayTagSystem = GameplayTagSystem;
}

void UGameplaySystemComponent::ApplyBlockingAndCancellingTags(const FGameplayTagContainer& BlockingTags, const FGameplayTagContainer& CancellingTags, UGameplayAbility* Callee)
{
	BlockedAbilityTags.AppendTags(BlockingTags);

	TArray<FGameplayAbilityHandle> HandlesToCancel;
	GetActiveAbilitiesByTags(BlockingTags, HandlesToCancel, Callee);
	CancelAbilities(HandlesToCancel);
}

void UGameplaySystemComponent::RemoveBlockingTags(const FGameplayTagContainer& BlockingTags)
{
	BlockedAbilityTags.RemoveTags(BlockingTags);
}

FGameplayTagSystem& UGameplaySystemComponent::GetGameplayTagSystemAsRef()
{
	return GameplayTagSystem;
}

FGameplayTagContainer UGameplaySystemComponent::GetBlockingAbilityTags() const
{
	return BlockedAbilityTags.GameplayTags;
}

FGameplayTagSystem* UGameplaySystemComponent::GetGameplayTagSystem()
{
	return &GameplayTagSystem;
}
 
FGameplayAbilityHandle UGameplaySystemComponent::GetAbilityHandleFromInstance(UGameplayAbility* Instance)
{
	if (!Instance)
	{
		return FGameplayAbilityHandle();
	}

	for(const auto& [AbilityHandle, AbilityInstance] : AbilityInstanceMap)
	{
		if (AbilityInstance == Instance)
		{
			return AbilityHandle;
		}
	}

	return FGameplayAbilityHandle();
}

FGameplayAbilityHandle UGameplaySystemComponent::GetAbilityHandleFromActiveAbility(const FActiveGameplayAbility& InActiveAbility)
{
	for (const auto& [AbilityHandle, ActiveAbility] : ActiveAbilityMap)
	{
		if (InActiveAbility == ActiveAbility)
		{
			return AbilityHandle;
		}
	}

	return FGameplayAbilityHandle();
}


void UGameplaySystemComponent::GetAbilitiesByTag(const FGameplayTag& TagToCheck, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore) const
{
	auto HasRequiredTag = [TagToCheck](const UGameplayAbility* Ability)
		{
			return Ability->GetAbilityTags().HasTag(TagToCheck);
		};

	GetAbilitiesByPredicate(HasRequiredTag, OutHandles, Ignore);
}

void UGameplaySystemComponent::GetActiveAbilitiesByTag(const FGameplayTag& TagToCheck, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore) const
{
	auto HasRequiredTag = [TagToCheck](const FActiveGameplayAbility* Ability)
		{
			return Ability->AbilityTags.HasTag(TagToCheck);
		};

	GetActiveAbilitiesByPredicate(HasRequiredTag, OutHandles, Ignore);
}

void UGameplaySystemComponent::GetAbilitiesByTags(const FGameplayTagContainer& TagsToCheck, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore) const
{
	auto HasRequiredTags = [TagsToCheck](const UGameplayAbility* Ability)
		{
			return Ability->GetAbilityTags().HasAny(TagsToCheck);
		};

	GetAbilitiesByPredicate(HasRequiredTags, OutHandles, Ignore);
}

void UGameplaySystemComponent::GetActiveAbilitiesByTags(const FGameplayTagContainer& TagsToCheck, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore) const
{
	auto HasRequiredTags = [TagsToCheck](const FActiveGameplayAbility* Ability)
		{
			return Ability->AbilityTags.HasAny(TagsToCheck);
		};

	GetActiveAbilitiesByPredicate(HasRequiredTags, OutHandles, Ignore);
}

void UGameplaySystemComponent::GetAbilitiesByClass(TSubclassOf<UGameplayAbility> Class, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore) const
{
	auto IsOfClass = [Class](const UGameplayAbility* Ability)
		{
			return Ability->IsA(Class);
		};

	GetAbilitiesByPredicate(IsOfClass, OutHandles, Ignore);
}

void UGameplaySystemComponent::GetActiveAbilitiesByClass(TSubclassOf<UGameplayAbility> Class, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore) const
{
	checkNoEntry(); // Not implemented yet
}

void UGameplaySystemComponent::GetAbilitiesByPredicate(std::function<bool(const UGameplayAbility*)> Predicate, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore) const
{
	for (const auto& [Handle, Instance] : AbilityInstanceMap)
	{
		if (Instance == Ignore)
		{
			continue;
		}

		if (Predicate(Instance))
		{
			OutHandles.Add(Handle);
		}
	}
}

void UGameplaySystemComponent::GetActiveAbilitiesByPredicate(std::function<bool(const FActiveGameplayAbility*)> Predicate, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore) const
{
	for (const auto& [Handle, ActiveAbility] : ActiveAbilityMap)
	{
		if (AbilityInstanceMap.FindRef(Handle) == Ignore)
		{
			continue;
		}

		if (!ActiveAbility.IsAbilityActive())
		{
			continue;
		}

		if (Predicate(&ActiveAbility))
		{
			OutHandles.Add(Handle);
		}
	}
}

int UGameplaySystemComponent::GetAbilityInstanceCount() const
{
	return AbilityInstanceMap.Num();
}

int UGameplaySystemComponent::GetActiveAbilityCount() const
{
	return ActiveAbilityMap.Num();
}

void UGameplaySystemComponent::ActivateQueuedAbility()
{
	if (QueuedAbility)
	{
		FGameplayAbilityHandle Handle;
		const bool bActivated = UseAbility(QueuedAbility, Handle);
	}

	ClearAbilityQueue();
}

void UGameplaySystemComponent::QueueAbility(TSubclassOf<UGameplayAbility> AbilityToQueue)
{
	// Check if we can queue the Ability
	if (bAllowAbilityQueueing && HasTag(GAMEPLAYTAG_Status_BufferingAction))
	{
		QueuedAbility = AbilityToQueue;
	}
}

void UGameplaySystemComponent::ClearAbilityQueue()
{
	QueuedAbility = nullptr;
}

bool UGameplaySystemComponent::CancelAbility(const FGameplayAbilityHandle& Handle, bool bIsAuthoritative)
{
	if (!Handle.IsValid())
	{
		return false;
	}

	UGameplayAbility* AbilityInstance = AbilityInstanceMap.FindRef(Handle);
	ensure(AbilityInstance);

	if (!AbilityInstance)
	{
		return false;
	}

	const bool bWasCancelled = AbilityInstance->TryCancelAbility(bIsAuthoritative);
		
	return true;
}

bool UGameplaySystemComponent::CancelAbilities(TArray<FGameplayAbilityHandle> HandlesToCancel, bool bIsAuthoritative)
{
	bool bCancelledAnAbility = true;

	for (const FGameplayAbilityHandle& Handle : HandlesToCancel)
	{
		bCancelledAnAbility |= CancelAbility(Handle, bIsAuthoritative);
	}

	return bCancelledAnAbility;
}

void UGameplaySystemComponent::EndAbility(const FGameplayAbilityHandle& Handle)
{
	UGameplayAbility* AbilityInstance = AbilityInstanceMap.FindRef(Handle);
	ensure(AbilityInstance);

	if (!AbilityInstance)
	{
		return;
	}

	const bool bWasEnded = AbilityInstance->TryEndAbility();
	if (!bWasEnded)
	{
		return;
	}

	if (AbilityInstance == GetAnimatingAbility())
	{
		ClearAnimMontageInfo();
	}

	OnAbilityEndedDelegate.Broadcast(Handle);
}

void UGameplaySystemComponent::AddAbilityInstance(TSubclassOf<UGameplayAbility> AbilityClass)
{
	check(AbilityClass);

	const UGameplayAbility* AbilityCDO = AbilityClass->GetDefaultObject<UGameplayAbility>();

	// There is no point in adding the instance if it's not going to be reused. The activation sequences will create instances as needed.
	if (AbilityCDO->GetInstancingPolicy() != EInstancingPolicy::EIP_InstancedPerActor)
	{
		return;
	}

	bool bOutCreatedNew = false;
	UGameplayAbility* Instance = GetOrCreateAbilityInstance(AbilityClass, bOutCreatedNew);

	if (bOutCreatedNew)
	{
		// This looks weird. We are requesting that the new instance we made is added to the component, so it can be found and reused in the future.
		PreActivateAbility(Instance);
	}
}

void UGameplaySystemComponent::RemoveAbilityInstance(TSubclassOf<UGameplayAbility> AbilityClass)
{
	checkNoEntry();
	// Does nothing yet. We need to add a concept of "learning" or "owning" abilities that we can activate, in order
	// to safely remove instances.
}

void UGameplaySystemComponent::InformAbilityEnded(UGameplayAbility* AbilityInstance)
{
	check(AbilityInstance);

	FGameplayAbilityHandle Handle = GetAbilityHandleFromInstance(AbilityInstance);
	ensure(Handle.IsValid());

	if (GetAnimatingAbility() == AbilityInstance)
	{
		ClearAnimMontageInfo();
	}

	OnAbilityEndedDelegate.Broadcast(Handle);
}

FGameplaySystemActorInfo* UGameplaySystemComponent::GetActorInfo() const
{
	check(GameplaySystemActorInfo.IsValid());

	return GameplaySystemActorInfo.Get();
}

float UGameplaySystemComponent::PlayMontage(UGameplayAbility* PlayingAbility, UAnimMontage* MontageToPlay, FPlayMontageParams& Params)
{
	check(PlayingAbility);
	check(MontageToPlay);

	UAnimInstance* AnimInstance = GetActorInfo()->GetAnimInstance();
	if (!AnimInstance)
	{
		GS_LOG(Error, TEXT("UGameplaySystemComponent: No AnimInstance found when trying to play AnimMontage!"));
		return 0.0f;
	}

	// Stop the AnimMontage before updating AnimMontageInfo to catch any last AnimNotifies or AnimNotifyStates
	if (AnimMontageInfo.CurrentMontage)
	{
		AnimInstance->Montage_Stop(0.0f, AnimMontageInfo.CurrentMontage);
	}

	// Assign before playing in case that there is a frame-1 AnimNotify or AnimNotifyState on MontageToPlay that requires the AnimMontageInfo
	AnimMontageInfo.AssignMontage(MontageToPlay, PlayingAbility);

	const float AnimDuration = AnimInstance->Montage_Play(MontageToPlay, Params.PlayRate, Params.PlayReturnType);

	AnimInstance->Montage_SetBlendingOutDelegate(Params.MontageBlendOutDelegate, MontageToPlay);
	AnimInstance->Montage_SetEndDelegate(Params.MontageEndedDelegate, MontageToPlay);

	const bool bIsSectionValid = !Params.StartSection.IsNone() && MontageToPlay->IsValidSectionName(Params.StartSection);
	if (bIsSectionValid)
	{
		if (Params.bUseEndOfSection)
		{
			AnimInstance->Montage_JumpToSectionsEnd(Params.StartSection, MontageToPlay);
		}
		else
		{
			AnimInstance->Montage_JumpToSection(Params.StartSection, MontageToPlay);

		}
	}

	return AnimDuration;
}

UGameplayAbility* UGameplaySystemComponent::GetAnimatingAbility() const
{
	return AnimMontageInfo.GetAnimatingAbility();
}

void UGameplaySystemComponent::ClearAnimMontageInfo()
{
	AnimMontageInfo = FGameplaySystemAnimMontageInfo();
}

FGameplaySystemAnimMontageInfo* UGameplaySystemComponent::GetAnimMontageInfo()
{
	return &AnimMontageInfo;
}

void UGameplaySystemComponent::DestroyActiveState()
{
	// Abilities might depend on the owning actor
	if (!GetOwner() || GetActorInfo()->OwningActor.IsValid())
	{
		ActiveAbilityMap.Empty();
		AbilityInstanceMap.Empty();
		return;
	}

	// Try to end the abilities gracefully. Any activated abilities at this stage wont be covered and will be forcefully GC'ed instead.
	for (auto& [Handle, ActiveAbility] : ActiveAbilityMap)
	{
		CancelAbility(Handle, true);
	}

	ActiveAbilityMap.Empty();
	AbilityInstanceMap.Empty();

	ActiveGameplayEffects.Empty();
}

UGameplayAbility* UGameplaySystemComponent::GetOrCreateAbilityInstance(TSubclassOf<UGameplayAbility> AbilityClass, bool& OutbCreatedNew)
{
	check(AbilityClass);

	UGameplayAbility* Instance = nullptr;
	UGameplayAbility* AbilityCDO = AbilityClass->GetDefaultObject<UGameplayAbility>();

	EInstancingPolicy InstancingPolicy = AbilityCDO->GetInstancingPolicy();
	switch (InstancingPolicy)
	{
		case EInstancingPolicy::EIP_NoLifetime:
		{
			Instance = AbilityCDO;
			OutbCreatedNew = false;
			break;
		}

		case EInstancingPolicy::EIP_InstancedPerActor:
		{
			TArray<FGameplayAbilityHandle> ExistingHandles;
			GetAbilitiesByClass(AbilityClass, ExistingHandles, GET_ABILITY_NO_IGNORE);

			ensure(ExistingHandles.Num() <= 1); // Should only ever be one instance per actor

			if (ExistingHandles.Num() <= 0)
			{
				Instance = CreateNewAbilityInstance(AbilityClass);
				OutbCreatedNew = true;
			}
			else
			{
				const FGameplayAbilityHandle Handle = ExistingHandles[0];
				Instance = GetAbilityInstanceFromHandle(Handle);
			}

			break;
		}

		case EInstancingPolicy::EIP_InstancedPerExecution: 
		{
			Instance = CreateNewAbilityInstance(AbilityClass);
			OutbCreatedNew = true;

			break;
		}

		default: 
		{
			checkNoEntry(); // Not supported yet.
			break;
		}
	}

	return Instance;
}

UGameplayAbility* UGameplaySystemComponent::GetAbilityInstanceFromHandle(const FGameplayAbilityHandle& Handle)
{
	TObjectPtr<UGameplayAbility>* AbilityInstance = AbilityInstanceMap.Find(Handle);

	if (AbilityInstance)
	{
		return *AbilityInstance;
	}

	return nullptr;
}

FActiveGameplayAbility UGameplaySystemComponent::GetActiveAbilityFromHandle(const FGameplayAbilityHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return FActiveGameplayAbility();
	}

	return ActiveAbilityMap.FindRef(Handle);
}

FActiveGameplayAbility* UGameplaySystemComponent::GetActiveAbilityFromHandle_Ptr(const FGameplayAbilityHandle& Handle)
{
	if (!Handle.IsValid())
	{
		return nullptr;
	}

	return ActiveAbilityMap.Find(Handle);
}

FActiveGameplayAbility* UGameplaySystemComponent::GetActiveAbilityFromInstance_Ptr(UGameplayAbility* Instance)
{
	FGameplayAbilityHandle Handle = GetAbilityHandleFromInstance(Instance);
	
	return GetActiveAbilityFromHandle_Ptr(Handle);
}

void UGameplaySystemComponent::GetAbilityInstancesFromHandles(const TArray<FGameplayAbilityHandle> Handles, TArray<UGameplayAbility*>& OutAbilities)
{
	OutAbilities.SetNumZeroed(Handles.Num());

	for (int i = 0; i < Handles.Num(); i++)
	{
		OutAbilities[i] = GetAbilityInstanceFromHandle(Handles[i]);
	}
}

bool UGameplaySystemComponent::HasCooldown(TSubclassOf<UGameplayAbility> AbilityClass) const
{
	for (const auto& [Handle, ActiveAbility] : ActiveAbilityMap)
	{
		if(ActiveAbility.Ability->IsA(AbilityClass))
		{
			return ActiveAbility.HasCooldown();
		}
	}

	return false;
}

bool UGameplaySystemComponent::CanActivateAbility(TSubclassOf<UGameplayAbility> AbilityToQuery, const FGameplayAbilityActivationData& ActivationData)
{
	check(AbilityToQuery);

	bool bOutCreatedNew = false;
	UGameplayAbility* AbilityInstance = GetOrCreateAbilityInstance(AbilityToQuery, bOutCreatedNew);
	ensure(AbilityInstance);

	if (!AbilityInstance)
	{
		return false;
	}

	TArray<FGameplayAbilityHandle> HandlesToCancel;
	GetActiveAbilitiesByTags(AbilityInstance->CancelAbilitiesWithTag, HandlesToCancel, AbilityInstance);

	// Apply on-removal modifiers of all abilities that would be cancelled if AbilityToQuery activates.
	TArray<FGameplayAbilityHandle> CancelledHandles;
	UGameplayAbility* InstanceToCancel = nullptr;
	for (FGameplayAbilityHandle Handle : HandlesToCancel)
	{
		InstanceToCancel = GetAbilityInstanceFromHandle(Handle);
		ensure(InstanceToCancel);

		if (!InstanceToCancel)
		{
			continue;
		}

		if (AbilityInstance->bIsAuthoritativeCancel || InstanceToCancel->IsCancellable())
		{
			const bool bAppliedModifiers = InstanceToCancel->TryApplyAbilityEndedModifiers();

			// Already cancelled abilities need to be filtered out, so we don't revert their modifiers when they weren't applied to begin with
			if (bAppliedModifiers)
			{
				CancelledHandles.Add(Handle);
			}
		}
	}

	const bool bCanActivateAbility = AbilityInstance->TryCheckAbilityRequirements(ActivationData);

	// Remove on-removal modifiers of all abilities that would be cancelled if AbilityToQuery activates.
	for (FGameplayAbilityHandle Handle : CancelledHandles)
	{
		InstanceToCancel = GetAbilityInstanceFromHandle(Handle);
		ensure(InstanceToCancel);

		if (!InstanceToCancel)
		{
			continue;
		}

		if (AbilityInstance->bIsAuthoritativeCancel || InstanceToCancel->IsCancellable())
		{
			InstanceToCancel->TryRemoveAbilityEndedModifiers();
		}
	}

	return bCanActivateAbility;
}

bool UGameplaySystemComponent::UseAbility(TSubclassOf<UGameplayAbility> AbilityToUse, FGameplayAbilityHandle& OutHandle)
{
	FGameplayAbilityActivationData ActivationData;
	return UseAbility_Internal(AbilityToUse, ActivationData, OutHandle);
}

bool UGameplaySystemComponent::UseAbility_ActivationData(TSubclassOf<UGameplayAbility> AbilityToUse, const FGameplayAbilityActivationData& ActivationData, FGameplayAbilityHandle& OutHandle)
{
	return UseAbility_Internal(AbilityToUse, ActivationData, OutHandle);
}

bool UGameplaySystemComponent::UseAbility_NoRequirements(TSubclassOf<UGameplayAbility> AbilityToUse, FGameplayAbilityHandle& OutHandle)
{
	check(AbilityToUse);

	bool bOutCreatedNew = false;
	UGameplayAbility* AbilityInstance = GetOrCreateAbilityInstance(AbilityToUse, bOutCreatedNew);

	TArray<FGameplayAbilityHandle> HandlesToCancel;
	GetActiveAbilitiesByTags(AbilityInstance->CancelAbilitiesWithTag, HandlesToCancel, AbilityInstance);
	CancelAbilities(HandlesToCancel);

	FGameplayAbilityActivationData ActivationData;

	FGameplayAbilityHandle Handle = PreActivateAbility(AbilityInstance);
	OutHandle = Handle;
	FActiveGameplayAbility& ActiveAbility = ActiveAbilityMap.FindChecked(OutHandle);

	const bool bActivatedAbility = AbilityInstance->TryCommitActivateAbility(ActivationData, ActiveAbility);
	if (!bActivatedAbility)
	{
		GS_LOG(Warning, TEXT("Ability %s could not be activated in UseAbility_NoRequirements!"), *AbilityInstance->GetDisplayName());
	}

	ClearAbilityQueue();
	return bActivatedAbility;
}

UGameplayAbility* UGameplaySystemComponent::CreateNewAbilityInstance(TSubclassOf<UGameplayAbility> AbilityToSetup)
{
	check(AbilityToSetup);

	// Create a new instance of the ability and add it to the available abilities map
	UGameplayAbility* NewAbility = NewObject<UGameplayAbility>(this, AbilityToSetup);

	NewAbility->Init(GetOwner(), this);

	return NewAbility;
}

bool UGameplaySystemComponent::UseAbility_Internal(TSubclassOf<UGameplayAbility> AbilityToUse, const FGameplayAbilityActivationData& ActivationData, FGameplayAbilityHandle& OutHandle)
{
	check(AbilityToUse);

	const bool bCanActivate = CanActivateAbility(AbilityToUse, ActivationData);

	if (!bCanActivate)
	{
		QueueAbility(AbilityToUse);
		return false;
	}

	bool bOutCreatedNew = false;
	UGameplayAbility* AbilityInstance = GetOrCreateAbilityInstance(AbilityToUse, bOutCreatedNew);

	TArray<FGameplayAbilityHandle> HandlesToCancel;
	GetActiveAbilitiesByTags(AbilityInstance->CancelAbilitiesWithTag, HandlesToCancel, AbilityInstance);
	CancelAbilities(HandlesToCancel);

	ClearAbilityQueue();

	FGameplayAbilityHandle Handle = PreActivateAbility(AbilityInstance);
	OutHandle = Handle;
	FActiveGameplayAbility& ActiveAbility = ActiveAbilityMap.FindChecked(OutHandle);

	// While it's not guaranteed that this will be able to activate - ApplyAbilityRequirements can still fail - it would be a gameplay error anyways
	// and would still make runtime unreliable. We trust CheckAbilityRequirements as that is the most protection we can offer.
	return AbilityInstance->TryCommitActivateAbility(ActivationData, ActiveAbility);
}

FGameplayAbilityHandle UGameplaySystemComponent::PreActivateAbility(UGameplayAbility* AbilityInstance)
{
	// Move the instance from the old handle to the new handle
	if (AbilityInstance->GetInstancingPolicy() == EInstancingPolicy::EIP_InstancedPerActor)
	{
		const FGameplayAbilityHandle* ExistingHandlePtr = AbilityInstanceMap.FindKey(AbilityInstance);

		if (ExistingHandlePtr)
		{
			const FGameplayAbilityHandle ExistingHandle = *ExistingHandlePtr;
			const FActiveGameplayAbility* ActiveAbility = ActiveAbilityMap.Find(ExistingHandle);

			// The ability is still active, we don't want to move things around or invalidate the ActiveAbility.
			if (ActiveAbility && ActiveAbility->HasActiveState())
			{
				return FGameplayAbilityHandle();
			}

			AbilityInstanceMap.Remove(ExistingHandle);
			ActiveAbilityMap.Remove(ExistingHandle);
		}
	}

	FGameplayAbilityHandle NewHandle = FGameplayAbilityHandle::CreateNew();

	AbilityInstanceMap.Add(NewHandle, AbilityInstance);
	ActiveAbilityMap.Add(NewHandle, FActiveGameplayAbility(AbilityInstance, NewHandle));

	return NewHandle;
}

FOnAttributeChangedSignature& FDelegateCollection::GetDelegate(EAttributeType Attribute)
{
	CheckDelegates();

	bIsDirty = true;
	
	return DelegateMap.FindOrAdd(Attribute);
}

void FDelegateCollection::GetMultipleDelegates(const TArray<EAttributeType>& Attributes, TArray<FOnAttributeChangedSignature*>& OutDelegates)
{
	CheckDelegates();

	bIsDirty = true;

	for (EAttributeType SearchedKey : Attributes)
	{
		OutDelegates.Emplace(&DelegateMap.FindOrAdd(SearchedKey));
	}
}

bool FDelegateCollection::HasDelegate(EAttributeType Attribute)
{
	return DelegateMap.Find(Attribute) != nullptr;
}

void FDelegateCollection::Broadcast(EAttributeType Attribute)
{
	if (BroadcastLockCount > 0)
	{
		EnqueueBroadcast(Attribute);
		return;
	}

	if (bIsSilenced)
	{
		return;
	}

	FOnAttributeChangedSignature* Delegate = DelegateMap.Find(Attribute);
	if (Delegate)
	{
		Delegate->Broadcast(Attribute);
	}
}

inline void FDelegateCollection::BroadcastMultiple(const TArray<EAttributeType>& Attributes)
{
	if (BroadcastLockCount > 0)
	{
		EnqueueBroadcasts(Attributes);
		return;
	}

	if (bIsSilenced)
	{
		return;
	}

	for (EAttributeType Type : Attributes)
	{
		Broadcast(Type);
	}
}

void FDelegateCollection::CheckDelegates()
{
	if (!bIsDirty)
	{
		return;
	}

	TArray<EAttributeType> KeysToRemove;

	// Find all empty delegates
	for (auto It = DelegateMap.CreateIterator(); It; ++It)
	{
		if (!It->Value.IsBound())
		{
			KeysToRemove.Emplace(It->Key);
		}
	}

	// Remove all empty delegates
	for (EAttributeType Key : KeysToRemove)
	{
		DelegateMap.Remove(Key);
	}

	bIsDirty = false;
}

void FDelegateCollection::EnqueueBroadcast(EAttributeType Attribute)
{
	QueuedBroadcasts.AddUnique(Attribute);
}

void FDelegateCollection::EnqueueBroadcasts(const TArray<EAttributeType>& Attributes)
{
	for (EAttributeType Type : Attributes)
	{
		EnqueueBroadcast(Type);
	}
}

void FDelegateCollection::BroadcastQueue()
{
	if (bIsSilenced)
	{
		return;
	}

	BroadcastMultiple(QueuedBroadcasts);
}

