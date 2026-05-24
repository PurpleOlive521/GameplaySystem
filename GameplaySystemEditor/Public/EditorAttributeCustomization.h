// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "UObject/FieldPath.h"
#include "IPropertyTypeCustomization.h"

/**
 * 
 */
class GAMEPLAYSYSTEMEDITOR_API FEditorAttributeCustomization : public IPropertyTypeCustomization
{
public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it. */
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	// IDetailCustomization interface

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> InStructPropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	void OnPropertyChanged(TSharedPtr<IPropertyUtilities> Utils);
};
