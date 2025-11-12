// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "GameplaySystemComponent.h"

#include "AttributeDataSet.h"
#include "AttributeEffect.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplaySaveGameTypes.h"
#include "GameplayTagDefines.h"
#include "DevelopmentTypes.h"

UGameplaySystemComponent::UGameplaySystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGameplaySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeAttributes();

	InitializeLevelSystem();

	InitializeGameplayTags();

	InitializeGameplaySystemActorInfo(); // Call before GameplayAbilities in case they need ActorInfo for their own initialization

	InitializeGameplayAbilities();

	// Applying the movement value to the Component at start
	if (UCharacterMovementComponent* MovementComponent = GetOwner()->FindComponentByClass<UCharacterMovementComponent>())
	{
		MovementComponent->MaxWalkSpeed = GetAttributeValue(EAttributeType::EAT_MovementSpeed);
	}
}

void UGameplaySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (GameplaySystemActorInfo.IsValid())
	{
		GameplaySystemActorInfo->ClearActorInfo();
		GameplaySystemActorInfo.Reset();
	}

	Super::EndPlay(EndPlayReason);
}

void UGameplaySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TArray<FGameplayEffectHandle> EffectsToRemove;

	for(auto It = ActiveGameplayEffects.CreateIterator(); It; ++It)
	{
		FActiveGameplayEffect& Effect = It->Value;

		Effect.TickGameplayEffect(DeltaTime);

		// We always check the Period first, to apply the Effects before removal if both events coincide
		if (Effect.IsPeriodPassed())
		{
			for (FAttributeEffect& AttributeEffect : Effect.AttributeEffects)
			{
				// Force instant application, since the GameplayEffect can't keep track and correctly remove the duplicate AttributeEffect instances 
				ApplyAttributeEffectNoRemoval(AttributeEffect);
			}
		}

		if (Effect.IsExpired())
		{
			EffectsToRemove.Emplace(It.Key());
		}
	}

	// Remove outside the for-loop
	for (FGameplayEffectHandle& Effect : EffectsToRemove)
	{
		RemoveGameplayEffectByHandle(Effect);
	}

	TArray<UGameplayAbility*> ExpiredAbilities;
	for (auto& [PairInstance, PairHandle] : ActiveAbilitesTable)
	{
		PairHandle.Tick(DeltaTime);

		const bool bHasDurationElapsed = PairHandle.bHasDurationElapsed;

		if (bHasDurationElapsed)
		{
			// Inform the ability that its duration has ended
			EndAbility(PairHandle.GameplayAbility);
		}

		// We only end the ability if both the cooldown AND duration have elapsed, in the case that the ability has shorter cooldown than duration.
		if (bHasDurationElapsed && PairHandle.bHasCooldownElapsed)
		{
			//Mark for deletion
			ExpiredAbilities.Emplace(PairInstance);
		}
	}

	// Clean up expired abilities
	for (const UGameplayAbility* Ability : ExpiredAbilities)
	{
		ActiveAbilitesTable.Remove(Ability);
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
	// First, assume that no Attribute has an Active Effect, and reset the CurrentValue
	for (TPair<EAttributeType, FAttribute>& AttributePair : Attributes)
	{
		FAttribute& Attribute = AttributePair.Value;
		Attribute.CurrentValue = Attribute.BaseValue;
	}

	// Sort such that the Effects are in the order of the Enums in EEffectApplicationType
	ActiveAttributeEffects.Sort([](const FAttributeEffect& A, const FAttributeEffect& B)
		{
			// Higher ApplicationType Enum values are done last
			return A.ApplicationType < B.ApplicationType;
		});

	// Now, iterate through all ActiveEffects and apply their change to the corresponding Attribute
	for (FAttributeEffect& Effect : ActiveAttributeEffects)
	{
		EAttributeType EffectAttribute = Effect.Attribute;
		FAttribute* AffectedAttribute = Attributes.Find(EffectAttribute);

		// No corresponding Attribute found to apply to, skip
		if (!AffectedAttribute)
		{
			continue;
		}

		Effect.ApplyAttributeEffect(*AffectedAttribute);
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
	for (const auto& AttributePair : AttributeDataSet->Attributes)
	{
		const FEditorAttribute& EditorAttribute = AttributePair.Value;

		FAttribute ParsedAttribute = { AttributePair.Key, EditorAttribute.BaseValue, EditorAttribute.CurrentValue };

		Attributes.Add(ParsedAttribute.AttributeType, ParsedAttribute);
	}

	bIsDirty = true;

	return;
}

void UGameplaySystemComponent::InitializeGameplayTags()
{
	GameplayTagSystem.AppendTags(StartUpTags);
}

void UGameplaySystemComponent::InitializeGameplayAbilities()
{
	// Set up all in-editor specified abilities
	TArray<TSubclassOf<UGameplayAbility>> AbilitiesCopy;

	for (auto& Ability : AvailableAbilities)
	{
		AbilitiesCopy.Emplace(Ability.Key);
	}

	for (auto& Ability : AbilitiesCopy)
	{
		CreateAbilityInstance(Ability);
	}
}

void UGameplaySystemComponent::InitializeGameplaySystemActorInfo()
{
	// Allocate a new FGameplaySystemActorInfo 
	GameplaySystemActorInfo = MakeShared<FGameplaySystemActorInfo>();
	GameplaySystemActorInfo->Init(GetOwner(), this);
}

float UGameplaySystemComponent::GetAttributeValue(EAttributeType AttributeType)
{
	if (bIsDirty)
	{
		EvaluateAttributes();
	}

	// FindRef avoids a null ptr issue that would otherwise arise on a default Find, or a FindOrAdd that recreates a missing value
	return Attributes.FindRef(AttributeType).CurrentValue;
}

float UGameplaySystemComponent::GetAttributeBaseValue(EAttributeType AttributeType)
{
	if (bIsDirty)
	{
		EvaluateAttributes();
	}

	return Attributes.FindRef(AttributeType).BaseValue;
}

void UGameplaySystemComponent::ModifyAttributeBaseValue(EAttributeType AttributeType, float ValueChange)
{
	FAttribute* Attribute = Attributes.Find(AttributeType);

	if (Attribute)
	{
		Attribute->BaseValue += ValueChange;

		bIsDirty = true;

		if (bChangeSilently == false)
		{
			if (OnAttributeChangedDelegateCollection.HasDelegate(AttributeType))
			{
				OnAttributeChangedDelegateCollection.GetDelegate(AttributeType).Broadcast(AttributeType);
			}
		}
	}
}

void UGameplaySystemComponent::ModifyAttributeValue(EAttributeType AttributeType, float ValueChange)
{
	FAttribute* Attribute = Attributes.Find(AttributeType);

	if (Attribute)
	{
		Attribute->CurrentValue += ValueChange;

		bIsDirty = true;

		if (bChangeSilently == false)
		{
			if (OnAttributeChangedDelegateCollection.HasDelegate(AttributeType))
			{
				OnAttributeChangedDelegateCollection.GetDelegate(AttributeType).Broadcast(AttributeType);
			}
		}
	}
}

void UGameplaySystemComponent::SetAttributeBaseValue(EAttributeType AttributeType, float NewValue)
{
	FAttribute* Attribute = Attributes.Find(AttributeType);

	if (Attribute)
	{
		Attribute->BaseValue = NewValue;

		bIsDirty = true;

		if (bChangeSilently == false)
		{
			if (OnAttributeChangedDelegateCollection.HasDelegate(AttributeType))
			{
				OnAttributeChangedDelegateCollection.GetDelegate(AttributeType).Broadcast(AttributeType);
			}
		}
	}
}

void UGameplaySystemComponent::SetAttributeValue(EAttributeType AttributeType, float NewValue)
{
	FAttribute* Attribute = Attributes.Find(AttributeType);

	if (Attribute)
	{
		Attribute->CurrentValue = NewValue;

		bIsDirty = true;

		if (bChangeSilently == false)
		{
			if (OnAttributeChangedDelegateCollection.HasDelegate(AttributeType))
			{
				OnAttributeChangedDelegateCollection.GetDelegate(AttributeType).Broadcast(AttributeType);
			}
		}
	}
}

void UGameplaySystemComponent::ClampAttributeBaseValue(EAttributeType AttributeType, float Min, float Max)
{
	FAttribute* Attribute = Attributes.Find(AttributeType);

	if (Attribute)
	{
		const float ValueBefore = Attribute->BaseValue;
		Attribute->BaseValue = FMath::Clamp(Attribute->BaseValue, Min, Max);

		// No change in value, no need to broadcast or recalculate
		if (ValueBefore == Attribute->BaseValue)
		{
			return;
		}

		bIsDirty = true;

		if (bChangeSilently == false)
		{
			if (OnAttributeChangedDelegateCollection.HasDelegate(AttributeType))
			{
				OnAttributeChangedDelegateCollection.GetDelegate(AttributeType).Broadcast(AttributeType);
			}
		}
	}
}

bool UGameplaySystemComponent::HasAttributeType(EAttributeType AttributeType)
{
	return Attributes.Contains(AttributeType);
}

void UGameplaySystemComponent::ApplyAttributeEffect(FAttributeEffect EffectToApply)
{
	ActiveAttributeEffects.Add(EffectToApply);

	//If it has the affected Attribute, prompt a recalculation
	if (HasAttributeType(EffectToApply.Attribute))
	{
		bIsDirty = true;

		if (bChangeSilently == false)
		{
			if (OnAttributeChangedDelegateCollection.HasDelegate(EffectToApply.Attribute))
			{
				OnAttributeChangedDelegateCollection.GetDelegate(EffectToApply.Attribute).Broadcast(EffectToApply.Attribute);
			}
		}
	}
}

void UGameplaySystemComponent::ApplyAttributeEffectNoRemoval(FAttributeEffect EffectToApply)
{
	FAttribute* AffectedAttribute = Attributes.Find(EffectToApply.Attribute);

	// No corresponding Attribute found to apply to, skip
	if (!AffectedAttribute)
	{
		return;
	}

	EffectToApply.ApplyAttributeEffect(*AffectedAttribute);
	bIsDirty = true;

	if (bChangeSilently == false)
	{
		if (OnAttributeChangedDelegateCollection.HasDelegate(EffectToApply.Attribute))
		{
			OnAttributeChangedDelegateCollection.GetDelegate(EffectToApply.Attribute).Broadcast(EffectToApply.Attribute);
		}
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

		if (bChangeSilently == false)
		{
			if (OnAttributeChangedDelegateCollection.HasDelegate(EffectToRemove.Attribute))
			{
				OnAttributeChangedDelegateCollection.GetDelegate(EffectToRemove.Attribute).Broadcast(EffectToRemove.Attribute);
			}
		}
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

	if (bChangeSilently == false)
	{
		if (OnAttributeChangedDelegateCollection.HasDelegate(Attribute.AttributeType))
		{
			OnAttributeChangedDelegateCollection.GetDelegate(Attribute.AttributeType).Broadcast(Attribute.AttributeType);
		}
	}
}

void UGameplaySystemComponent::SimulateAttributes(const TArray<FAttributeEffect>& AttributeEffectsToSimulate, TMap<EAttributeType, FAttribute>& GeneratedAttributesOut)
{
	// We dont want other systems to try and react to these changes
	bChangeSilently = true;

	TMap<EAttributeType, FAttribute> OriginalAttributes;
	TArray<FAttributeEffect> OriginalActiveEffects;

	GetAttributes(OriginalAttributes);
	GetActiveAttributeEffects(OriginalActiveEffects);

	// Apply the AttributeEffectsToSimulate
	for (const FAttributeEffect& Effect : AttributeEffectsToSimulate)
	{
		ApplyAttributeEffect(Effect);
	}

	// Get all affected Attributes
	for (const FAttributeEffect& Effect : AttributeEffectsToSimulate)
	{
		// Calling GetValue functions to prompt a recalculation to account for the added AttributeEffects
		FAttribute ChangedAttribute = { Effect.Attribute, GetAttributeBaseValue(Effect.Attribute), GetAttributeValue(Effect.Attribute) };
		GeneratedAttributesOut.Add(Effect.Attribute, ChangedAttribute);
	}

	// Restore state
	SetAttributes(OriginalAttributes);
	SetActiveAttributeEffects(OriginalActiveEffects);

	bChangeSilently = false;
}

int UGameplaySystemComponent::GetEntityLevel() const
{
	return EntityLevel;
}

void UGameplaySystemComponent::SetEntityLevel(const int& Level, bool bDoSilently)
{
	const int LevelBefore = EntityLevel;

	if (!LevelScalingCurveTable)
	{
		GS_LOG(Error, TEXT("Error - No assigned Curve Table."));
		return;
	}

	// No negative levels allowed
	if (Level < 0)
	{
		GS_LOG(Warning, TEXT("Level cannot be negative."));
		return;
	}

	// Indicates that the Entity does not want to use the Level system
	if (EntityLevel == 0 || Level == 0)
	{
		return;
	}

	// Assigned here to avoid a bug where level 1 entities are ignored, since their level is already before this call in the initialisation process.
	FName RowName = FName("ExpForNextLevel");

	FSimpleCurve* ExperienceCurve = LevelScalingCurveTable->FindSimpleCurve(RowName, TEXT("GetExperienceForLevel"));

	if (ExperienceCurve == nullptr)
	{
		GS_LOG(Error, TEXT("GameplaySystemComponent: Error - No curve found for LevelSystem"));
		return;
	}

	CachedExpForNextLevel = ExperienceCurve->Eval(EntityLevel);

	// Since we are changing by adding the Delta between Level values, this would result in a net-zero change
	if (Level == EntityLevel)
	{
		return;
	}

	FString LeftString;
	FString RightString;

	// Scaling all available Attributes up to the new Level value
	for (auto& Elem : Attributes)
	{
		// These are modified and scaled later, skip for now
		if (Elem.Key == EAttributeType::EAT_Health || Elem.Key == EAttributeType::EAT_Charge)
		{
			continue;
		}

		FString EnumString = UEnum::GetValueAsString(Elem.Key);
		EnumString.Split(TEXT("::EAT_"), &LeftString, &RightString);
		FName EnumName = FName(*RightString);

		FSimpleCurve* ScalingCurve = LevelScalingCurveTable->FindSimpleCurve(EnumName, TEXT("GetAttributeScalingValue"));

		if (ScalingCurve == nullptr)
		{
			FString Message = "Error - No curve found for " + EnumString;
			GS_LOG(Error, TEXT("%s"), *Message);
			continue;
		}

		const float ScalingValue = ScalingCurve->Eval(Level);

		const float PreviousScalingValue = ScalingCurve->Eval(EntityLevel);

		float OriginalValue = AttributeDataSet->Attributes[Elem.Key].BaseValue;

		// Get the Delta value from the prior level to the new one
		const float DeltaValue = OriginalValue * (ScalingValue - PreviousScalingValue);

		Elem.Value.BaseValue += DeltaValue;

		// When Max... Attributes are changed we want to scale the nonMax values up by Delta too, to keep the value before and after at the same % relative to Max... . 
		switch (Elem.Key)
		{
		case EAttributeType::EAT_MaxHealth:
		{
			FAttribute* Ptr = Attributes.Find(EAttributeType::EAT_Health);

			if (Ptr)
			{
				Ptr->BaseValue += DeltaValue;
			}
			break;
		}
		case EAttributeType::EAT_MaxCharge:
		{
			FAttribute* Ptr = Attributes.Find(EAttributeType::EAT_Charge);
			if (Ptr)
			{
				Ptr->BaseValue += DeltaValue;
			}
			break;
		}
		}
	}

	bIsDirty = true;
	EntityLevel = Level;

	// Invoke last in case any bound event relies on the new level system state
	if (bDoSilently == false)
	{
		OnLeveledUpDelegate.Broadcast(LevelBefore /*Level before levelups*/, EntityLevel /*Current Level*/, 0);
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
		if (TotalExperience >= CachedExpForNextLevel)
		{
			Experience -= CachedExpForNextLevel;

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
		OnLeveledUpDelegate.Broadcast(LevelBefore /*Level before levelups*/, EntityLevel /*Current Level*/, CachedExpForNextLevel /*Exp required for next level*/);
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
	return CachedExpForNextLevel;
}

float UGameplaySystemComponent::GetExperienceRemainingForNextLevel() const
{
	return CachedExpForNextLevel - EntityExperience;
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

	for (auto It = GetConstGameplayEffectIterator(); It; ++It)
	{
		if (It.Value().GetDefinition() == EffectToRemove)
		{
			return RemoveGameplayEffectByHandle(It.Key());
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

bool UGameplaySystemComponent::AddGameplayEffectByHandle(const FActiveGameplayEffect& EffectToAdd, FGameplayEffectHandle& OutHandle)
{
	return AddGameplayEffect_Internal(EffectToAdd, OutHandle);
}

bool UGameplaySystemComponent::RemoveGameplayEffectFromType(TSubclassOf<UGameplayEffect> EffectToRemove)
{
	check(EffectToRemove);

	for (auto It = GetConstGameplayEffectIterator(); It; ++It)
	{
		if (It->Value.GetDefinition()->IsA(EffectToRemove))
		{
			return RemoveGameplayEffectByHandle(It->Key);
		}
	}

	return false;
}

bool UGameplaySystemComponent::AddGameplayEffect_Internal(const FActiveGameplayEffect& EffectToAdd, FGameplayEffectHandle& OutHandle)
{
	check(EffectToAdd.GetDefinition());

	const bool bAppliedSuccessfully = EffectToAdd.GetDefinition()->ApplyGameplayEffect(this, GetOwner());
	if (bAppliedSuccessfully)
	{
		// Instant effects are not stored, since they do not need a state and are not removable after application
		if (EffectToAdd.DurationType != EDurationType::EDT_Instant)
		{
			FGameplayEffectHandle GeneratedHandle = {};

			OutHandle = GeneratedHandle;
			ActiveGameplayEffects.Add(GeneratedHandle, EffectToAdd);
		}
	}
	return bAppliedSuccessfully;
}

bool UGameplaySystemComponent::RemoveGameplayEffectByHandle(const FGameplayEffectHandle& EffectToRemove)
{
	FActiveGameplayEffect* FoundEffect = ActiveGameplayEffects.Find(EffectToRemove);

	if (FoundEffect)
	{
		FoundEffect->OnGameplayEffectRemoved(this, GetOwner());

		ActiveGameplayEffects.Remove(EffectToRemove);
		return true;
	}
	
	return false;
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
		return Effect.Name == Name;
	};

	const int RemovedCount = RemoveAllGameplayEffectsByPredicate(CompareNames);

	return (bool)RemovedCount;
}

bool UGameplaySystemComponent::RemoveAllGameplayEffectsWithTag(const FGameplayTag& TagToRemove)
{
	auto HasTag = [TagToRemove](const FActiveGameplayEffect& Effect)
	{
		return Effect.TagsOnEffect.HasTag(TagToRemove);
	};

	const int RemovedCount = RemoveAllGameplayEffectsByPredicate(HasTag);

	return (bool)RemovedCount;
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

bool UGameplaySystemComponent::HasGameplayEffectOfInstance(const UGameplayEffect* EffectToCheck, FGameplayEffectHandle& OutHandle) const
{
	for (auto It = GetConstGameplayEffectIterator(); It; ++It)
	{
		if (It->Value.GetDefinition() == EffectToCheck)
		{
			OutHandle = It->Key;
			return true;
		}
	}

	return false;
}

TMap<FGameplayEffectHandle, FActiveGameplayEffect>::TConstIterator UGameplaySystemComponent::GetConstGameplayEffectIterator() const
{
	return TMap<FGameplayEffectHandle, FActiveGameplayEffect>::TConstIterator(ActiveGameplayEffects);
}

void UGameplaySystemComponent::K2_GetGameplayTagSystem(FGameplayTagSystem& OutGameplayTagSystem) const
{
	OutGameplayTagSystem = GameplayTagSystem;
}

void UGameplaySystemComponent::ApplyBlockingAndCancellingTags(const FGameplayTagContainer& BlockingTags, const FGameplayTagContainer& CancellingTags)
{
	BlockedAbilityTags.AppendTags(BlockingTags);

	CancelAllAbilitiesWithTags(CancellingTags);
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

bool UGameplaySystemComponent::GetAbilityHandleFromInstance(UGameplayAbility* AbilityInstance, FActiveGameplayAbility& OutHandle)
{
	FActiveGameplayAbility* AbilityHandle = ActiveAbilitesTable.Find(*AbilityInstance);
	if (!AbilityHandle)
	{
		return false;
	}

	OutHandle = *AbilityHandle;
	return true;
}

FActiveGameplayAbility* UGameplaySystemComponent::GetAbilityHandlePtrFromInstance(UGameplayAbility* AbilityInstance)
{
	return ActiveAbilitesTable.Find(*AbilityInstance);
}

void UGameplaySystemComponent::GetAllAbilitiesWithTag(FGameplayTag TagToCheck, TArray<UGameplayAbility*>& AbilitiesOut) const
{
	auto HasRequiredTag = [TagToCheck](const UGameplayAbility* Ability)
		{
			return Ability->GetAbilityTags().HasTag(TagToCheck);
		};

	GetAllAbilitiesByPredicate(HasRequiredTag, AbilitiesOut);
}

void UGameplaySystemComponent::GetAllActiveAbilitiesWithTag(FGameplayTag TagToCheck, TArray<UGameplayAbility*>& AbilitiesOut) const
{
	auto HasRequiredTag = [TagToCheck](const UGameplayAbility* Ability)
		{
			return Ability->GetAbilityTags().HasTag(TagToCheck);
		};

	GetAllActiveAbilitiesByPredicate(HasRequiredTag, AbilitiesOut);
}

void UGameplaySystemComponent::GetAllAbilitiesWithTags(const FGameplayTagContainer& TagsToCheck, TArray<UGameplayAbility*>& AbilitiesOut) const
{
	auto HasRequiredTags = [TagsToCheck](const UGameplayAbility* Ability)
		{
			return Ability->GetAbilityTags().HasAny(TagsToCheck);
		};

	GetAllAbilitiesByPredicate(HasRequiredTags, AbilitiesOut);
}

void UGameplaySystemComponent::GetAllActiveAbilitiesWithTags(const FGameplayTagContainer& TagsToCheck, TArray<UGameplayAbility*>& AbilitiesOut) const
{
	auto HasRequiredTags = [TagsToCheck](const UGameplayAbility* Ability)
		{
			return Ability->GetAbilityTags().HasAny(TagsToCheck);
		};

	GetAllActiveAbilitiesByPredicate(HasRequiredTags, AbilitiesOut);
}

void UGameplaySystemComponent::GetAllAbilitiesByPredicate(std::function<bool(const UGameplayAbility*)> Predicate, TArray<UGameplayAbility*>& AbilitiesOut) const
{
	for (auto It = GetConstAbilityIterator(); It; ++It)
	{
		if (Predicate(It->Value))
		{
			AbilitiesOut.Add(It->Value);
		}
	}
}

void UGameplaySystemComponent::GetAllActiveAbilitiesByPredicate(std::function<bool(const UGameplayAbility*)> Predicate, TArray<UGameplayAbility*>& AbilitiesOut) const
{
	for (auto It = GetConstActiveAbilityIterator(); It; ++It)
	{
		if (Predicate(It->Key))
		{
			AbilitiesOut.Add(It->Key);
		}
	}
}

int UGameplaySystemComponent::GetAbilityCount() const
{
	return AvailableAbilities.Num();
}

int UGameplaySystemComponent::GetActiveAbilityCount() const
{
	return ActiveAbilitesTable.Num();
}

void UGameplaySystemComponent::ActivateQueuedAbility()
{
	if (QueuedAbility)
	{
		FActiveGameplayAbility Handle;
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

bool UGameplaySystemComponent::CancelAbility(UGameplayAbility* AbilityToCancel, bool bIsAuthoritative)
{
	if (!AbilityToCancel)
	{
		return false;
	}

	FActiveGameplayAbility* AbilityHandle = ActiveAbilitesTable.Find(AbilityToCancel);

	// Ability is not active, nothing to cancel
	if (!AbilityHandle)
	{
		return false;
	}

	// If not authoritative, only cancellable abilities can be cancelled
	if(!bIsAuthoritative && !AbilityToCancel->IsCancellable())
	{
		return false;
	}

	if (AbilityHandle->IsAbilityActive())
	{
		AbilityHandle->CancelAbility();

		RemoveBlockingTags(AbilityToCancel->BlockAbilitiesWithTag);

		if (AbilityToCancel == GetAnimatingAbility())
		{
			ClearAnimMontageInfo();
		}
		
		return true;
	}

	return false;
}

bool UGameplaySystemComponent::CancelAbilities(TArray<UGameplayAbility*> AbilitiesToCancel, bool bIsAuthoritative)
{
	bool bCancelledAnAbility = false;

	for (UGameplayAbility* AbilityToCancel : AbilitiesToCancel)
	{
		const bool bWasCancelled = CancelAbility(AbilityToCancel, bIsAuthoritative);

		if(bWasCancelled)
		{
			bCancelledAnAbility = true;
		}
	}

	return bCancelledAnAbility;
}

bool UGameplaySystemComponent::CancelAllAbilitiesWithTag(FGameplayTag Tag, bool bIsAuthoritative)
{
	bool bCancelledAnAbility = false;

	TArray<TObjectPtr<UGameplayAbility>> AbilitiesToCancel;

	for(auto It = ActiveAbilitesTable.CreateConstIterator(); It; ++It)
	{
		if (It->Value.AbilityTags.HasTag(Tag))
		{
			AbilitiesToCancel.Add(It.Key());
		}
	}

	// CancelAbility removes the AbilityHandle when cancelling, so we need the separate array to iterate over.
	for (TObjectPtr<UGameplayAbility> AbilityToCancel : AbilitiesToCancel)
	{
		FActiveGameplayAbility* AbilityHandle = ActiveAbilitesTable.Find(AbilityToCancel);

		const bool bWasCancelled = CancelAbility(AbilityToCancel, bIsAuthoritative);

		if(bWasCancelled)
		{
			bCancelledAnAbility = true;
		}
	}

	return bCancelledAnAbility;
}

bool UGameplaySystemComponent::CancelAllAbilitiesWithTags(const FGameplayTagContainer& Tags, bool bIsAuthoritative)
{
	bool bCancelledAnAbility = false;

	TArray<TObjectPtr<UGameplayAbility>> AbilitiesToCancel;

	for (auto It = ActiveAbilitesTable.CreateConstIterator(); It; ++It)
	{
		if (It->Value.AbilityTags.HasAny(Tags))
		{
			AbilitiesToCancel.Add(It.Key());
		}
	}

	// CancelAbility removes the AbilityHandle when cancelling, so we need the separate array to iterate over.
	for (TObjectPtr<UGameplayAbility> AbilityToCancel : AbilitiesToCancel)
	{
		FActiveGameplayAbility* AbilityHandle = ActiveAbilitesTable.Find(AbilityToCancel);

		const bool bWasCancelled = CancelAbility(AbilityToCancel, bIsAuthoritative);

		if (bWasCancelled)
		{
			bCancelledAnAbility = true;
		}
	}

	return bCancelledAnAbility;
}

void UGameplaySystemComponent::EndAbility(UGameplayAbility* AbilityToEnd)
{
	FActiveGameplayAbility* AbilityHandle = GetAbilityHandlePtrFromInstance(AbilityToEnd);

	check(AbilityHandle);

	AbilityHandle->EndAbility();

	RemoveBlockingTags(AbilityToEnd->BlockAbilitiesWithTag);

	if (AbilityToEnd == GetAnimatingAbility())
	{
		ClearAnimMontageInfo();
	}
}

void UGameplaySystemComponent::EndAbilityEarly(UGameplayAbility* AbilityToEnd)
{
	EndAbility(AbilityToEnd);
}

FGameplaySystemActorInfo* UGameplaySystemComponent::GetActorInfo() const
{
	check(GameplaySystemActorInfo.IsValid());

	return GameplaySystemActorInfo.Get();
}


TMap<TSubclassOf<UGameplayAbility>, TObjectPtr<UGameplayAbility>>::TConstIterator UGameplaySystemComponent::GetConstAbilityIterator() const
{
	return TMap<TSubclassOf<UGameplayAbility>, TObjectPtr<UGameplayAbility>>::TConstIterator(AvailableAbilities);
}

TMap<TObjectPtr<UGameplayAbility>, FActiveGameplayAbility>::TConstIterator UGameplaySystemComponent::GetConstActiveAbilityIterator() const
{
	return TMap<TObjectPtr<UGameplayAbility>, FActiveGameplayAbility>::TConstIterator(ActiveAbilitesTable);
}

float UGameplaySystemComponent::PlayMontage(UGameplayAbility* PlayingAbility, UAnimMontage* MontageToPlay, float PlayRate, FName StartSection, FName EndSection)
{
	check(PlayingAbility);
	check(MontageToPlay);

	AnimMontageInfo.AssignMontage(MontageToPlay, PlayingAbility);

	UAnimInstance* AnimInstance = GetActorInfo()->GetAnimInstance();
	check(AnimInstance);

	const float AnimDuration = AnimInstance->Montage_Play(MontageToPlay, PlayRate);

	// StartSection takes priority if it's supplied
	// Make sure that the FName is valid and that it points to a valid Section
	if (!StartSection.IsNone() && MontageToPlay->IsValidSectionName(StartSection))
	{
		AnimInstance->Montage_JumpToSection(StartSection, MontageToPlay);
	}
	else if (!EndSection.IsNone() && MontageToPlay->IsValidSectionName(EndSection))
	{
		AnimInstance->Montage_JumpToSectionsEnd(EndSection, MontageToPlay);
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

TMap<TObjectPtr<UGameplayAbility>, FActiveGameplayAbility>::TIterator UGameplaySystemComponent::GetActiveAbilityIterator()
{
	return TMap<TObjectPtr<UGameplayAbility>, FActiveGameplayAbility>::TIterator(ActiveAbilitesTable);
}

bool UGameplaySystemComponent::GetAbilityHandle(TSubclassOf<UGameplayAbility> Ability, FActiveGameplayAbility& OutHandle)
{
	UGameplayAbility* FoundAbility = GetAbilityInstance(Ability);

	if (!FoundAbility)
	{
		return false;
	}

	FActiveGameplayAbility* AbilityHandle = ActiveAbilitesTable.Find(FoundAbility);
	if (!AbilityHandle)
	{
		return false;
	}

	OutHandle = *AbilityHandle;
	return true;
}

void UGameplaySystemComponent::AddAbilityHandle(UGameplayAbility* AbilityInstance, const FActiveGameplayAbility& Handle)
{
	ActiveAbilitesTable.Add(AbilityInstance, Handle);
}

UGameplayAbility* UGameplaySystemComponent::GetOrAddAbilityInstance(TSubclassOf<UGameplayAbility> Ability, bool& OutGeneratedNewHandle)
{
	check(Ability);

	UGameplayAbility* AbilityInstance = AvailableAbilities.FindRef(Ability);
	OutGeneratedNewHandle = false;

	if (!AbilityInstance)
	{
		// No instance has been generated yet, make one now
		CreateAbilityInstance(Ability);
		AbilityInstance = AvailableAbilities.FindRef(Ability);

		OutGeneratedNewHandle = true;
	}

	return AbilityInstance;
}

UGameplayAbility* UGameplaySystemComponent::GetAbilityInstance(TSubclassOf<UGameplayAbility> Ability)
{
	TObjectPtr<UGameplayAbility>* AbilityInstance = AvailableAbilities.Find(Ability);

	if (AbilityInstance)
	{
		return *AbilityInstance;
	}

	return nullptr;
}

bool UGameplaySystemComponent::HasCooldown(const UGameplayAbility* AbilityToQuery) const
{
	// Each instance has a different pointer, so Find() won't return anything. We need to verify by comparing against the CDO instead.
	if (AbilityToQuery->GetInstancingPolicy() == EInstancingPolicy::EIP_InstancedPerExecution)
	{
		const UGameplayAbility* InstanceCDO = AbilityToQuery->GetClass()->GetDefaultObject<UGameplayAbility>();

		for (auto It = GetConstActiveAbilityIterator(); It; ++It)
		{
			if (It->Key->GetClass()->GetDefaultObject() == InstanceCDO)
			{
				return It->Value.IsAbilityActive();
			}
		}

		return false;
	}

	if (const FActiveGameplayAbility* ActiveAbility = ActiveAbilitesTable.Find(AbilityToQuery))
	{
		return ActiveAbility->IsAbilityActive();
	}

	return false;
}

bool UGameplaySystemComponent::UseAbility(TSubclassOf<UGameplayAbility> AbilityToUse, FActiveGameplayAbility& GeneratedHandle)
{
	check(AbilityToUse);

	const UGameplayAbility* DefaultAbility = AbilityToUse->GetDefaultObject<UGameplayAbility>();

	// --- Start activation routine

	bool bCreatedNewInstance;
	UGameplayAbility* AbilityInstance = GetOrAddAbilityInstance(AbilityToUse, bCreatedNewInstance);
	const EInstancingPolicy InstancingPolicy = DefaultAbility->GetInstancingPolicy();

	// Follow InstancingPolicy by creating a new one per execution if needed
	if (!bCreatedNewInstance && InstancingPolicy == EInstancingPolicy::EIP_InstancedPerExecution)
	{
		CancelAbility(AbilityInstance); // Make sure we don't leave a active instance around
		CreateAbilityInstance(AbilityToUse);
	}

	TArray<UGameplayAbility*> AbilitiesToCancel;
	GetAllActiveAbilitiesWithTags(AbilityInstance->CancelAbilitiesWithTag, AbilitiesToCancel);

	// Apply on-removal modifiers of all abilities that would be cancelled if it activates.
	for (UGameplayAbility* Ability : AbilitiesToCancel)
	{
		// Only apply on-removal modifiers if the ability allows cancelling, or if the cancel itself is authoritative.
		if (AbilityInstance->bIsAuthoritativeCancel || Ability->IsCancellable())
		{
			Ability->TryApplyAbilityEndedModifiers();
		}
	}

	// Try to activate the ability now that we are in the same state that would be after the to-be abilities are cancelled
	const bool bActivatedAbility = AbilityInstance->AttemptActivateAbility(GeneratedHandle);
	
	if(!bActivatedAbility)
	{
		// Undo on-removal modifiers of all abilities that would have been cancelled, since we couldn't activate.
		for (UGameplayAbility* Ability : AbilitiesToCancel)
		{

			if (AbilityInstance->bIsAuthoritativeCancel || Ability->IsCancellable())
			{
				Ability->TryRemoveAbilityEndedModifiers();
			}
		}

		QueueAbility(AbilityToUse);
	}

	return bActivatedAbility;
}

// TODO: Update to match the UseAbility implementation with the new cancellation logic
void UGameplaySystemComponent::UseAbility_NoRequirements(TSubclassOf<UGameplayAbility> AbilityToUse, FActiveGameplayAbility& GeneratedHandle)
{
	checkNoEntry(); // NOT IMPLEMENTED PROPERLY YET, SEE TODO ABOVE!

	check(AbilityToUse);

	const UGameplayAbility* DefaultAbility = AbilityToUse->GetDefaultObject<UGameplayAbility>();

	// --- Check activation tags

	CancelAllAbilitiesWithTags(DefaultAbility->CancelAbilitiesWithTag);

	// --- Start activation routine
	bool bCreatedNewInstance;
	UGameplayAbility* AbilityInstance = GetOrAddAbilityInstance(AbilityToUse, bCreatedNewInstance);

	const EInstancingPolicy InstancingPolicy = DefaultAbility->GetInstancingPolicy();

	// Uphold InstancingPolicy by creating a new one per execution if needed
	if (!bCreatedNewInstance && InstancingPolicy == EInstancingPolicy::EIP_InstancedPerExecution)
	{
		CancelAbility(AbilityInstance); // Make sure we don't leave a active instance around
		CreateAbilityInstance(AbilityToUse);
	}

	// Force the activation by calling the internal function directly, bypassing the requirements check
	AbilityInstance->ActivateAbility(GeneratedHandle);

	if (InstancingPolicy != EInstancingPolicy::EIP_NoLifetime)
	{
		// Add the handle to the active abilities table to allow lookups, cancelling and state-management.
		ActiveAbilitesTable.Add(AbilityInstance, GeneratedHandle);
	}
}

void UGameplaySystemComponent::AddAbility(TSubclassOf<UGameplayAbility> AbilityToAdd)
{
	check(AbilityToAdd);

	if (AvailableAbilities.Contains(AbilityToAdd))
	{
		return;
	}

	AvailableAbilities.Add(AbilityToAdd, nullptr);
	CreateAbilityInstance(AbilityToAdd);
}

bool UGameplaySystemComponent::RemoveAbility(TSubclassOf<UGameplayAbility> AbilityToRemove)
{
	check(AbilityToRemove);

	// Check if the ability has an instance assigned
	UGameplayAbility* AbilityInstance = GetAbilityInstance(AbilityToRemove);

	if (!AbilityInstance)
	{
		return false;
	}

	// Check if the ability has a handle
	FActiveGameplayAbility Handle;
	const bool bHandleFound = GetAbilityHandle(AbilityToRemove, Handle);

	if (bHandleFound)
	{
		return false;
	}

	// The ability is added and an instance exists, remove it
	AvailableAbilities.Remove(AbilityToRemove);
	return true;
}

void UGameplaySystemComponent::CreateAbilityInstance(TSubclassOf<UGameplayAbility> AbilityToSetup)
{
	check(AbilityToSetup);

	// Create a new instance of the ability and add it to the available abilities map
	UGameplayAbility* NewAbility = NewObject<UGameplayAbility>(this, AbilityToSetup);

	check(NewAbility); // Sanity check

	NewAbility->Init(GetOwner(), this);
	AvailableAbilities.Emplace(AbilityToSetup, NewAbility);
}


FOnAttributeChangedSignature& FDelegateCollection::GetDelegate(EAttributeType Attribute)
{
	CheckDelegates();

	bIsDirty = true;
	
	return DelegateMap.FindOrAdd(Attribute);
}

void FDelegateCollection::GetMultipleDelegates(TArray<EAttributeType> Attributes, TArray<FOnAttributeChangedSignature*> OutDelegates)
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
