// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GSAnimNotify.h"

#if WITH_EDITOR

void UGSAnimNotify::OnAnimNotifyCreatedInEditor(FAnimNotifyEvent& ContainingAnimNotifyEvent)
{
	bIsPlayingInEditor = true;
}

#endif //WITH_EDITOR