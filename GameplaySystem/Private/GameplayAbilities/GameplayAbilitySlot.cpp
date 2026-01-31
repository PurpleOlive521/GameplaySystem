// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


#include "GameplayAbilitySlot.h"
#include "Kismet/GameplayStatics.h"


void FGameplayAbilitySlot::Init(FGameplayAbilitySlotContainer* Owner, const FGameplayTag& InSlotTag)
{
	check(Owner);

	OwningContainer = Owner;
	SlotTag = InSlotTag;

	if (Ability)
	{
		UGameplaySystemComponent* GameplaySystem = GetGameplaySystemComponent();
		GameplaySystem->AddAbilityInstance(Ability);
	}
}

bool FGameplayAbilitySlot::ActivateAbility()
{
	if (!Ability)
	{
		GA_LOG(Warning, TEXT("GameplayAbilitySlot: Tried to activate Slot without a assigned Ability!"));
		return false;
	}

	UGameplaySystemComponent* GameplaySystem = GetGameplaySystemComponent();
	const bool bActivated = GameplaySystem->UseAbility(Ability, AbilityHandle);

	if (bActivated)
	{
		OwningContainer->OnAbilityActivatedDelegate.Broadcast(Ability, SlotTag);
	}

	return bActivated;
}

void FGameplayAbilitySlot::SetAbility(TSubclassOf<UGameplayAbility> InAbility)
{
	check(InAbility);

	if (Ability == InAbility)
	{
		return;
	}

	UGameplaySystemComponent* GameplaySystem = OwningContainer->GetGameplaySystem();
	GameplaySystem->RemoveAbilityInstance(Ability);

	Ability = InAbility;

	// We assume that this ability is new to the system
	GameplaySystem->AddAbilityInstance(Ability);

	OwningContainer->OnAbilitySwitchedDelegate.Broadcast(Ability);
}

float FGameplayAbilitySlot::GetCurrentCooldown() const
{
	UGameplaySystemComponent* GameplaySystem = OwningContainer->GetGameplaySystem();
	FActiveGameplayAbility* ActiveAbility = GameplaySystem->GetActiveAbilityFromHandle_Ptr(AbilityHandle);

	if (!ActiveAbility)
	{
		return 0.0f;
	}

	return ActiveAbility->GetRemainingCooldown();
}

float FGameplayAbilitySlot::GetCurrentCooldownAsPercentage() const
{
	UGameplaySystemComponent* GameplaySystem = OwningContainer->GetGameplaySystem();
	FActiveGameplayAbility* ActiveAbility = GameplaySystem->GetActiveAbilityFromHandle_Ptr(AbilityHandle);

	if (!ActiveAbility)
	{
		return 0.0f;
	}

	return ActiveAbility->GetRemainingCooldownAsPercentage();
}

float FGameplayAbilitySlot::GetCurrentDuration() const
{
	UGameplaySystemComponent* GameplaySystem = OwningContainer->GetGameplaySystem();
	FActiveGameplayAbility* ActiveAbility = GameplaySystem->GetActiveAbilityFromHandle_Ptr(AbilityHandle);

	if (!ActiveAbility)
	{
		return 0.0f;
	}

	return ActiveAbility->GetRemainingDuration();
}

float FGameplayAbilitySlot::GetCurrentDurationAsPercentage() const
{
	UGameplaySystemComponent* GameplaySystem = OwningContainer->GetGameplaySystem();
	FActiveGameplayAbility* ActiveAbility = GameplaySystem->GetActiveAbilityFromHandle_Ptr(AbilityHandle);

	if (!ActiveAbility)
	{
		return 0.0f;
	}

	return ActiveAbility->GetRemainingDurationAsPercentage();
}

UGameplaySystemComponent* FGameplayAbilitySlot::GetGameplaySystemComponent() const
{
	check(OwningContainer);
	return OwningContainer->GetGameplaySystem();
}

FGameplayAbilitySlot* FGameplayAbilitySlotContainer::GetSlot(const FGameplayTag& SlotTag)
{
	return GameplayAbilitySlotTable.Find(SlotTag);
}

bool FGameplayAbilitySlotContainer::GetSlotRef(const FGameplayTag& SlotTag, FGameplayAbilitySlot& OutSlot) const
{
	if (GameplayAbilitySlotTable.Contains(SlotTag))
	{
		OutSlot = GameplayAbilitySlotTable.FindRef(SlotTag);
		return true;
	}

	return false;
}

inline UGameplaySystemComponent* FGameplayAbilitySlotContainer::GetGameplaySystem() const
{
	check(GameplaySystem.IsValid())
	return GameplaySystem.Get();
}

void FGameplayAbilitySlotContainer::Init(UGameplaySystemComponent* InGameplaySystem)
{
	GameplaySystem = InGameplaySystem;

	for(auto& [Tag, Slot] : GameplayAbilitySlotTable)
	{
		Slot.Init(this, Tag);
	}
}


