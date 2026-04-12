// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "SaveGameSettings.h"

bool USaveGameSettings::CanSerializeLevel(LevelName Level) const
{
	if (Level.IsNone())
	{
		return false;
	}

	for (const auto& LevelPath : NeverSerialize)
	{
		LevelName Name = LevelUtilities::GetLevelNameFromSoftObjectPtr(LevelPath);

		if (Name == Level)
		{
			return false;
		}
	}

	return true;
}
