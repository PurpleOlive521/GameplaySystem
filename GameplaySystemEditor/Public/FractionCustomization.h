// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "UObject/FieldPath.h"
#include "IPropertyTypeCustomization.h"

/**
 *
 */
class GAMEPLAYSYSTEMEDITOR_API FFractionCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();


	// Begin IDetailCustomization Interface
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	// End IDetailCustomization Interface

	void OnPropertyChanged(TSharedPtr<IPropertyUtilities> Utils);
};
