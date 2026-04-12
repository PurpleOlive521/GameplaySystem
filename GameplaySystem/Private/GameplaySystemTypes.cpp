// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplaySystemTypes.h"
#include "GameplaySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "DevelopmentTypes.h"
#include "GameFramework/Actor.h"
#include "GSAnimInstanceInterface.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif // WITH_EDITOR

void FGameplaySystemActorInfo::Init(AActor* InOwner, UGameplaySystemComponent* InComponent)
{
	check(InOwner);
	check(InComponent);

	OwningActor = InOwner;
	OwningComponent = InComponent;

	MovementComponent = OwningActor->FindComponentByClass<UCharacterMovementComponent>();
	SkeletalMeshComponent = OwningActor->FindComponentByClass<USkeletalMeshComponent>();

	if(UAnimInstance* AnimInstance = GetAnimInstance())
	{
		if (IGSAnimInstanceInterface* GSAnimInterface = Cast<IGSAnimInstanceInterface>(AnimInstance))
		{
			GSAnimInterface->InitializeWithGameplaySystem(InComponent);
		}
	}
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

FGameplaySystemSnapshot::FGameplaySystemSnapshot(UGameplaySystemComponent* GameplaySystem)
{
	GameplaySystem->GetAttributes(Attributes);

	GameplayTags = *GameplaySystem->GetGameplayTagSystem();
}

FGameplayTagBlueprintPropertyMap::FGameplayTagBlueprintPropertyMap(const FGameplayTagBlueprintPropertyMap& Other)
{
	ensureMsgf(Other.Owner.IsExplicitlyNull(), TEXT("FGameplayTagBlueprintPropertyMap cannot be used inside an array or other container that is copied after register!"));
	PropertyMappings = Other.PropertyMappings;
}

FGameplayTagBlueprintPropertyMap::~FGameplayTagBlueprintPropertyMap()
{
	Unregister();
}

#if WITH_EDITOR
EDataValidationResult FGameplayTagBlueprintPropertyMap::IsDataValid(const UObject* ContainingAsset, FDataValidationContext& Context) const
{
	UClass* OwnerClass = ((ContainingAsset != nullptr) ? ContainingAsset->GetClass() : nullptr);
	if (!OwnerClass)
	{
		GS_LOG(Error, TEXT("FGameplayTagBlueprintPropertyMap: IsDataValid() called with an invalid Owner."));
		return EDataValidationResult::Invalid;
	}

	for (const FGameplayTagBlueprintPropertyMapping& Mapping : PropertyMappings)
	{
		if (!Mapping.TagToMap.IsValid())
		{
			Context.AddError(FText::Format(INVTEXT("The GameplayTag [{0}] for property [{1}] is empty or invalid."),
				FText::AsCultureInvariant(Mapping.TagToMap.ToString()),
				FText::FromName(Mapping.PropertyName)));
		}

		if (FProperty* Property = OwnerClass->FindPropertyByName(Mapping.PropertyName))
		{
			if (!IsPropertyTypeValid(Property))
			{
				Context.AddError(FText::Format(INVTEXT("The property [{0}] for GameplayTag [{1}] is not a supported type.  Supported types are: integer, float, and boolean."),
					FText::FromName(Mapping.PropertyName),
					FText::AsCultureInvariant(Mapping.TagToMap.ToString())));
			}
		}
		else
		{
			Context.AddError(FText::Format(INVTEXT("The property [{0}] for GameplayTag [{1}] could not be found."),
				FText::FromName(Mapping.PropertyName),
				FText::AsCultureInvariant(Mapping.TagToMap.ToString())));
		}
	}

	return ((Context.GetNumErrors() > 0) ? EDataValidationResult::Invalid : EDataValidationResult::Valid);
}
#endif // #if WITH_EDITOR

void FGameplayTagBlueprintPropertyMap::Initialize(UObject* InOwner, UGameplaySystemComponent* InGameplaySystem)
{
	UClass* OwnerClass = (InOwner ? InOwner->GetClass() : nullptr);
	if (!OwnerClass)
	{
		GS_LOG(Error, TEXT("FGameplayTagBlueprintPropertyMap: Initialize() called with an invalid Owner."));
		return;
	}

	if (!InGameplaySystem)
	{
		GS_LOG(Error, TEXT("FGameplayTagBlueprintPropertyMap: Initialize() called with an invalid GameplaySystemComponent."));
		return;
	}

	if ((InOwner == Owner) && (InGameplaySystem == GameplaySystem))
	{
		// Already initialized.
		return;
	}

	// Unregister old owner
	if (Owner.IsValid())
	{
		Unregister();
	}

	Owner = InOwner;
	GameplaySystem = InGameplaySystem;

	// Process array starting at the end so we can remove invalid entries.
	for (int32 MappingIndex = (PropertyMappings.Num() - 1); MappingIndex >= 0; --MappingIndex)
	{
		FGameplayTagBlueprintPropertyMapping& Mapping = PropertyMappings[MappingIndex];

		if (Mapping.TagToMap.IsValid())
		{
			FProperty* Property = OwnerClass->FindPropertyByName(Mapping.PropertyName);
			if (Property && IsPropertyTypeValid(Property))
			{
				Mapping.PropertyToEdit = Property;
				continue;
			}
		}

		// Entry was invalid.  Remove it from the array.
		GS_LOG(Error, TEXT("FGameplayTagBlueprintPropertyMap: Removing invalid GameplayTagBlueprintPropertyMapping [Index: %d, Tag:%s, Property:%s] for [%s]."),
			MappingIndex, *Mapping.TagToMap.ToString(), *Mapping.PropertyName.ToString(), *GetNameSafe(InOwner));

		PropertyMappings.RemoveAtSwap(MappingIndex, 1, EAllowShrinking::No);
	}

	DelegateHandle = InGameplaySystem->GetGameplayTagSystem()->OnGameplayTagModifiedDelegate.AddRaw(this, &FGameplayTagBlueprintPropertyMap::GameplayTagEventCallback);

}

void FGameplayTagBlueprintPropertyMap::Unregister()
{
	if (UGameplaySystemComponent* DerefGameplaySystem = GameplaySystem.Get())
	{
		for (FGameplayTagBlueprintPropertyMapping& Mapping : PropertyMappings)
		{
			Mapping.PropertyToEdit = nullptr;
		}

		DerefGameplaySystem->GetGameplayTagSystem()->OnGameplayTagModifiedDelegate.Remove(DelegateHandle);
		DelegateHandle.Reset();
	}

	Owner = nullptr;
	GameplaySystem = nullptr;
}

void FGameplayTagBlueprintPropertyMap::GameplayTagEventCallback(FGameplayTag GameplayTag, int NewCount, int Delta)
{
	UObject* DerefOwner = Owner.Get();
	if (!DerefOwner)
	{
		GS_LOG(Warning, TEXT("FGameplayTagBlueprintPropertyMap::GameplayTagEventCallback has an invalid Owner."));
		return;
	}

	FGameplayTagBlueprintPropertyMapping* Mapping = PropertyMappings.FindByPredicate([GameplayTag](const FGameplayTagBlueprintPropertyMapping& Test)
		{
			return (GameplayTag == Test.TagToMap);
		});

	if (Mapping && Mapping->PropertyToEdit.Get())
	{
		if (const FBoolProperty* BoolProperty = CastField<const FBoolProperty>(Mapping->PropertyToEdit.Get()))
		{
			BoolProperty->SetPropertyValue_InContainer(DerefOwner, NewCount > 0);
		}
		else if (const FIntProperty* IntProperty = CastField<const FIntProperty>(Mapping->PropertyToEdit.Get()))
		{
			IntProperty->SetPropertyValue_InContainer(DerefOwner, NewCount);
		}
		else if (const FFloatProperty* FloatProperty = CastField<const FFloatProperty>(Mapping->PropertyToEdit.Get()))
		{
			FloatProperty->SetPropertyValue_InContainer(DerefOwner, (float)NewCount);
		}
	}
}

void FGameplayTagBlueprintPropertyMap::ApplyCurrentTags()
{
	UObject* DerefOwner = Owner.Get();
	if (!DerefOwner)
	{
		GS_LOG(Warning, TEXT("FGameplayTagBlueprintPropertyMap::ApplyCurrentTags called with an invalid Owner."));
		return;
	}

	UGameplaySystemComponent* DerefGameplaySystem = GameplaySystem.Get();
	if (!DerefGameplaySystem)
	{
		GS_LOG(Warning, TEXT("FGameplayTagBlueprintPropertyMap::ApplyCurrentTags called with an invalid GameplaySystemComponent."));
		return;
	}

	for (FGameplayTagBlueprintPropertyMapping& Mapping : PropertyMappings)
	{
		if (Mapping.PropertyToEdit.Get() && Mapping.TagToMap.IsValid())
		{
			int32 NewCount = DerefGameplaySystem->GetTagCount(Mapping.TagToMap);

			if (const FBoolProperty* BoolProperty = CastField<const FBoolProperty>(Mapping.PropertyToEdit.Get()))
			{
				BoolProperty->SetPropertyValue_InContainer(DerefOwner, NewCount > 0);
			}
			else if (const FIntProperty* IntProperty = CastField<const FIntProperty>(Mapping.PropertyToEdit.Get()))
			{
				IntProperty->SetPropertyValue_InContainer(DerefOwner, NewCount);
			}
			else if (const FFloatProperty* FloatProperty = CastField<const FFloatProperty>(Mapping.PropertyToEdit.Get()))
			{
				FloatProperty->SetPropertyValue_InContainer(DerefOwner, (float)NewCount);
			}
		}
	}
}

bool FGameplayTagBlueprintPropertyMap::IsPropertyTypeValid(const FProperty* Property) const
{
	check(Property);
	return (Property->IsA<FBoolProperty>() || Property->IsA<FIntProperty>() || Property->IsA<FFloatProperty>());
}
