// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once


#include "CoreMinimal.h"

struct FArchiveSectionContainer;

constexpr uint64 INVALID_SECTION_OFFSET = 0U;

struct FArchiveSection
{
	enum class EScopeType : uint8
	{
		Unset,
		Header,
		Sublevel,
		Global,
		Versions
	};

	FArchiveSection(FArchiveSectionContainer* InContainer, EScopeType InType) : Container(InContainer), Type(InType) {};

	// Returns true if both StartOffset and EndOffset are INVALID_SECTION_OFFSET.
	bool IsUnset() const;

	// Asserts if the Section is malformed.
	void CheckErrors() const;

	uint64 GetSize() const;

	void MarkSizeChanged();

	// Moves the start and end offset Delta amount
	void Shift(int64 Delta);

	// Returns true if OtherScope starts after this Section ends.
	bool IsAfter(const FArchiveSection& OtherScope) const;

	FArchiveSectionContainer* Container = nullptr;

	uint64 PreviousSize = 0U;

	uint64 StartOffset = 0U;

	uint64 EndOffset = 0U;

	EScopeType Type = EScopeType::Unset;

	bool operator==(const FArchiveSection& Other) const
	{
		return	Container == Other.Container &&
			PreviousSize == Other.PreviousSize &&
			StartOffset == Other.StartOffset &&
			EndOffset == Other.EndOffset &&
			Type == Other.Type;
	}

	bool operator!=(const FArchiveSection& Other) const
	{
		return	Container != Other.Container &&
			PreviousSize != Other.PreviousSize &&
			StartOffset != Other.StartOffset &&
			EndOffset != Other.EndOffset &&
			Type != Other.Type;

	}
};
