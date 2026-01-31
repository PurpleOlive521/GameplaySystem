// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


#include "Animation/GSAnimInstance.h"

void UGSAnimInstance::InitializeWithGameplaySystem(UGameplaySystemComponent* GameplaySystem)
{
	GameplayTagPropertyMap.Initialize(this, GameplaySystem);
}

#if WITH_EDITOR
EDataValidationResult UGSAnimInstance::IsDataValid(class FDataValidationContext& Context) const
{
	return GameplayTagPropertyMap.IsDataValid(this, Context);
}
#endif // WITH_EDITOR

