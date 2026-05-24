// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplaySystemEditorModule.h"

#include "GameplayTagBlueprintPropertyMappingDetails.h"
#include "EditorAttributeCustomization.h"
#include "FractionCustomization.h"

IMPLEMENT_GAME_MODULE(FGameplaySystemEditorModule, GameplaySystemEditor);

void FGameplaySystemEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("GameplayTagBlueprintPropertyMapping", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGameplayTagBlueprintPropertyMappingDetails::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("EditorAttribute", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FEditorAttributeCustomization::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("Fraction", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FFractionCustomization::MakeInstance));
}

void FGameplaySystemEditorModule::ShutdownModule()
{
	// Unregister customizations
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout("GameplayTagBlueprintPropertyMapping");
		PropertyModule.UnregisterCustomPropertyTypeLayout("EditorAttribute");
		PropertyModule.UnregisterCustomPropertyTypeLayout("Fraction");
	}
}