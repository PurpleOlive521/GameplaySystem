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

	UFUNCTION()
	void OnMovementSpeedChanged(EAttributeType Attribute);

protected:

	UPROPERTY(EditDefaultsOnly, Category = "GameplayTags")
	FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;

	UPROPERTY(BlueprintReadWrite, Category = "References")
	TWeakObjectPtr<UGameplaySystemComponent> GameplaySystem = nullptr;

	// The current movement speed. Affected by modifiers such as running.
	UPROPERTY(BlueprintReadOnly, Category = "Values")
	float MovementSpeed = 0.0f;

	// Movement speed unaffected by modifiers such as buffs, debuffs and running.
	// Can be considered the walking speed.
	UPROPERTY(BlueprintReadOnly, Category = "Values")
	float BaseMovementSpeed = 0.0f;

};
