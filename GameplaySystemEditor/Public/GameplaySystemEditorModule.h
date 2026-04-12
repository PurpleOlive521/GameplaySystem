// Copyright (c) 2026, Heavy Duty Tape Studios. All rights reserved.

#pragma once
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FGameplaySystemEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};