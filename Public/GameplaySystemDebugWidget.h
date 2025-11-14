// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.


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

/**
 * --- Widget that displays critical system information for easier debugging. 
 * Defaults to the players GameplaySystemComponent, but can be used to cycle through all found GameplaySystemComponent in the level.
 * 
 * For a entitity's GameplaySystemComponent to be found, it must implement the IGameplaySystemOwnerInterface.
 */
UCLASS(Blueprintable)
class GAMEPLAYSYSTEM_API UGameplaySystemDebugWidget : public UUserWidget
{
	GENERATED_BODY()

	// We rely on statics to keep the candidates independent of any one widget instance, so that we do not need to keep track of the widgets lifetime between activation.

	// TODO: Make sure that cleaning the array wont mess with other active widgets and their indices.
	static void CleanSystemCandidates();
	
	static TArray<TWeakObjectPtr<UGameplaySystemComponent>> ActiveSystemCandidates;

public:

	// --- Begin UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// --- End UUUserWidget interface

	// Ensures that the UI state is valid when called and that a GameplaySystem to display exists
	UFUNCTION(BlueprintImplementableEvent, Category = "DebugWidget")
	void TickGameplaySystemDisplay(float DeltaTime);

	// Should not rely on the GameplaySystem being valid
	UFUNCTION(BlueprintImplementableEvent, Category = "DebugWidget")
	void TickGenericDisplay(float DeltaTime);

	// Removes stale information from the display in case a new GameplaySystem is never found and updates the UI to clear itself.
	UFUNCTION(BlueprintImplementableEvent, Category = "DebugWidget")
	void ClearDisplay();

	// Gets some basic information about the GameplaySystem and the owning Actor.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DebugWidget")
	FString GetGenericDisplayInfo() const;

	// Index 0 contains the Attribute types, index 1 contains the Attribute values.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "DebugWidget")
	void GetAttributeDisplayInfo(TArray<FString>& OutArray) const;

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

	// Draws lines on the current targets owning Actor to help identify the current Actor.
	UFUNCTION(BlueprintCallable, Category = "DebugWidget")
	void DrawDebugLines();

protected:

	UPROPERTY(BlueprintReadOnly, Category = "DebugWidget")
	bool bEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DebugWidget")
	TSoftObjectPtr<UInputMappingContext> InputMapping;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DebugWidget")
	TSoftObjectPtr<UInputAction> ToggleMenuAction;

	uint32 ToggleMenuHandle = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "DebugWidget")
	TSoftObjectPtr<UInputAction> CycleTargetAction;

	uint32 CycleTargetHandle = 0;

private:

	// Sets up input bindings, and binds to the player's GameplaySystemComponent if it has one.
	void BindToPlayer();

	// Binds the widget to a new GameplaySystemComponent. Adds the component to the list of candidates if not already present.
	void BindToGameplaySystem(UGameplaySystemComponent* NewSystem);

	// Toggles the widgets visiblity between Hidden and Visible.
	void ToggleWidget();

	// Cycles through all GameplaySystemComponents in the Level.
	void CycleDebugTarget();

	int SystemCandidateIndex = 0;

	TWeakObjectPtr<UGameplaySystemComponent> BoundGameplaySystem;
	
	TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> UsedInputSystem;

	TWeakObjectPtr<UEnhancedInputComponent> UsedInputComponent;
};
