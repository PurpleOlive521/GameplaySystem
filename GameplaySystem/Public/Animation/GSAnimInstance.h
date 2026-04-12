// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GSAnimInstanceInterface.h"
#include "GameplaySystemTypes.h"
#include "GSAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UGSAnimInstance : public UAnimInstance, public IGSAnimInstanceInterface
{
	GENERATED_BODY()
	
public:
	virtual void InitializeWithGameplaySystem(UGameplaySystemComponent* GameplaySystem);

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif // #if WITH_EDITOR

protected:

	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;
};
