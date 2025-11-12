// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "Attribute.h"
#include "AttributeEffect.h"
#include "GameplayAbilitySlot.h"
#include "StructValidityKey.h"

#include "GameplaySystemBlueprintLibrary.generated.h"

struct FActiveGameplayAbility;
class UGameplaySystemComponent;

UCLASS()
class GAMEPLAYSYSTEM_API UGameplaySystemBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// --- Abilities

	// See GameplayAbility.h
	UFUNCTION(BlueprintPure ,BlueprintCallable, Category = "GameplaySystem|Ability") 
	static float GetGameplayAbilityCooldown(const FActiveGameplayAbility& ActiveGameplayAbility);

	// See GameplayAbility.h
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "GameplaySystem|Ability")
	static float GetGameplayAbilityDuration(const FActiveGameplayAbility& ActiveGameplayAbility);

	// Gets the instance from the GameplayAbility. Can return nullptr if not assigned as part of the instancing process.
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "GameplaySystem|Ability")
	static UGameplayAbility* GetGameplayAbilityInstance(const FActiveGameplayAbility& ActiveGameplayAbility);

	// Returns true if valid, false if not.
	// Defined as non-valid if the GameplayAbility instance pointer is null
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "GameplaySystem|Ability")
	static bool CheckGameplayAbilityHandle(const FActiveGameplayAbility& ActiveGameplayAbility);

	// --- Ability Slots

	// See GameplayAbilitySlot.h
	UFUNCTION(BlueprintCallable, Category = "GameplaySystem|AbilitySlot")
	static bool ActivateAbility(const FGameplayAbilitySlot& Slot);

	// See GameplayAbilitySlot.h
	UFUNCTION(BlueprintCallable, Category = "GameplaySystem|AbilitySlot")
	static void SetAbility(FGameplayAbilitySlot& SlotContainer, TSubclassOf<UGameplayAbility> Ability);

	// See GameplayAbilitySlot.h
	UFUNCTION(BlueprintCallable, Category = "GameplaySystem|AbilitySlot")
	static bool GetSlot(const FGameplayAbilitySlotContainer& Slot, const FGameplayTag& SlotTag, FGameplayAbilitySlot& OutSlot);

	// --- Attributes

	// Converts the AttributeType to a display friendly string. Works in shipping builds!
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "GameplaySystem|Attributes")
	static FString ConvertAttributeToDisplayName(EAttributeType Attribute);

	// Converts the ApplicationType to a display friendly string. Works in shipping builds!
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "GameplaySystem|Attributes")
	static FString ConvertEffectApplicationTypeToDisplayName(EEffectApplicationType ApplicationType);
	
	// Converts the TargetValue type to a display friendly string. Works in shipping builds!
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "GameplaySystem|Attributes")
	static FString ConvertTargetValueToDisplayName(ETargetValue TargetType);

	// Returns a value between 0 and 1.
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "GameplaySystem|Attributes")
	static float GetHealthAsPercentage(UGameplaySystemComponent* GameplaySystem);

	// Returns a value between 0 and 1.
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "GameplaySystem|Attributes")
	static float GetChargeAsPercentage(UGameplaySystemComponent* GameplaySystem);

	// Returns a value between 0 and 1.
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "GameplaySystem|Attributes")
	static float GetEnergyAsPercentage(UGameplaySystemComponent* GameplaySystem);

	// --- StructValidityKey

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "StructValidityKey")
	static FStructValidityKey MakeValidValidityKey();

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "StructValidityKey")
	static bool CheckValidityKey(const FStructValidityKey& ValidityKey);








};
