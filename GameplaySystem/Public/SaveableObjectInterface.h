// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"

#include "SaveableObjectInterface.generated.h"

class UGameplaySaveGame;

UINTERFACE()
class GAMEPLAYSYSTEM_API USaveableObjectInterface : public UInterface
{
	GENERATED_BODY()
};

class GAMEPLAYSYSTEM_API ISaveableObjectInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveableObject")
	void SaveToObject(UGameplaySaveGame* SaveGameObject);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "SaveableObject")
	void LoadFromObject(UGameplaySaveGame* SaveGameObject);

};
