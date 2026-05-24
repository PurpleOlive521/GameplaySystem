// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "IsDefaultConstructible.h"

// Has no type safety since we are handling a raw data pointer.
template<class T>
static T GetPropertyRaw(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	static_assert(TIsDefaultConstructible<T>::Value, "'T' template parameter to GetPropertyRaw must be default constructible!");

	checkf(PropertyHandle, TEXT("PropertyHandle was null when calling GetPropertyRaw - can't extract value."));

	TArray<void*> RawData;
	PropertyHandle->AccessRawData(RawData);

	T Value = {};
	if (RawData.Num() > 0)
	{
		Value = *static_cast<T*>(RawData[0]);
	}

	return Value;
}
