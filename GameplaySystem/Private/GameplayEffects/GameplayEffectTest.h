// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#pragma once

// WITH_EDITOR
#if false

#include "CoreMinimal.h"


#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

#include "GameplaySystemComponent.h"
#include "AttributeEffect.h"
#include "AttributeDataSet.h"

static UWorld* GetWorld();
static void ExitWorld();


IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayEffectTestSimple, "VerticalSlice.GameplayEffectTests.SimpleEffects", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

/* Testing if simple addition of multiple Attributes works, when applied as one single GameplayEffect.
* In short, ensures that these functions work:
- Addition on multiple Attributes is done correctly
- Infinite duration Effects are applied correctly
- Recalculation works, and is done in the right way
*/
bool FGameplayEffectTestSimple::RunTest(const FString& Parameters)
{
    // ---                  Creating objects                     ---
   
	UAttributeDataSet* AttributeDataSet = NewObject<UAttributeDataSet>();
	
	UGameplaySystemComponent* GameplaySystem = NewObject<UGameplaySystemComponent>();

    UGameplayEffect* GameplayEffect = NewObject<UGameplayEffect>();
	GameplayEffect->DurationType = EDurationType::EDT_Infinite;



    // ---                  Setting up properties                     ---

    // Create some default Attributes
	FEditorAttribute HealthAttribute = FEditorAttribute(100, 100);
	FEditorAttribute MaxHealthAttribute = FEditorAttribute(100, 100);
	FEditorAttribute DamageAttribute = FEditorAttribute(10, 10);

    // Create different variations of Attribute Effects
	FAttributeEffect HealthEffect = FAttributeEffect(EAttributeType::EAT_Health, 50.0f, EEffectApplicationType::EEAT_Addition, ETargetValue::ETV_CurrentValue);

	FAttributeEffect MaxHealthEffect = FAttributeEffect(EAttributeType::EAT_MaxHealth, 10.0f, EEffectApplicationType::EEAT_Addition, ETargetValue::ETV_CurrentValue);

	FAttributeEffect DamageEffect = FAttributeEffect(EAttributeType::EAT_Damage, -20.0f, EEffectApplicationType::EEAT_Addition, ETargetValue::ETV_CurrentValue);

    // Populate with the Attributes
	AttributeDataSet->Attributes.Add(EAttributeType::EAT_Health, HealthAttribute);
	AttributeDataSet->Attributes.Add(EAttributeType::EAT_MaxHealth, MaxHealthAttribute);
	AttributeDataSet->Attributes.Add(EAttributeType::EAT_Damage, DamageAttribute);

    // Add Attribute Effects to the Gameplay Effect
    GameplayEffect->AttributeEffects.Add(HealthEffect);
    GameplayEffect->AttributeEffects.Add(MaxHealthEffect);
    GameplayEffect->AttributeEffects.Add(DamageEffect);



    // Initialize the GameplaySystem
    GameplaySystem->SetAttributeDataSet(AttributeDataSet);
    GameplaySystem->BeginPlay();


    // Apply the Gameplay Effect
    FGameplayEffectHandle Handle;
    bool bAppliedSuccessfully = GameplaySystem->AddGameplayEffect(GameplayEffect, Handle);
	AddErrorIfFalse(bAppliedSuccessfully, TEXT("Gameplay Effect was not applied successfully"));


    // ---                  Perform tests                     ---

    TestEqual(TEXT("Health should be increased by 50"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_Health), 150.0f);
    TestEqual(TEXT("Max Health should be increased by 10"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_MaxHealth), 110.0f);
    TestEqual(TEXT("Damage should be decreased by 20"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_Damage), -10.0f);

    // Clean up
	AttributeDataSet->RemoveFromRoot();
    GameplaySystem->RemoveFromRoot();

    GameplayEffect->RemoveFromRoot();

    return true;
}





IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayEffectTestInstant, "VerticalSlice.GameplayEffectTests.InstantEffects", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

