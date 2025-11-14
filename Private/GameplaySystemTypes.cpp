// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "GameplaySystemTypes.h"
#include "GameplaySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

void FGameplaySystemActorInfo::Init(AActor* InOwner, UGameplaySystemComponent* InComponent)
{
	check(InOwner);
	check(InComponent);

	OwningActor = InOwner;
	OwningComponent = InComponent;

	MovementComponent = OwningActor->FindComponentByClass<UCharacterMovementComponent>();
	SkeletalMeshComponent = OwningActor->FindComponentByClass<USkeletalMeshComponent>();
}

void FGameplaySystemActorInfo::ClearActorInfo()
{
	OwningActor = nullptr;
	OwningComponent = nullptr;

	MovementComponent = nullptr;
	SkeletalMeshComponent = nullptr;
}

UAnimInstance* FGameplaySystemActorInfo::GetAnimInstance() const
{
	if (USkeletalMeshComponent* SkeletalMeshComp = SkeletalMeshComponent.Get())
	{
		return SkeletalMeshComp->GetAnimInstance();
	}

	return nullptr;
}

void FGameplaySystemAnimMontageInfo::AssignMontage(UAnimMontage* NewMontage, UGameplayAbility* Ability)
{
	CurrentMontage = NewMontage;
	AnimatingAbility = Ability;
}

void FGameplaySystemAnimMontageInfo::AssignOverrideAbility(UGameplayAbility* Ability)
{
	AnimatingAbility = Ability;
	bAbilityIsOverriding = true;
}

bool FGameplaySystemAnimMontageInfo::IsActiveMontage(UAnimMontage* InMontage) const
{
	// We want any querying ability to be routed to the overriding ability, even if it's not the same montage.
	if (bAbilityIsOverriding)
	{
		return true;
	}

	return InMontage == CurrentMontage;
}

UGameplayAbility* FGameplaySystemAnimMontageInfo::GetAnimatingAbility() const
{
	// While overriding there will not be a current montage, but we still want to return the ability.
	if (bAbilityIsOverriding || CurrentMontage)
	{
		return AnimatingAbility.Get();
	}

	return nullptr;
}

