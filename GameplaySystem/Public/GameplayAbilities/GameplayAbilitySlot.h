// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplaySystemComponent.h"
#include "GameplayAbilitySlot.generated.h"

struct FGameplayAbilitySlotContainer;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAbilityActivatedSignature, TSubclassOf<UGameplayAbility> /* ActivatedAbility */, const FGameplayTag& /* SlotTag */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilitySwitchedSignature, TSubclassOf<UGameplayAbility> /* NewAbility */);

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayAbilitySlot
{
	GENERATED_BODY()

	FGameplayAbilitySlot() = default;

	void Init(FGameplayAbilitySlotContainer* Owner, const FGameplayTag& SlotTag);

	bool ActivateAbility();

	// Will attempt to remove the old Abilities Instance first, before adding the new one.
	void SetAbility(TSubclassOf<UGameplayAbility> Ability);

	// Returns 0 if no cooldown is present.
	float GetCurrentCooldown() const;

	// Returns a 0 to 1 value with 0 as no cooldown and 1 as full cooldown remaining.
	float GetCurrentCooldownAsPercentage() const;

	// Returns 0 if no duration is present.
	float GetCurrentDuration() const;

	// Returns a 0 to 1 value with 0 as no duration and 1 as full duration remaining.
	float GetCurrentDurationAsPercentage() const;

	UGameplaySystemComponent* GetGameplaySystemComponent() const;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<UGameplayAbility> Ability = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FGameplayAbilityHandle AbilityHandle;

	FGameplayAbilitySlotContainer* OwningContainer = nullptr;

	FGameplayTag SlotTag;
};

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayAbilitySlotContainer
{
	GENERATED_BODY()

	FGameplayAbilitySlotContainer() = default;

	void Init(UGameplaySystemComponent* InGameplaySystem);

	// Returns nullptr if no GameplayAbilitySlot is associated with the tag.
	FGameplayAbilitySlot* GetSlot(const FGameplayTag& SlotTag);

	// Returns true if the slot was found and OutSlot is valid.
	bool GetSlotRef(const FGameplayTag& SlotTag, FGameplayAbilitySlot& OutSlot) const;

	inline UGameplaySystemComponent* GetGameplaySystem() const;

	UPROPERTY(BlueprintReadOnly, EditAnywhere);
	TMap<FGameplayTag, FGameplayAbilitySlot> GameplayAbilitySlotTable;

	TWeakObjectPtr<UGameplaySystemComponent> GameplaySystem = nullptr;

	// --- Delegates

	// When a slot's ability is activated. Only triggered if activated through the slot!
	FOnAbilityActivatedSignature OnAbilityActivatedDelegate;

	// When a slot has it's stored ability changed to a different one.
	FOnAbilitySwitchedSignature OnAbilitySwitchedDelegate;
};
