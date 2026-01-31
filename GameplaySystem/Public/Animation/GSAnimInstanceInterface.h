// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GSAnimInstanceInterface.generated.h"

class UGameplaySystemComponent;

UINTERFACE(MinimalAPI)
class UGSAnimInstanceInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Native-only interface for AnimInstances. 
 * Used to implement automatic syncing between Blueprint properties and GameplayTags from a GameplaySystemComponent.
 */
class GAMEPLAYSYSTEM_API IGSAnimInstanceInterface
{
	GENERATED_BODY()

public:

	virtual void InitializeWithGameplaySystem(UGameplaySystemComponent* GameplaySystem);
};