/* Testing if Instant effects work and are applied when added.
* In short, ensures that these functions work:
- Instant Effects are calculated when added, and not stored internally
- Instant Effects are required to be applied on BaseValue in order for the change to be permanent
*/
bool FGameplayEffectTestInstant::RunTest(const FString& Parameters)
{
    // ---                  Creating objects                     ---

    UAttributeDataSet* AttributeDataSet = NewObject<UAttributeDataSet>();

    UGameplaySystemComponent* GameplaySystem = NewObject<UGameplaySystemComponent>();

    UGameplayEffect* GameplayEffect = NewObject<UGameplayEffect>();



    // ---                  Setting up properties                     ---

    // Create some default Attributes
    FEditorAttribute HealthAttribute = FEditorAttribute(100, 100);
    FEditorAttribute MaxHealthAttribute = FEditorAttribute(100, 100);
    FEditorAttribute DamageAttribute = FEditorAttribute(10, 10);

    // Create different variations of Attribute Effects
    FAttributeEffect HealthEffect = FAttributeEffect(EAttributeType::EAT_Health, 50.0f, EEffectApplicationType::EEAT_Addition, ETargetValue::ETV_BaseValue);

    FAttributeEffect MaxHealthEffect = FAttributeEffect(EAttributeType::EAT_MaxHealth, 10.0f, EEffectApplicationType::EEAT_Addition, ETargetValue::ETV_BaseValue);

    FAttributeEffect DamageEffect = FAttributeEffect(EAttributeType::EAT_Damage, -20.0f, EEffectApplicationType::EEAT_Addition, ETargetValue::ETV_BaseValue);

    // Populate with the Attributes
    AttributeDataSet->Attributes.Add(EAttributeType::EAT_Health, HealthAttribute);
    AttributeDataSet->Attributes.Add(EAttributeType::EAT_MaxHealth, MaxHealthAttribute);
    AttributeDataSet->Attributes.Add(EAttributeType::EAT_Damage, DamageAttribute);

    // Add Attribute Effects to the Gameplay Effect
    GameplayEffect->AttributeEffects.Add(HealthEffect);
    GameplayEffect->AttributeEffects.Add(MaxHealthEffect);
    GameplayEffect->AttributeEffects.Add(DamageEffect);



    // Initialize the GameplaySystem
    GameplaySystem->SetAttributeDataSet(AttributeDataSet);
    GameplaySystem->BeginPlay();

    // Apply the Gameplay Effect
    FGameplayEffectHandle Handle;
    bool bAppliedSuccessfully = GameplaySystem->AddGameplayEffect(GameplayEffect, Handle);
    AddErrorIfFalse(bAppliedSuccessfully, TEXT("Gameplay Effect was not applied successfully"));


    // ---                  Perform tests                     ---

    // 100 + 50
    TestEqual(TEXT("Health"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_Health), 150.0f);
    // 100 + 10
    TestEqual(TEXT("Max Health"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_MaxHealth), 110.0f);
    // 10 - 20
    TestEqual(TEXT("Damage"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_Damage), -10.0f);

    TestEqual(TEXT("GameplayEffects"), GameplaySystem->GetActiveGameplayEffectsCount(), 0);

    // Clean up
    AttributeDataSet->RemoveFromRoot();
    GameplaySystem->RemoveFromRoot();

    GameplayEffect->RemoveFromRoot();

    return true;
}





IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayEffectTestCompound, "VerticalSlice.GameplayEffectTests.CompoundEffects", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

