// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


#include "GameplaySystemEditorModule.h"

#include "GameplayTagBlueprintPropertyMappingDetails.h"
#include "EditorAttributeCustomization.h"

IMPLEMENT_GAME_MODULE(FGameplaySystemEditorModule, GameplaySystemEditor);

void FGameplaySystemEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("GameplayTagBlueprintPropertyMapping", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGameplayTagBlueprintPropertyMappingDetails::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("EditorAttribute", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FEditorAttributeCustomization::MakeInstance));
}

void FGameplaySystemEditorModule::ShutdownModule()
{
	// Unregister customizations
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout("GameplayTagBlueprintPropertyMapping");
		PropertyModule.UnregisterCustomPropertyTypeLayout("EditorAttribute");
	}
}