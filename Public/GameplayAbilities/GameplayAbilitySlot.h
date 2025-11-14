// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplaySystemComponent.h"
#include "GameplayAbilitySlot.generated.h"

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayAbilitySlot
{
	GENERATED_BODY()

	FGameplayAbilitySlot();

	void Init(UGameplaySystemComponent* InGameplaySystem);

	bool ActivateAbility() const;

	// Will attempt to remove the old Abilities Instance first, before adding the new one.
	void SetAbility(TSubclassOf<UGameplayAbility> Ability);

	UPROPERTY(BlueprintReadOnly, Category = "GameplayAbilitySlot")
	TSubclassOf<UGameplayAbility> StoredAbility = nullptr;

	TWeakObjectPtr<UGameplaySystemComponent> GameplaySystem = nullptr;
};


USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayAbilitySlotContainer
{
	GENERATED_BODY()

	FGameplayAbilitySlotContainer();

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameplayAbilitySlotContainer");
	TMap<FGameplayTag, FGameplayAbilitySlot> GameplayAbilitySlotTable;

	// Returns false if no GameplayAbilitySlot is associated with the tag.
	bool GetSlot(const FGameplayTag& SlotTag, FGameplayAbilitySlot& OutSlot) const;

	// Propagates the GameplaySystem to all slots in the container
	void Init(UGameplaySystemComponent* InGameplaySystem);

};
