// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "ArchiveSectionContainer.h"

FString FArchiveSectionContainer::GetSectionName(const FArchiveSection& SectionToSearch) const
{
	for (const auto& [Name, Section] : Sections)
	{
		// Compare by adress
		if (&Section == &SectionToSearch)
		{
			return Name;
		}
	}

	return FString();
}

void FArchiveSectionContainer::OnSectionResized(FArchiveSection& ChangedOffset, int64 Delta)
{
	FString Key = GetSectionName(ChangedOffset);

	uint64 OldEndOffset;
	if (Delta < 0)
	{
		check(FMath::AddAndCheckForOverflow(ChangedOffset.EndOffset, uint64(Delta), OldEndOffset));
	}
	else
	{
		check(FMath::SubtractAndCheckForOverflow(ChangedOffset.EndOffset, uint64(abs(Delta)), OldEndOffset));
	}

	if (not Key.IsEmpty())
	{
		for (auto& [Name, Section] : Sections)
		{
			// Don't move the resized Section
			if (Key == Name)
			{
				continue;
			}

			// All Sections after the resized Section will need to be moved Delta amount
			if (Section.StartOffset >= OldEndOffset)
			{
				Section.Shift(Delta);
			}
		}
	}
}

FArchiveSection& FArchiveSectionContainer::GetOrAddSection(const FString& Name, FArchiveSection::EScopeType Type)
{
	ensureAlways(!Name.IsEmpty());

	if (FArchiveSection* Section = Sections.Find(Name))
	{
		check(Section->Type == Type); // An existing Section with the same name exists, but with the wrong type
		return *Section;
	}

	FArchiveSection Section = { this, Type };
	return Sections.Emplace(Name, Section);
}

const FArchiveSection* FArchiveSectionContainer::GetSection(const FString& Name) const
{
	ensureAlways(not Name.IsEmpty());

	return Sections.Find(Name);
}

// Note: We are intentionally creating a copy since a reference or pointer would be invalidated
void FArchiveSectionContainer::RenameSection(FArchiveSection& Section, const FString& NewName)
{
	if (NewName.IsEmpty())
	{
		return;
	}

	FString Key = GetSectionName(Section);

	// Already is the desired name
	if (Key == NewName)
	{
		return;
	}

	FArchiveSection SectionCopy = Section;

	Sections.Remove(Key);

	Sections.Emplace(NewName, SectionCopy);
}

bool FArchiveSectionContainer::HasSection(const FString& Name) const
{
	return Sections.Contains(Name);
}

int32 FArchiveSectionContainer::GetSectionIndex(const FString& InName) const
{
	int32 Index = INVALID_SECTION_INDEX;

	if (const FArchiveSection* ReferenceScope = GetSection(InName))
	{
		Index = 0;

		for (const auto& [Name, Section] : Sections)
		{
			if (ReferenceScope->IsAfter(Section))
			{
				Index++;
			}
		}
	}

	return Index;
}

FString FArchiveSectionContainer::GetSectionAtIndex(int32 Index) const
{
	// TODO: Find a way to cache the indices properly, so we don't need a O(n^2) search to find the right indices....
	for (const auto& [Name, Section] : Sections)
	{
		if (GetSectionIndex(Name) == Index)
		{
			return Name;
		}
	}

	return FString();
}

const FArchiveSection* FArchiveSectionContainer::GetLastSectionOfType(FArchiveSection::EScopeType Type) const
{
	int32 HighestIndex = INT32_MIN;
	const FArchiveSection* HighestScope = nullptr;

	// TODO: Find a way to cache the indices properly, so we don't need a O(n^2) search to find the right indices....
	for (const auto& [Name, Section] : Sections)
	{
		if (Section.Type == Type)
		{
			int32 Index = GetSectionIndex(Name);
			if (Index >= HighestIndex)
			{
				HighestIndex = Index;
				HighestScope = &Section;
			}
		}
	}

	return HighestScope;
}

bool FArchiveSectionContainer::RemoveSection(const FString& Name)
{
	return bool(Sections.Remove(Name));
}

void FArchiveSectionContainer::Reset()
{
	Sections.Reset();
}