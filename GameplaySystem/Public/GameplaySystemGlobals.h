// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplaySystemComponent.h"
#include "GameplaySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class GAMEPLAYSYSTEM_API UGlobalGameplaySubsystem : public UTickableWorldSubsystem
{

	GENERATED_BODY()

public:
	UGlobalGameplaySubsystem() = default;

	// --- Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// --- End USubsystem Interface

	// --- Begin FTickableObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	virtual bool IsTickableWhenPaused() const override;
	// --- End FTickableObject Interface

	// Helper getter
	static UGlobalGameplaySubsystem* Get(const UObject* WorldContext);

	UFUNCTION(BlueprintCallable, Category = "GameplaySystem|Globals")
	UGameplaySystemComponent* GetGlobalGameplaySystemComponent();

protected:

	UPROPERTY(Transient)
	mutable TObjectPtr<UGameplaySystemComponent> GlobalGameplaySystem;
};
