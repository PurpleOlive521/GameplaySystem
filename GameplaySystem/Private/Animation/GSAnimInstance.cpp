// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "Animation/GSAnimInstance.h"
#include "GameplaySystemComponent.h"

void UGSAnimInstance::InitializeWithGameplaySystem(UGameplaySystemComponent* InGameplaySystem)
{
	GameplaySystem = MakeWeakObjectPtr(InGameplaySystem);

	GameplaySystem->OnAttributeChangedDelegateCollection.GetDelegate(EAttributeType::EAT_MovementSpeed).AddUObject(this, &UGSAnimInstance::OnMovementSpeedChanged);
	OnMovementSpeedChanged(EAttributeType::EAT_MovementSpeed);

	GameplayTagPropertyMap.Initialize(this, InGameplaySystem);
}

#if WITH_EDITOR

EDataValidationResult UGSAnimInstance::IsDataValid(class FDataValidationContext& Context) const
{
	return GameplayTagPropertyMap.IsDataValid(this, Context);
}

#endif // WITH_EDITOR

void UGSAnimInstance::OnMovementSpeedChanged(EAttributeType Attribute)
{
	if (UGameplaySystemComponent* GS = GameplaySystem.Get())
	{
		MovementSpeed =		GS->GetAttributeValue(EAttributeType::EAT_MovementSpeed, EAttributeValue::EAV_CurrentValue);
		BaseMovementSpeed = GS->GetAttributeValue(EAttributeType::EAT_MovementSpeed, EAttributeValue::EAV_BaseValue);
	}
}
