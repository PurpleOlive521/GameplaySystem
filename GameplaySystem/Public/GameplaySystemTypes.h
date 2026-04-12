// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AttributeTypes.h"
#include "Attribute.h"
#include "GameplayTags/GameplayTagSystem.h"
#include "GameplaySystemTypes.generated.h"

class UGameplaySystemComponent;
class UCharacterMovementComponent;
class USkeletalMeshComponent;
class UAnimInstance;
class UGameplayAbility;
class AEnemyAIControllerBase;
class AActor;

// A collection of frequently used properties for abilities, to avoid needing to search & cache the properties per ability.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplaySystemActorInfo
{
	GENERATED_BODY()

	FGameplaySystemActorInfo() = default;

	virtual void Init(AActor* InOwner, UGameplaySystemComponent* InComponent);

	virtual void ClearActorInfo();
	
	// Gets the AnimInstance from the SkeletalMeshComponent. Returns nullptr if none exists or no SkeletalMeshComponent is present.
	UAnimInstance* GetAnimInstance() const;

	TWeakObjectPtr<AActor> OwningActor = nullptr;

	TWeakObjectPtr<UGameplaySystemComponent> OwningComponent = nullptr;

	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent = nullptr;

	TWeakObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent = nullptr;
};

// Data specific to the activation of this GameplayAbility.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplayAbilityActivationData
{
	GENERATED_BODY()

	FGameplayAbilityActivationData() = default;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Target = nullptr;
};

// Contains information about any AnimMontage that this GameplaySystem has activated, and which ability it was activated from.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplaySystemAnimMontageInfo
{
	GENERATED_BODY()

	FGameplaySystemAnimMontageInfo() = default;

	void AssignMontage(UAnimMontage* NewMontage, UGameplayAbility* Ability);

	// Intended for use when blendspaces are in charge of the AnimMontage thats played and we want any triggered AnimNotify's to always route to this ability while active.
	void AssignOverrideAbility(UGameplayAbility* Ability);

	// Returns true if the given montage is the one currently being played by the GameplaySystem. 
	bool IsActiveMontage(UAnimMontage* InMontage) const;

	// Can return nullptr if not currently animating.
	UGameplayAbility* GetAnimatingAbility() const;

	// The AnimMontage we are currently playing.
	TObjectPtr<UAnimMontage> CurrentMontage;

	// The ability that played the CurrentMontage.
	TWeakObjectPtr<UGameplayAbility> AnimatingAbility;

	// Routes AnimNotifies from blendspaces to the correct ability. We have no way of knowing what AnimMontage the blendspace is playing,
	// let alone which ability is responsible for triggering the blendspace. As such, an ability can "claim" all notifies while this is true.
	bool bAbilityIsOverriding = false;
};

// Caches select properties for comparing system state at different points in time. Mainly intended for GameplayEffects.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplaySystemSnapshot
{
	GENERATED_BODY();

	FGameplaySystemSnapshot() = default;

	FGameplaySystemSnapshot(UGameplaySystemComponent* GameplaySystem);

	UPROPERTY(BlueprintReadWrite)
	TMap<EAttributeType, FAttribute> Attributes;

	UPROPERTY(BlueprintReadWrite)
	FGameplayTagSystem GameplayTags;
};

// All credit to Unreal's GameplayAbilitySystem for the original implementation of FGameplayTagBlueprintPropertyMapping.

// Mapping for updating a Blueprint Property based on the state of a GameplayTag in a Actors GameplaySystemComponent.
USTRUCT()
struct GAMEPLAYSYSTEM_API FGameplayTagBlueprintPropertyMapping
{
	GENERATED_BODY()

public:

	FGameplayTagBlueprintPropertyMapping() = default;

	// GameplayTag being tracked.
	UPROPERTY(EditAnywhere, Category = "GameplayTagBlueprintProperty")
	FGameplayTag TagToMap;

	// Property to update with the GameplayTag count.
	UPROPERTY(VisibleAnywhere, Category = "GameplayTagBlueprintProperty")
	TFieldPath<FProperty> PropertyToEdit;

	// Name of property being edited.
	UPROPERTY(VisibleAnywhere, Category = "GameplayTagBlueprintProperty")
	FName PropertyName;

	// Guid of property being edited.
	UPROPERTY(VisibleAnywhere, Category = "GameplayTagBlueprintProperty")
	FGuid PropertyGuid;
};


// All credit to Unreal's GameplayAbilitySystem for the original implementation.

// Container for Properties mapped to GameplayTags.
USTRUCT()
struct GAMEPLAYSYSTEM_API FGameplayTagBlueprintPropertyMap
{
	GENERATED_BODY()

public:

	FGameplayTagBlueprintPropertyMap() = default;
	FGameplayTagBlueprintPropertyMap(const FGameplayTagBlueprintPropertyMap& Other);
	~FGameplayTagBlueprintPropertyMap();

	// Call this to initialize and bind the properties to a GameplaySystemComponent.
	void Initialize(UObject* Owner, class UGameplaySystemComponent* GameplaySystem);

	// Call to manually apply the current tag state, can handle cases where callbacks were skipped
	void ApplyCurrentTags();

#if WITH_EDITOR
	// This can optionally be called in the owner's IsDataValid() for data validation.
	EDataValidationResult IsDataValid(const UObject* ContainingAsset, class FDataValidationContext& Context) const;
#endif // #if WITH_EDITOR

protected:

	void Unregister();

	void GameplayTagEventCallback(FGameplayTag GameplayTag, int NewCount, int Delta);

	bool IsPropertyTypeValid(const FProperty* Property) const;

protected:

	TWeakObjectPtr<UObject> Owner;
	TWeakObjectPtr<UGameplaySystemComponent> GameplaySystem;

	UPROPERTY(EditAnywhere, Category = "GameplayTagBlueprintProperty")
	TArray<FGameplayTagBlueprintPropertyMapping> PropertyMappings;

	// Handle to delegate bound on the GameplaySystemComponent.
	FDelegateHandle DelegateHandle;
};

namespace GameplaySystemConstants
{
	constexpr int NO_LEVEL = -1;
}