// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplaySystemDeveloperSettings.h"

const UGameplaySystemProperties* UGameplaySystemDeveloperSettings::GetDefaultProperties() const
{
    if (const UGameplaySystemProperties* DerefProperties = DefaultProperties.Get())
    {
        return DerefProperties;
    }

    return GetDefault<UGameplaySystemProperties>();
}

float UGameplaySystemDeveloperSettings::GetGlobalAnimPlayRate() const
{
    return GlobalAnimPlayRate;
}
