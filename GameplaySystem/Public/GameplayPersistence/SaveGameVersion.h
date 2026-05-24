// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"

#include "CoreTypes.h"
#include "Misc/Guid.h"

class GAMEPLAYSYSTEM_API FSaveGameVersion
{
public:
	enum Type
	{

		PreReleaseTesting,
		StackingGameplayEffects, // Advent of Stacking GameplayEffects and overall improvements to GameplaySystem framework.

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};

	const static FGuid GUID;
};
