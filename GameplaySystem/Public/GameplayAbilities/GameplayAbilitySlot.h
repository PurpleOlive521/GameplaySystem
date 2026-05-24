// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplaySystemComponent.h"
#include "GameplayAbilitySlot.generated.h"

struct FGameplayAbilitySlotContainer;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSlotAbilityActivatedSignature, TSubclassOf<UGameplayAbility> /* ActivatedAbility */, const FGameplayTag& /* SlotTag */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSlotAbilityFinishedSignature, TSubclassOf<UGameplayAbility> /* ActivatedAbility */, const FGameplayTag& /* SlotTag */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotAbilitySwitchedSignature, TSubclassOf<UGameplayAbility> /* NewAbility */);

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayAbilitySlot
{
	GENERATED_BODY()

	FGameplayAbilitySlot() = default;

	void Init(FGameplayAbilitySlotContainer* Owner, const FGameplayTag& SlotTag);

	bool ActivateAbility(const FGameplayAbilityActivationData& ActivationData);

	// Will attempt to remove the old Abilities Instance first, before adding the new one.
	void SetAbility(TSubclassOf<UGameplayAbility> Ability);

	// Returns 0 if no cooldown is present.
	float GetCurrentCooldown() const;

	// Returns a 0 to 1 value with 0 as no cooldown and 1 as full cooldown remaining.
	float GetCurrentCooldownAsPercentage() const;

	bool IsAbilityActive() const;

	UGameplaySystemComponent* GetGameplaySystemComponent() const;

	void BindToAbility();

	void UnbindFromAbility() const;

	void OnAbilityFinished(UGameplayAbility* Ability) const;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	TSubclassOf<UGameplayAbility> Ability = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FGameplayAbilityHandle AbilityHandle;

	FGameplayAbilitySlotContainer* OwningContainer = nullptr;

	FGameplayTag SlotTag;

	FDelegateHandle AbilityFinishedHandle;
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
	FGameplayAbilitySlot& GetSlotRef(const FGameplayTag& SlotTag);

	inline UGameplaySystemComponent* GetGameplaySystem() const;

	UPROPERTY(BlueprintReadOnly, EditAnywhere);
	TMap<FGameplayTag, FGameplayAbilitySlot> GameplayAbilitySlotTable;

	TWeakObjectPtr<UGameplaySystemComponent> GameplaySystem = nullptr;

	// --- Delegates

	// When a slot's ability is activated. Only triggered if activated through the slot!
	FOnSlotAbilityActivatedSignature OnSlotAbilityActivatedDelegate;

	// When a slot has it's stored ability changed to a different one.
	FOnSlotAbilitySwitchedSignature OnSlotAbilitySwitchedDelegate;

	FOnSlotAbilityFinishedSignature OnSlotAbilityFinishedDelegate;
};
