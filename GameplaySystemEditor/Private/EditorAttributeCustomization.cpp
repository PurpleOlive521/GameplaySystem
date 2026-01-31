// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


#include "EditorAttributeCustomization.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "AttributeDataSet.h"
#include "GameplaySystemBlueprintLibrary.h"

TSharedRef<IPropertyTypeCustomization> FEditorAttributeCustomization::MakeInstance()
{
	return MakeShared<FEditorAttributeCustomization>();
}

void FEditorAttributeCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

void FEditorAttributeCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) 
{
	TSharedPtr<IPropertyHandle> TypePropertyHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FEditorAttribute, Type));
	TSharedPtr<IPropertyHandle> BaseValuePropertyHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FEditorAttribute, BaseValue));

	check(TypePropertyHandle && BaseValuePropertyHandle);

	TArray<void*> RawData;
	TypePropertyHandle->AccessRawData(RawData);

	EAttributeType AttributeType = EAttributeType::EAT_NONE;
	if (RawData.Num() > 0)
	{
		AttributeType = *static_cast<EAttributeType*>(RawData[0]);
	}

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


