// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "ArchiveSection.h"
#include "ArchiveSectionContainer.h"

bool FArchiveSection::IsUnset() const
{
	return StartOffset == INVALID_SECTION_OFFSET &&
		EndOffset == INVALID_SECTION_OFFSET;
}

void FArchiveSection::CheckErrors() const
{
	check(StartOffset <= EndOffset);
}

uint64 FArchiveSection::GetSize() const
{
	CheckErrors();

	return EndOffset - StartOffset;
}

void FArchiveSection::MarkSizeChanged()
{
	CheckErrors();

	uint64 NewSize = GetSize();
	if (NewSize != PreviousSize)
	{
		check(NewSize < INT64_MAX); // Overflow will occur when calculating difference
		int64 Delta = NewSize - PreviousSize;
		Container->OnSectionResized(*this, Delta);
		PreviousSize = NewSize;
	}
}

void FArchiveSection::Shift(int64 Delta)
{
	if (Delta < 0)
	{
		uint64 NewDelta = uint64(abs(Delta));
		check(FMath::SubtractAndCheckForOverflow(StartOffset, NewDelta, StartOffset));
		check(FMath::SubtractAndCheckForOverflow(EndOffset, NewDelta, EndOffset));
	}
	else
	{
		uint64 NewDelta = uint64(Delta);
		check(FMath::AddAndCheckForOverflow(StartOffset, NewDelta, StartOffset));
		check(FMath::AddAndCheckForOverflow(EndOffset, NewDelta, EndOffset));
	}
}

bool FArchiveSection::IsAfter(const FArchiveSection& OtherScope) const
{
	CheckErrors();
	return OtherScope.StartOffset < EndOffset && OtherScope.EndOffset <= EndOffset;
}