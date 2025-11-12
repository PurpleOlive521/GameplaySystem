// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "GameplayAbilitySlot.h"
#include "Kismet/GameplayStatics.h"

FGameplayAbilitySlot::FGameplayAbilitySlot()
{
}

void FGameplayAbilitySlot::Init(UGameplaySystemComponent* InGameplaySystem)
{
	check(InGameplaySystem);

	GameplaySystem = InGameplaySystem;

	if (StoredAbility)
	{
		// We assume that this ability is new to the system
		GameplaySystem->AddAbility(StoredAbility);
	}
}

bool FGameplayAbilitySlot::ActivateAbility() const
{
	// We allow empty slots, in case actions for this slot are prohibited or not yet unlocked
	if (StoredAbility)
	{
		check(GameplaySystem.IsValid())

		FActiveGameplayAbility Handle;
		return GameplaySystem->UseAbility(StoredAbility, Handle);
	}

	return false;
}

void FGameplayAbilitySlot::SetAbility(TSubclassOf<UGameplayAbility> Ability)
{
	check(Ability);
	check(GameplaySystem.IsValid());

	// To avoid keeping stale references to abilities that are no longer in use, thereby keeping their respective assets in memory,
	// we attempt to remove the old ability before replacing it. 
	GameplaySystem->RemoveAbility(StoredAbility);

	StoredAbility = Ability;

	// We assume that this ability is new to the system
	GameplaySystem->AddAbility(StoredAbility);
}

FGameplayAbilitySlotContainer::FGameplayAbilitySlotContainer()
{
}

bool FGameplayAbilitySlotContainer::GetSlot(const FGameplayTag& SlotTag, FGameplayAbilitySlot& OutSlot) const
{
	if (GameplayAbilitySlotTable.Contains(SlotTag))
	{
		OutSlot = GameplayAbilitySlotTable.FindRef(SlotTag);
		return true;
	}

	return false;
}

void FGameplayAbilitySlotContainer::Init(UGameplaySystemComponent* InGameplaySystem)
{
	for(auto& SlotPair : GameplayAbilitySlotTable)
	{
		FGameplayAbilitySlot& Slot = SlotPair.Value;
		Slot.Init(InGameplaySystem);
	}
}


