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
		check(NewSize <= INT64_MAX); // Overflow will occur when calculating difference
		check(PreviousSize <= INT64_MAX); 

		int64 Delta = 0;
		const bool bNoOverflow = FMath::SubtractAndCheckForOverflow((int64)NewSize, (int64)PreviousSize, Delta);
		check(bNoOverflow);

		Container->OnSectionResized(*this, Delta);
		PreviousSize = NewSize;
	}
}

void FArchiveSection::Shift(int64 Delta)
{
	bool bNoOverflow = true;

	if (Delta < 0)
	{
		uint64 NewDelta = uint64(abs(Delta));
		bNoOverflow &= FMath::SubtractAndCheckForOverflow(StartOffset, NewDelta, StartOffset);
		bNoOverflow &= FMath::SubtractAndCheckForOverflow(EndOffset, NewDelta, EndOffset);
	}
	else
	{
		uint64 NewDelta = uint64(Delta);
		bNoOverflow &= FMath::AddAndCheckForOverflow(StartOffset, NewDelta, StartOffset);
		bNoOverflow &= FMath::AddAndCheckForOverflow(EndOffset, NewDelta, EndOffset);
	}

	check(bNoOverflow);
}

bool FArchiveSection::IsAfter(const FArchiveSection& OtherScope) const
{
	CheckErrors();
	return OtherScope.StartOffset < EndOffset && OtherScope.EndOffset <= EndOffset;
}