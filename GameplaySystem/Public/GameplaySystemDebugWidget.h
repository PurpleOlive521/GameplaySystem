// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "EnhancedInputComponent.h"

#include "GameplaySystemDebugWidget.generated.h"

class UGameplaySystemComponent;
class UInputMappingContext;
class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
struct FEnhancedInputActionEventBinding;
class UGameplayEventSubsystem;

constexpr int PAGE_START_INDEX = 1;

// Can affect performance when using the debug view, since we recalculate attributes every frame with this enabled.
// Will ensure that the displayed attributes are accurate to their true value.
constexpr bool FORCE_RECALCULATE_ATTRIBUTES = false;

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FAttributeDisplayInfo
{
	GENERATED_BODY()

	FAttributeDisplayInfo() = default;

	static FAttributeDisplayInfo MakeInvalid();

	UPROPERTY(BlueprintReadWrite, Category = "AttributeDisplayInfo")
	FString TypeInfo = "";

	UPROPERTY(BlueprintReadWrite, Category = "AttributeDisplayInfo")
	FString ValueInfo = "";
};

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FEventsDisplayInfo
{
	GENERATED_BODY()

	FEventsDisplayInfo() = default;

	static FEventsDisplayInfo MakeInvalid();

	UPROPERTY(BlueprintReadWrite, Category = "EventsDisplayInfo")
	FString ActiveEvents = "";

	UPROPERTY(BlueprintReadWrite, Category = "EventsDisplayInfo")
	FString InactiveEvents = "";
};

/**
 * --- Widget that displays critical system information for easier debugging. 
 * Defaults to the players GameplaySystemComponent, but can be used to cycle through all found GameplaySystemComponent in the level.
 * 
 * For a entity's GameplaySystemComponent to be found, it must implement the IGameplaySystemOwnerInterface.
 */
UCLASS(Blueprintable)
class GAMEPLAYSYSTEM_API UGameplaySystemDebugWidget : public UUserWidget
{
	GENERATED_BODY()

	// We rely on statics to keep the candidates independent of any one widget instance, so that we do not need to manage our lifetime between possible widget instances.

	// TODO: Make sure that cleaning the array wont mess with other active widgets and their indices.
	static void CleanSystemCandidates();
	
	static TArray<TWeakObjectPtr<UGameplaySystemComponent>> ActiveSystemCandidates;

public:

	// --- Begin UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// --- End UUUserWidget interface

	virtual void OnEnabledTick(const FGeometry& MyGeometry, float InDeltaTime) {};

	virtual void OnBoundToGameplaySystem(UGameplaySystemComponent* GameplaySystem) {};
	
	// Only called when it's safe to update the GameplayEvent display.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Tick GameplayEvent Display"), Category = "DebugWidget")
	void K2_TickGameplayEventDisplay(float DeltaTime);

	// Only called when it's safe to update the GameplaySystem display.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Tick GameplaySystem Display"), Category = "DebugWidget")
	void K2_TickGameplaySystemDisplay(float DeltaTime);

	// Should not rely on the GameplaySystem being valid
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Tick Generic Display"), Category = "DebugWidget")
	void K2_TickGenericDisplay(float DeltaTime);

	// Removes stale information from the display in case a new GameplaySystem is never found and updates the UI to clear itself.
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Clear Display"), Category = "DebugWidget")
	void K2_ClearDisplay();

	// Cycle to a new page. 1-indexed.
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, meta = (DisplayName = "Cycle Menu"), Category = "DebugWidget")
	void K2_CycleMenu(int PageIndex);

	// Gets some basic information about the GameplaySystem and the owning Actor.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DebugWidget")
	FString GetGenericDisplayInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DebugWidget")
	FAttributeDisplayInfo GetAttributeDisplayInfo(TArray<FString>& OutArray) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DebugWidget")
	FString GetGameplayEffectsDisplayInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DebugWidget")
	FString GetLevelSystemDisplayInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DebugWidget")
	FString GetAvailableAbilitiesDisplayInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DebugWidget")
	FString GetActiveAbilitiesDisplayInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DebugWidget")
	FString GetGameplayTagSystemDisplayInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DebugWidget")
	FEventsDisplayInfo GetGlobalGameplayEventDisplayInfo() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DebugWidget")
	FEventsDisplayInfo GetActorGameplayEventDisplayInfo() const;

	// Draws lines on the current targets owning Actor to help identify the current Actor.
	UFUNCTION(BlueprintCallable, Category = "DebugWidget")
	void DrawDebugLines();

protected:

	// Sets up input bindings, and binds to the player's GameplaySystemComponent if it has one.
	void BindToPlayer();

	// Binds the widget to a new GameplaySystemComponent. Adds the component to the list of candidates if not already present.
	void BindToGameplaySystem(UGameplaySystemComponent* NewSystem);

	void BindToGameplayEventSubsystem();

	// Toggles the widgets visiblity between Hidden and Visible.
	void ToggleWidget();

	// Cycles through all GameplaySystemComponents in the Level.
	void CycleDebugTarget();

	// Cycles through the currently displayed page of the widget.
	void CycleMenu();

	UPROPERTY(BlueprintReadOnly, Category = "DebugWidget")
	bool bEnabled = false;

	UPROPERTY(EditDefaultsOnly, Category = "DebugWidget")
	int PageCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DebugWidget")
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DebugWidget")
	TSoftObjectPtr<UInputAction> ToggleMenuAction;

	uint32 ToggleMenuHandle = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DebugWidget")
	TSoftObjectPtr<UInputAction> CycleMenuAction;

	uint32 CycleMenuHandle = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DebugWidget")
	TSoftObjectPtr<UInputAction> CycleTargetAction;

	uint32 CycleTargetHandle = 0;

	int SystemCandidateIndex = 0;

	int DisplayedPage = 1;

	TWeakObjectPtr<UGameplaySystemComponent> BoundGameplaySystem = nullptr;
	
	TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> UsedInputSystem = nullptr;

	TWeakObjectPtr<UEnhancedInputComponent> UsedInputComponent = nullptr;

	TWeakObjectPtr<UGameplayEventSubsystem> BoundEventSubsystem = nullptr;
};