/* Testing if the Attributes values are as expected when using multiple effects of different types and values, and seeing if its recalculated correctly when Effects are removed.
* In short, ensures that these functions work:
- Multiplicative Effects are calculated correctly
- Removing multiplicative Effects reverts the affected Attributes value correctly
- Removing effects through API
*/ 
bool FGameplayEffectTestCompound::RunTest(const FString& Parameters)
{
    // ---                  Creating objects                     ---

    UAttributeDataSet* AttributeDataSet = NewObject<UAttributeDataSet>();

    UGameplaySystemComponent* GameplaySystem = NewObject<UGameplaySystemComponent>();

    UGameplayEffect* GameplayEffect = NewObject<UGameplayEffect>();
    GameplayEffect->DurationType = EDurationType::EDT_Infinite;
    UGameplayEffect* GameplayEffectToRemove = NewObject<UGameplayEffect>();
    GameplayEffectToRemove->DurationType = EDurationType::EDT_Infinite;




    // ---                  Setting up properties                     ---

    // Create some default Attributes
    FEditorAttribute HealthAttribute = FEditorAttribute(100, 100);
    FEditorAttribute MaxHealthAttribute = FEditorAttribute(100, 100);
    FEditorAttribute DamageAttribute = FEditorAttribute(10, 10);

    // Create different variations of Attribute Effects
	FAttributeEffect HealthEffectMult = FAttributeEffect(EAttributeType::EAT_Health, 2.0f, EEffectApplicationType::EEAT_Multiplication, ETargetValue::ETV_CurrentValue);

	FAttributeEffect HealthEffect = FAttributeEffect(EAttributeType::EAT_Health, 10.0f, EEffectApplicationType::EEAT_Addition, ETargetValue::ETV_CurrentValue);

	FAttributeEffect MaxHealthEffect = FAttributeEffect(EAttributeType::EAT_MaxHealth, 10.0f, EEffectApplicationType::EEAT_Addition, ETargetValue::ETV_CurrentValue);

	FAttributeEffect DamageEffect = FAttributeEffect(EAttributeType::EAT_Damage, -20.0f, EEffectApplicationType::EEAT_Addition, ETargetValue::ETV_CurrentValue);

    // Populate with the Attributes
    AttributeDataSet->Attributes.Add(EAttributeType::EAT_Health, HealthAttribute);
    AttributeDataSet->Attributes.Add(EAttributeType::EAT_MaxHealth, MaxHealthAttribute);
    AttributeDataSet->Attributes.Add(EAttributeType::EAT_Damage, DamageAttribute);

    // Add Attribute Effects to the Gameplay Effect
    GameplayEffect->AttributeEffects.Add(HealthEffect);
    GameplayEffect->AttributeEffects.Add(HealthEffectMult);
    GameplayEffect->AttributeEffects.Add(MaxHealthEffect);
    GameplayEffect->AttributeEffects.Add(DamageEffect);


	FAttributeEffect MaxHealthEffectMult = FAttributeEffect(EAttributeType::EAT_MaxHealth, 1.5f, EEffectApplicationType::EEAT_Multiplication, ETargetValue::ETV_CurrentValue);

    GameplayEffectToRemove->AttributeEffects.Add(MaxHealthEffectMult);


    // Initialize the GameplaySystem
    GameplaySystem->SetAttributeDataSet(AttributeDataSet);
    GameplaySystem->BeginPlay();


    // Apply the Gameplay Effects
    FGameplayEffectHandle Handle;
    FGameplayEffectHandle HandleToRemove;

    bool bAppliedSuccessfully = GameplaySystem->AddGameplayEffect(GameplayEffect, Handle);
    bool bAppliedSuccessfully2 = GameplaySystem->AddGameplayEffect(GameplayEffectToRemove, HandleToRemove);
    AddErrorIfFalse(bAppliedSuccessfully, TEXT("Gameplay Effect was not applied successfully"));
    AddErrorIfFalse(bAppliedSuccessfully2, TEXT("Gameplay Effect was not applied successfully"));



    // ---                  Perform tests                     ---

    // 4 in first effect, 1 in to remove effect
    TestEqual(TEXT("Attribute Effects"), GameplaySystem->GetActiveEffectsCount(), 5);

    // 2
    TestEqual(TEXT("Gameplay Effects"), GameplaySystem->GetActiveGameplayEffectsCount(), 2);

    // 100 + 10 * 2
    TestEqual(TEXT("Health"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_Health), 220.0f);

    // 100 + 10 * 1.5
    TestEqual(TEXT("Max Health"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_MaxHealth), 165.0f);

    // 10 - 20
    TestEqual(TEXT("Damage"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_Damage), -10.0f);



    bool bRemovesSuccessfully = GameplaySystem->RemoveGameplayEffectByHandle(HandleToRemove);
	AddErrorIfFalse(bRemovesSuccessfully, TEXT("Gameplay Effect was not removed successfully"));

    // 100 + 10 
    TestEqual(TEXT("Max Health"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_MaxHealth), 110.0f);





    // Clean up
    AttributeDataSet->RemoveFromRoot();
    GameplaySystem->RemoveFromRoot();

    GameplayEffect->RemoveFromRoot();
    GameplayEffectToRemove->RemoveFromRoot();

    return true;
}





IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayEffectTestDataIntegrity, "VerticalSlice.GameplayEffectTests.DataIntegrity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

/* Testing if the Class comparison works, and if the pointers cause issues with data integrity.
* We try to change the GameplayEffects after application to see if the already existing ones in the GameplayEffectComponent still work, and 
* we see if we can remove Effects despite it not being a exact copy of existing ones. In short, ensures that these functions work:
- Copying of effects in GameplayEffectComponent
- Modifying effects after application through API
- Removing effects through API
*/
bool FGameplayEffectTestDataIntegrity::RunTest(const FString& Parameters)
{
    // ---                  Creating objects

    UAttributeDataSet* AttributeDataSet = NewObject<UAttributeDataSet>();

    UGameplaySystemComponent* GameplaySystem = NewObject<UGameplaySystemComponent>();

    UGameplayEffect* GameplayEffect = NewObject<UGameplayEffect>();
	GameplayEffect->DurationType = EDurationType::EDT_Infinite;



    // ---                  Setting up properties

    // Create some default Attributes
    FEditorAttribute HealthAttribute = FEditorAttribute(100, 100);
    FEditorAttribute MaxHealthAttribute = FEditorAttribute(100, 100);
    FEditorAttribute DamageAttribute = FEditorAttribute(10, 10);

    // Create different variations of Attribute Effects
	FAttributeEffect HealthEffect = FAttributeEffect(EAttributeType::EAT_Health, 50.0f, EEffectApplicationType::EEAT_Addition, ETargetValue::ETV_CurrentValue);

	FAttributeEffect MaxHealthEffect = FAttributeEffect(EAttributeType::EAT_MaxHealth, 10.0f, EEffectApplicationType::EEAT_Addition, ETargetValue::ETV_CurrentValue);



    // Populate with the Attributes
    AttributeDataSet->Attributes.Add(EAttributeType::EAT_Health, HealthAttribute);
    AttributeDataSet->Attributes.Add(EAttributeType::EAT_MaxHealth, MaxHealthAttribute);

    // Add Attribute Effects to the Gameplay Effect
    GameplayEffect->AttributeEffects.Add(HealthEffect);
    GameplayEffect->AttributeEffects.Add(MaxHealthEffect);



    // Initialize the GameplaySystem
    GameplaySystem->SetAttributeDataSet(AttributeDataSet);

    // Apply the Gameplay Effect

    FGameplayEffectHandle Handle;
    bool bAppliedSuccessfully = GameplaySystem->AddGameplayEffect(GameplayEffect, Handle);
    AddErrorIfFalse(bAppliedSuccessfully, TEXT("Gameplay Effect was not applied successfully"));


    // ---                  Perform tests                     ---

    // 100 + 50
    TestEqual(TEXT("Health"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_Health), 150.0f);

    // 100 + 10
    TestEqual(TEXT("Max Health"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_MaxHealth), 110.0f);


    FActiveGameplayEffect NewEffect(GameplayEffect);

	int EffectIndex = NewEffect.GetAttributeEffect(HealthEffect);

    AddErrorIfFalse(EffectIndex != INDEX_NONE, TEXT("AttributeEffect was not found in the GameplayEffectComponent"));

    //Index is valid
    if (EffectIndex != INDEX_NONE) 
    {
        NewEffect.AttributeEffects[EffectIndex].Value = 1.2f;
    }

    FGameplayEffectHandle Handle2;
    GameplaySystem->AddGameplayEffectByHandle(NewEffect, Handle2);

    // 100 + 50 + 1.2
	TestEqual(TEXT("Health"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_Health), 151.2f);


    HealthEffect.Value = 1812484.f;
	bool bRemovedSuccessfully = GameplaySystem->RemoveGameplayEffect(GameplayEffect);
	AddErrorIfFalse(bRemovedSuccessfully, TEXT("Gameplay Effect was not removed successfully"));

    // Clean up
    AttributeDataSet->RemoveFromRoot();
    GameplaySystem->RemoveFromRoot();

    GameplayEffect->RemoveFromRoot();

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameplayEffectTestUnique, "VerticalSlice.GameplayEffectTests.Uniques", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

/* Testing if the Unique effects can only be applied once.
- Unique application keeps intended behaviour, prohibiting multiple copies of the same GameplayEffect being added
*/
bool FGameplayEffectTestUnique::RunTest(const FString& Parameters)
{
    // ---                  Creating objects                     ---

    UAttributeDataSet* AttributeDataSet = NewObject<UAttributeDataSet>();

    UGameplaySystemComponent* GameplaySystem = NewObject<UGameplaySystemComponent>();

    UGameplayEffect* GameplayEffect = NewObject<UGameplayEffect>();
    GameplayEffect->DurationType = EDurationType::EDT_Infinite;
    GameplayEffect->bIsUnique = true;



    // ---                  Setting up properties                     ---

    // Create some default Attributes
    FEditorAttribute DamageAttribute = FEditorAttribute(10, 10);

    // Create different variations of Attribute Effects
    FAttributeEffect DamageEffect = FAttributeEffect(EAttributeType::EAT_Damage, 10.0f, EEffectApplicationType::EEAT_Addition, ETargetValue::ETV_CurrentValue);


    // Populate with the Attributes
    AttributeDataSet->Attributes.Add(EAttributeType::EAT_Damage, DamageAttribute);

    // Add Attribute Effects to the Gameplay Effect
    GameplayEffect->AttributeEffects.Add(DamageEffect);

    // Initialize the GameplaySystem
    GameplaySystem->SetAttributeDataSet(AttributeDataSet);

    // Apply the Gameplay Effect
    FGameplayEffectHandle Handle;
    bool bAppliedSuccessfully = GameplaySystem->AddGameplayEffect(GameplayEffect, Handle);
    AddErrorIfFalse(bAppliedSuccessfully, TEXT("Gameplay Effect was not applied successfully"));


    // ---                  Perform tests                     ---

    // 10 + 10
    TestEqual(TEXT("Damage"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_Damage), 20.0f);

    FGameplayEffectHandle Handle2;
	bool bAppliedSuccessfully2 = GameplaySystem->AddGameplayEffect(GameplayEffect, Handle2);

	AddErrorIfFalse(!bAppliedSuccessfully2, TEXT("Gameplay Effect was applied successfully the second time"));

    // 10 + 10
    TestEqual(TEXT("Damage"), GameplaySystem->GetAttributeValue(EAttributeType::EAT_Damage), 20.0f);

    bool bRemovedSuccessfully = GameplaySystem->RemoveGameplayEffect(GameplayEffect);
    AddErrorIfFalse(bRemovedSuccessfully, TEXT("Gameplay Effect was not removed successfully"));

	TestEqual(TEXT("Gameplay Effects"), GameplaySystem->GetActiveGameplayEffectsCount(), 0);

    // Clean up
    AttributeDataSet->RemoveFromRoot();

    GameplaySystem->RemoveFromRoot();

    GameplayEffect->RemoveFromRoot();

    return true;
}

#endif //WITH_EDITOR