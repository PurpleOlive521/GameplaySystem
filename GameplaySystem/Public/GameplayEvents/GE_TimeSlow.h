// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEvents/GameplayEvent.h"
#include "TimeSlowAsset.h"
#include "GE_TimeSlow.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UGE_TimeSlow : public UGameplayEvent
{
	GENERATED_BODY()

public:

	UGE_TimeSlow();

#if WITH_EDITORONLY_DATA

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

#endif //WITH_EDITORONLY_DATA

	virtual void TriggerEvent(const FGameplayEventActivationData& ActivationData) override;

	// Only inverts if bInvertTimeDilation is true, does nothing otherwise.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GE_TimeSlow")
	float TryInvert(float Value) const;

protected:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GE_TimeSlow")
	TObjectPtr<UTimeSlowAsset> Asset = nullptr;

	// Inverts the value. Use to undo the supplied Asset, instead of needing to create a new inverted Asset separately.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GE_TimeSlow")
	bool bInvertTimeDilation = false;

	// Forces the Type to Global and compensates so that the Owner is unaffected by the TimeDilation.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GE_TimeSlow")
	bool bCompensateOnActivator = false;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GE_TimeSlow")
	ETimeDilationType TimeDilationType = ETimeDilationType::ETDT_Global;
};
