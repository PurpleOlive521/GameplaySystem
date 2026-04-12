// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once


#include "CoreMinimal.h"
#include "ArchiveSection.h"

constexpr int32 INVALID_SECTION_INDEX = -1;

struct FArchiveSectionContainer
{
	FArchiveSectionContainer() = default;

	// Returns the key to SectionToSearch, or an empty string if none can be found.
	FString GetSectionName(const FArchiveSection& SectionToSearch) const;

	void OnSectionResized(FArchiveSection& ChangedOffset, int64 Delta);

	FArchiveSection& GetOrAddSection(const FString& Name, FArchiveSection::EScopeType Type);

	const FArchiveSection* GetSection(const FString& Name) const;

	// Renames the given Section. Will invalidate any references or pointers to the Section!
	void RenameSection(FArchiveSection& Section, const FString& NewName);

	// Returns true if a Section by Name exists, false otherwise.
	bool HasSection(const FString& Name) const;

	// Returns INVALID_SPAN_INDEX if Name isn't a valid Section
	int32 GetSectionIndex(const FString& Name) const;

	// Returns a empty string if no Section is found at Index
	FString GetSectionAtIndex(int32 Index) const;

	const FArchiveSection* GetLastSectionOfType(FArchiveSection::EScopeType Type) const;

	bool RemoveSection(const FString& Name);

	// Clears out any Sections that are being tracked.
	void Reset();

	TMap<FString, FArchiveSection> Sections;
};
