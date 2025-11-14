// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "GameplaySystemTypes.generated.h"

class UGameplaySystemComponent;
class UCharacterMovementComponent;
class USkeletalMeshComponent;
class UAnimInstance;
class UGameplayAbility;


// A collection of frequently used properties for abilities, to avoid needing to search & cache the properties per ability.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplaySystemActorInfo
{
	GENERATED_BODY()

	FGameplaySystemActorInfo() = default;

	void Init(AActor* InOwner, UGameplaySystemComponent* InComponent);

	void ClearActorInfo();
	
	// Shorthand for safely getting the AnimInstance from the SkeletalMeshComponent
	UAnimInstance* GetAnimInstance() const;


	TWeakObjectPtr<AActor> OwningActor;

	TWeakObjectPtr<UGameplaySystemComponent> OwningComponent;

	TWeakObjectPtr<UCharacterMovementComponent> MovementComponent;

	TWeakObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;
};

// Contains information about any AnimMontage that this GameplaySystem has activated, and which ability it was activated from.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FGameplaySystemAnimMontageInfo
{
	GENERATED_BODY()

	FGameplaySystemAnimMontageInfo() = default;

	void AssignMontage(UAnimMontage* NewMontage, UGameplayAbility* Ability);

	// Intended for use when blendspaces are in charge of the actual AnimMontage thats played and we want those to always route to this ability instead.
	void AssignOverrideAbility(UGameplayAbility* Ability);

	// Returns true if the given montage is the one currently being played by the GameplaySystem. 
	bool IsActiveMontage(UAnimMontage* InMontage) const;

	// Can return nullptr if not currently animating.
	UGameplayAbility* GetAnimatingAbility() const;

	// The AnimMontage we are currently playing (if any)
	TObjectPtr<UAnimMontage> CurrentMontage;

	// The ability that played the CurrentMontage
	TWeakObjectPtr<UGameplayAbility> AnimatingAbility;

	// Used to route notifies from blendspaces to the correct ability. We have no way of knowing what AnimMontage the blendspace is playing
	// let alone which ability is responsible for triggering the blendspace. As such, a ability can "claim" all notifies while this is true.
	bool bAbilityIsOverriding = false;
};
