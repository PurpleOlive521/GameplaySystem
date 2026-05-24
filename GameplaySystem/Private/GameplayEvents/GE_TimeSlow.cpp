// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayEvents/GE_TimeSlow.h"
#include "DevelopmentTypes.h"

UGE_TimeSlow::UGE_TimeSlow()
{
    DisplayName = "TimeSlow";

    TickSource = ETickSource::ETS_AbsoluteDeltaTime;
    bTickWhenPaused = false;
}

#if WITH_EDITORONLY_DATA

void UGE_TimeSlow::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (bCompensateOnActivator)
    {
        TimeDilationType = ETimeDilationType::ETDT_Global;
    }
}

#endif //WITH_EDITORONLY_DATA

void UGE_TimeSlow::TriggerEvent(const FGameplayEventActivationData& ActivationData)
{
    Super::TriggerEvent(ActivationData);

    if (!Asset)
    {
        GE_LOG(Error, TEXT("UGE_TimeSlow triggered without a valid TimeSlowAsset!"));
        return;
    }
}

float UGE_TimeSlow::TryInvert(float Value) const
{
    if (not bInvertTimeDilation)
    {
        return Value;
    }

    if (Value == 0.0f)
    {
        return Value;
    }

    return 1.0f / Value;
}
