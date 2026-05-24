// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DebugPageWidget.generated.h"

class UGameplaySystemDebugWidget;

/**
 * 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UDebugPageWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UDebugPageWidget(const FObjectInitializer& ObjectInitializer);

	// --- Begin UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// --- End UUUserWidget interface

	// Requested when the owning DebugWidget deems it safe to update the page.
	UFUNCTION(BlueprintCallable, Category = "DebugPageWidget")
	void OnSafeTick(UGameplaySystemDebugWidget* OwningDebugWidget, float DeltaTime);

	// Requested when the owning DebugWidget deems it safe to update the page.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "BP On Safe Tick"), Category = "DebugPageWidget")
	void K2_OnSafeTick(UGameplaySystemDebugWidget* OwningDebugWidget, float DeltaTime);

	// Clear all content that depends on OnSafeTick, or is invalid if not updated through it.
	UFUNCTION(BlueprintCallable, Category = "DebugPageWidget")
	void ClearDisplay();

	// Clear all content that depends on OnSafeTick, or is invalid if not updated through it.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "BP Clear Display"), Category = "DebugPageWidget")
	void K2_ClearDisplay();

protected:

	UPROPERTY(BlueprintReadWrite, Category = "DebugPageWidget")
	FText FallbackText = {};
	
};
