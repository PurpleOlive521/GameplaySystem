// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplaySaveGame.h"

float UGameplaySaveGame::GetPlayTime() const
{
	return PlayTime;
}


void UGameplaySaveGame::SetPlayTime(float InPlayTime)
{
	PlayTime = InPlayTime;
}

void UGameplaySaveGame::SaveGameplaySystem(const FGuidTag& Id, const FGameplaySystemSaveObject& SaveObject)
{
	GameplaySystemToIdMap.Add(Id, SaveObject);
}

FGameplaySystemSaveObject UGameplaySaveGame::LoadGameplaySystem(const FGuidTag& Id)
{
	if (auto* ValuePtr = GameplaySystemToIdMap.Find(Id))
	{
		return *ValuePtr;
	}

	return FGameplaySystemSaveObject{};
}
