// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


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

bool FGameplayAbilitySlot::ActivateAbility(const FGameplayAbilityActivationData& ActivationData)
{
	if (!Ability)
	{
		GA_LOG(Warning, TEXT("GameplayAbilitySlot: Tried to activate Slot without a assigned Ability!"));
		return false;
	}

	UGameplaySystemComponent* GameplaySystem = GetGameplaySystemComponent();
	const bool bActivated = GameplaySystem->UseAbility_ActivationData(Ability, ActivationData, AbilityHandle);

	if (bActivated)
	{
		OwningContainer->OnSlotAbilityActivatedDelegate.Broadcast(Ability, SlotTag);

		BindToAbility();
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

	UnbindFromAbility();

	UGameplaySystemComponent* GameplaySystem = OwningContainer->GetGameplaySystem();
	GameplaySystem->RemoveAbilityInstance(Ability);

	Ability = InAbility;

	// We assume that this ability is new to the system
	GameplaySystem->AddAbilityInstance(Ability);

	OwningContainer->OnSlotAbilitySwitchedDelegate.Broadcast(Ability);
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

bool FGameplayAbilitySlot::IsAbilityActive() const
{
	UGameplaySystemComponent* GameplaySystem = OwningContainer->GetGameplaySystem();
	FActiveGameplayAbility* ActiveAbility = GameplaySystem->GetActiveAbilityFromHandle_Ptr(AbilityHandle);

	if (ActiveAbility)
	{
		return ActiveAbility->IsAbilityActive();
	}

	return false;
}

UGameplaySystemComponent* FGameplayAbilitySlot::GetGameplaySystemComponent() const
{
	check(OwningContainer);
	return OwningContainer->GetGameplaySystem();
}

void FGameplayAbilitySlot::BindToAbility()
{
	UGameplaySystemComponent* GameplaySystem = OwningContainer->GetGameplaySystem();
	UGameplayAbility* AbilityInstance = GameplaySystem->GetAbilityInstanceFromHandle(AbilityHandle);

	AbilityFinishedHandle = AbilityInstance->OnAbilityFinishedDelegate.AddRaw(this, &FGameplayAbilitySlot::OnAbilityFinished);
}

void FGameplayAbilitySlot::UnbindFromAbility() const
{
	if (OwningContainer)
	{
		if (UGameplaySystemComponent* GameplaySystem = OwningContainer->GetGameplaySystem())
		{
			if (UGameplayAbility* AbilityInstance = GameplaySystem->GetAbilityInstanceFromHandle(AbilityHandle))
			{
				AbilityInstance->OnAbilityFinishedDelegate.Remove(AbilityFinishedHandle);
			}
		}
	}
}

void FGameplayAbilitySlot::OnAbilityFinished(UGameplayAbility* InAbility) const
{
	OwningContainer->OnSlotAbilityFinishedDelegate.Broadcast(Ability, SlotTag);	
}

FGameplayAbilitySlot* FGameplayAbilitySlotContainer::GetSlot(const FGameplayTag& SlotTag)
{
	return GameplayAbilitySlotTable.Find(SlotTag);
}

FGameplayAbilitySlot& FGameplayAbilitySlotContainer::GetSlotRef(const FGameplayTag& SlotTag)
{
	return GameplayAbilitySlotTable.FindChecked(SlotTag);
}

inline UGameplaySystemComponent* FGameplayAbilitySlotContainer::GetGameplaySystem() const
{
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


