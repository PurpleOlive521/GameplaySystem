// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

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

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "GameplaySystem|Ability")
	static bool IsEqual(const FGameplayAbilityHandle& A, const FGameplayAbilityHandle& B);

	// See GameplayAbility.h
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "GameplaySystem|Ability") 
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
	static bool ActivateAbility(UPARAM(ref) FGameplayAbilitySlotContainer& Container, const FGameplayTag& Tag);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Activate Ability With Activation Data"), Category = "GameplaySystem|AbilitySlot")
	static bool ActivateAbility_ActivationData(UPARAM(ref) FGameplayAbilitySlotContainer& Container, const FGameplayTag& Tag, const FGameplayAbilityActivationData& ActivationData);

	// See GameplayAbilitySlot.h
	UFUNCTION(BlueprintCallable, Category = "GameplaySystem|AbilitySlot")
	static bool SetAbility(UPARAM(ref) FGameplayAbilitySlotContainer& Container, const FGameplayTag& Tag, TSubclassOf<UGameplayAbility> Ability);

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

	// --- StructValidityKey

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "StructValidityKey")
	static FStructValidityKey MakeValidValidityKey();

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "StructValidityKey")
	static bool CheckValidityKey(const FStructValidityKey& ValidityKey);

	// Time Dilation

	// Returns the delta that we were allowed to apply Globally. Still clamped between WorldSettings min and max dilation.
	UFUNCTION(BlueprintCallable, Category = "GameplaySystem|Time", meta = (WorldContext = "WorldContextObject"))
	static float ModifyGlobalTimeDilation(const UObject* WorldContextObject, float Delta);

	// Returns the delta that we were allowed to apply to this Actor. Still clamped between WorldSettings min and max dilation.
	UFUNCTION(BlueprintCallable, Category = "GameplaySystem|Time", meta = (WorldContext = "WorldContextObject"))
	static float ModifyCustomTimeDilation(const UObject* WorldContextObject, AActor* Actor, float Delta);





};
