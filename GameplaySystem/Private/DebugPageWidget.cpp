// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "DebugPageWidget.h"
#include "GameplaySystemDebugWidget.h"

UDebugPageWidget::UDebugPageWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	FString DefaultString = TEXT("<Default_Red>Invalid Configuration.</>");
	FallbackText = FText::AsCultureInvariant(DefaultString);
}

void UDebugPageWidget::NativeConstruct()
{

}

void UDebugPageWidget::NativeDestruct()
{
}

void UDebugPageWidget::OnSafeTick(UGameplaySystemDebugWidget* OwningDebugWidget, float DeltaTime)
{
	K2_OnSafeTick(OwningDebugWidget, DeltaTime);
}

void UDebugPageWidget::ClearDisplay()
{
	K2_ClearDisplay();
}
