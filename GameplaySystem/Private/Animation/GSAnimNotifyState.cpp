// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GSAnimNotifyState.h"

#if WITH_EDITOR

void UGSAnimNotifyState::OnAnimNotifyCreatedInEditor(FAnimNotifyEvent& ContainingAnimNotifyEvent)
{
	bIsPlayingInEditor = true;
}

#endif //WITH_EDITOR