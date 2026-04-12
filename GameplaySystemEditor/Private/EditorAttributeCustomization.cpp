// Copyright (c) 2026, Heavy Duty Tape Studios. All rights reserved.


#include "EditorAttributeCustomization.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "AttributeDataSet.h"
#include "GameplaySystemBlueprintLibrary.h"
#include "EditorCustomizationHelpers.h"
#include "IPropertyUtilities.h"

TSharedRef<IPropertyTypeCustomization> FEditorAttributeCustomization::MakeInstance()
{
	return MakeShared<FEditorAttributeCustomization>();
}

void FEditorAttributeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

void FEditorAttributeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) 
{
	const FSimpleDelegate OnValueChangedDelegate = FSimpleDelegate::CreateSP(this, &FEditorAttributeCustomization::OnPropertyChanged, StructCustomizationUtils.GetPropertyUtilities());
	InStructPropertyHandle->SetOnChildPropertyValueChanged(OnValueChangedDelegate);

	TSharedPtr<IPropertyHandle> TypePropertyHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FEditorAttribute, Type));
	TSharedPtr<IPropertyHandle> BaseValuePropertyHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FEditorAttribute, BaseValue));

	check(BaseValuePropertyHandle);

	EAttributeType AttributeType = GetPropertyRaw<EAttributeType>(TypePropertyHandle);

	const FText FinalRowName = FText::AsCultureInvariant(INVTEXT("Attribute: ").ToString() + UGameplaySystemBlueprintLibrary::ConvertAttributeToDisplayName(AttributeType));

	ChildBuilder.AddCustomRow(INVTEXT("EditorAttribute: "))
		.NameContent()
		[
			SNew(STextBlock)
				.Text(FinalRowName)
				.Font(StructCustomizationUtils.GetRegularFont())
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f, 3.0f)
			[
				SNew(SBox)
				.MinDesiredWidth(250.0f)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						TypePropertyHandle->CreatePropertyNameWidget()
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						TypePropertyHandle->CreatePropertyValueWidget()
					]
				]
			]

			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(5.0f, 3.0f)
			[
				SNew(SBox)
				.MinDesiredWidth(250.0f)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						BaseValuePropertyHandle->CreatePropertyNameWidget()
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						BaseValuePropertyHandle->CreatePropertyValueWidget()
					]
				]
			]
		];
		
}

void FEditorAttributeCustomization::OnPropertyChanged(TSharedPtr<IPropertyUtilities> Utils)
{
	if (Utils)
	{
		Utils->RequestForceRefresh();
	}
}


