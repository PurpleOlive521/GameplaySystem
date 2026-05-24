// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayEffectAsset.generated.h"

/**
 * Contains any visual assets for a GameplayEffect that is to be represented or displayed in game UI.
 * All content assets should be Soft pointers to avoid always loading them whenever the represented GameplayEffect is used.
 */
UCLASS()
class GAMEPLAYSYSTEM_API UGameplayEffectVisualsAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:

	// --- Begin UPrimaryDataAsset Interface
	FPrimaryAssetId GetPrimaryAssetId() const override;
	// --- End UPrimaryDataAsset Interface

	// Can return nullptr if no Icon is selected for this asset.
	UFUNCTION(BlueprintCallable, Category = "GameplayEffectAsset")
	UObject* GetIcon() const;

	// Matching FSlateBrush::ResourceObject implementation.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (DisplayThumbnail = "true", AllowedClasses = "/Script/Engine.Texture,/Script/Engine.MaterialInterface,/Script/Engine.SlateTextureAtlasInterface", DisallowedClasses = "/Script/MediaAssets.MediaTexture"), Category = "GameplayEffectAsset")
	TSoftObjectPtr<UObject> Icon = nullptr;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayEffectAsset")
	FLinearColor DisplayColor = FLinearColor::White;
};
