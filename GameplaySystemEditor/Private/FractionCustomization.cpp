// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "FractionCustomization.h"
#include "Fraction.h"
#include "EditorCustomizationHelpers.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"

TSharedRef<IPropertyTypeCustomization> FFractionCustomization::MakeInstance()
{
	return MakeShared<FFractionCustomization>();
}

void FFractionCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
}

void FFractionCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	const FSimpleDelegate OnValueChangedDelegate = FSimpleDelegate::CreateSP(this, &FFractionCustomization::OnPropertyChanged, StructCustomizationUtils.GetPropertyUtilities());
	InStructPropertyHandle->SetOnChildPropertyValueChanged(OnValueChangedDelegate);

	TSharedPtr<IPropertyHandle> NumeratorPropertyHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFraction, Numerator));
	TSharedPtr<IPropertyHandle> DenominatorPropertyHandle = InStructPropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FFraction, Denominator));

	int Numerator = GetPropertyRaw<int>(NumeratorPropertyHandle);
	int Denominator = GetPropertyRaw<int>(DenominatorPropertyHandle);

	FFraction Temporary = { Numerator, Denominator };

	const FString Percentage = FString::Printf(TEXT("%.2f%%"), Temporary.GetPercentage());
	const FString Title = INVTEXT("Fraction: ").ToString();
	const FText FinalRowName = FText::AsCultureInvariant(Title + Percentage);

	ChildBuilder.AddCustomRow(INVTEXT("Fraction: "))
		.NameContent()
		[
			SNew(STextBlock)
				.Text(FinalRowName)
				.Font(StructCustomizationUtils.GetRegularFont())
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5.0f, 3.0f)
				[
					SNew(SBox)
						.MinDesiredWidth(50.0f)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									NumeratorPropertyHandle->CreatePropertyNameWidget()
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									NumeratorPropertyHandle->CreatePropertyValueWidget()
								]
						]
				]

			+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5.0f, 3.0f)
				[
					SNew(SBox)
						.MinDesiredWidth(50.0f)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(SVerticalBox)
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									DenominatorPropertyHandle->CreatePropertyNameWidget()
								]
								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									DenominatorPropertyHandle->CreatePropertyValueWidget()
								]
						]
				]
		];

}

void FFractionCustomization::OnPropertyChanged(TSharedPtr<IPropertyUtilities> Utils)
{
	if (Utils)
	{
		Utils->RequestForceRefresh();
	}
}
