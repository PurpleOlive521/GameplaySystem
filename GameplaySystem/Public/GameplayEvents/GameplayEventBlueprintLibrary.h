// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayEvent.h"
#include "GameplayEventHandle.h"
#include "NiagaraFunctionLibrary.h"
#include "GameplayEventBlueprintLibrary.generated.h"

class UGameplayEventSubsystem;

/**
 * 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UGameplayEventBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	// Entrypoint for activating any GameplayEvent. 
	// Returns a valid GameplayEventHandle if activated successfully.
	UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = "Owner"), Category = "GameplayEvent")
	static FGameplayEventHandle TriggerEvent(TSubclassOf<UGameplayEvent> EventClass, UObject* Owner);

	// Entrypoint for activating any GameplayEvent with additional ActivationData.
	// Returns a valid GameplayEventHandle if activated successfully.
	UFUNCTION(BlueprintCallable, meta = (DefaultToSelf = "Owner", DisplayName = "Trigger Event with Activation Data"), Category = "GameplayEvent")
	static FGameplayEventHandle TriggerEvent_ActivationData(TSubclassOf<UGameplayEvent> EventClass, UObject* Owner, const FGameplayEventActivationData& ActivationData);

	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldObject", DefaultToSelf = "WorldObject"), Category = "GameplayEvent")
	static bool AbortEvent(const UObject* WorldObject, const FGameplayEventHandle& Handle);

	// Can return nullptr if the Handle is invalid.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldObject", DefaultToSelf = "WorldObject"), Category = "GameplayEvent")
	static UGameplayEvent* GetEventFromHandle(const UObject* WorldObject, const FGameplayEventHandle& Handle);

	// Each index maps 1:1 between Handles and OutEvents. Any indices can be nullptr if the corresponding handle is invalid.
	UFUNCTION(BlueprintCallable, meta = (HidePin = "WorldObject", DefaultToSelf = "WorldObject"), Category = "GameplayEvent")
	static void GetEventsFromHandles(const UObject* WorldObject, const TArray<FGameplayEventHandle>& Handles, TArray< UGameplayEvent*> OutEvents);

	// Returns true if the Handle is valid, otherwise false.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayEvent")
	static bool IsHandleValid(const FGameplayEventHandle& Handle);

	// Returns true if the Handle's event is Active and valid, otherwise false.
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (HidePin = "WorldObject", DefaultToSelf = "WorldObject"), Category = "GameplayEvent")
	static bool IsEventActive(const FGameplayEventHandle& Handle, UObject* WorldObject);

	// Converts the TickSource type to a display friendly string. Works in shipping builds!
	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "GameplayEvent")
	static FString ConvertTickSourceToDisplayName(ETickSource TickSourceType);

	// --- Helpers

	// Same as UGameplayStatic::GetPlayerController but with exposed WorldContextObject.
	UFUNCTION(BlueprintPure, Category = "GameplayEvent|Helpers")
	static APlayerController* GetPlayerControllerForEvent(const UObject* WorldContextObject, int32 PlayerIndex);

	// Same as UGameplayStatic::GetGameInstance but with exposed WorldContextObject.
	UFUNCTION(BlueprintPure, Category = "GameplayEvent|Helpers")
	static UGameInstance* GetGameInstanceForEvent(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "GameplayEvent|Helpers")
	static UNiagaraComponent* SpawnSystemAtLocation(const UObject* WorldContextObject, class UNiagaraSystem* SystemTemplate, FVector Location, FRotator Rotation = FRotator::ZeroRotator, FVector Scale = FVector(1.f), bool bAutoDestroy = true, bool bAutoActivate = true, ENCPoolMethod PoolingMethod = ENCPoolMethod::None, bool bPreCullCheck = true);

private: 
	
	// Asserts if not valid.
	static inline UGameplayEventSubsystem* GetGameplayEventSubsystem(const UObject* WorldObject);
};
