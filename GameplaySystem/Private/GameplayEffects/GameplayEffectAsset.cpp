// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayEffects/GameplayEffectAsset.h"

FPrimaryAssetId UGameplayEffectVisualsAsset::GetPrimaryAssetId() const
{
	return FPrimaryAssetId("GameplayEffectVisualsItems", GetFName());
}

UObject* UGameplayEffectVisualsAsset::GetIcon() const
{
	return Icon.LoadSynchronous();
}