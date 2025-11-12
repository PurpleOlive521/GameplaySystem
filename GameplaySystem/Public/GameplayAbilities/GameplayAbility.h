// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "GameplayTagContainer.h"

#include "GameplayAbility.generated.h"

class UGameplaySystemComponent;
class UGameplayAbility;
class UTexture2D;

UENUM(BlueprintType)
enum class EInstancingPolicy : uint8
{
	// The Ability has no lifetime, and performs all it's logic on ActivateAbility. EndAbility is not called for these abilities. No latent logic should be used!
	EIP_NoLifetime		UMETA(DisplayName = "Not Instanced"),

	// One instance is created per GameplaySystemComponent. This is the most performant option, but any state needs to be reset and designed for reusability.
	EIP_InstancedPerActor		UMETA(DisplayName = "Instanced Per Actor"),

	// A new instance is created for each execution. This is the most flexible but also expensive option, depending on the frequency of activation and the setup logic required (loading assets or heavy initialisation).
	EIP_InstancedPerExecution	UMETA(DisplayName = "Instanced Per Execution"),
};

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FActiveGameplayAbility
{
	GENERATED_BODY()

	FActiveGameplayAbility() {};

	// Sets up handle for generic Ability
	FActiveGameplayAbility(UGameplayAbility* BaseAbility);

	// Updates duration and cooldown
	void Tick(float DeltaTime);

	// Returns the abilities current cooldown
	float GetCurrentCooldown() const;

	// Returns the abilities elapsed duration
	float GetCurrentDuration() const;

	// Returns the remaining duration of the ability
	float GetRemainingDuration() const;

	// Returns false if both the duration and cooldown has elapsed.
	bool IsAbilityActive() const;

	// Cancels the ability, calling CancelAbility if the instance exists. EndAbility is not called after this.
	void CancelAbility();

	// Ends the ability, calling EndAbility if the instance exists.
	void EndAbility();

	// Resets the pointers, ensuring that no lingering references are left.
	void Reset();

	// Creates a display friendly string with information about the abilities state.
	FString ToString() const;
	
	// Creates a display friendly string with text tags for readability and information about the abilities state.
	FString ToStringWithDebugTags() const;

	// Pointer to the instance
	UPROPERTY(BlueprintReadOnly, Category = "ActiveGameplayAbility")
	TObjectPtr<UGameplayAbility> GameplayAbility = nullptr;

	// If Duration has passed
	UPROPERTY(BlueprintReadWrite, Category = "ActiveGameplayAbility")
	bool bHasDurationElapsed = false;

	// If Cooldown has passed
	UPROPERTY(BlueprintReadWrite, Category = "ActiveGameplayAbility")
	bool bHasCooldownElapsed = false;

	// Is true if the ability has been ended through either EndAbility or CancelAbility.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ActiveGameplayAbility")
	bool bHasBeenEnded = false;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ActiveGameplayAbility")
	float Duration = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ActiveGameplayAbility")
	float Cooldown = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ActiveGameplayAbility")
	float ElapsedTime = 0;

	// The tags applied to the Instance itself. Can be modified at runtime.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ActiveGameplayAbility")
	FGameplayTagContainer AbilityTags;
};

// Defines a ability that can be activated with a GameplaySystemComponent.
// Derived abilities can override various functions to implement custom activation and requirement logic:
// * CheckAbilityRequirements & ApplyAbilityRequirements
// 
// - The core life-cycle functions that define the ability's behavior.
// - Only CancelAbility is called if the ability is cancelled. EndAbility is only called when the ability ends naturally after it's duration has elapsed.
// * ActivateAbility, EndAbility & CancelAbility
// 
// - Only required if the ability allows ability queueing. Allows us to query if the cancellation of an ability would allow/disallow a upcoming ability's activation.
// * ApplyAbilityEndedModifiers & RemoveAbilityEndedModifiers
UCLASS(Blueprintable)
class GAMEPLAYSYSTEM_API UGameplayAbility : public UObject
{
	GENERATED_BODY()

public:
	UGameplayAbility();

	// --- Begin UObject 

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual UWorld* GetWorld() const override;

	virtual void FinishDestroy() override;

	// --- End UObject

	void Init(AActor* OwningActor, UGameplaySystemComponent* OwningComponent);

	// Returns the Actor that owns this Ability. Can return nullptr.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	AActor* GetOwningActor() const;

	// Ensured to be valid
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	AActor* GetOwningActor_Checked() const;

	// Returns the GameplaySystemComponent that owns this Ability. Can return nullptr.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	UGameplaySystemComponent* GetOwningComponent() const;

	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	UGameplaySystemComponent* GetOwningComponent_Checked() const;


	// Tries to activate the Ability. Can fail due to active cooldowns or lack of resources required for the ability activation.
	// Returns true if activated, otherwise returns false.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool AttemptActivateAbility(FActiveGameplayAbility& OutGameplayAbilityHandle);

	// Checks if the abilities activation requirements are fullfilled, without applying any costs or starting cooldowns.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameplayAbility")
	bool CheckAbilityRequirements() const;
	virtual bool CheckAbilityRequirements_Implementation() const;

	// Applies all the requirements for the ability to be activated. This can be through modifying attributes, GameplayTags, GameplayEffects or similar.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameplayAbility")
	bool ApplyAbilityRequirements();
	virtual bool ApplyAbilityRequirements_Implementation();

	// Does the boilerplate work of setting up the abilities active state and handle. Always called before ActivateAbility.
	// Should generally not need to be overridden.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameplayAbility")
	void PreActivateAbility(FActiveGameplayAbility& OutActiveGameplayAbility);
	virtual void PreActivateAbility_Implementation(FActiveGameplayAbility& OutActiveGameplayAbility);

	// The function called when a ability is activated. Override this behaviour to build the
	// activation logic of your ability.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameplayAbility")
	void ActivateAbility(FActiveGameplayAbility& OutActiveGameplayAbility);
	virtual void ActivateAbility_Implementation(FActiveGameplayAbility& OutActiveGameplayAbility);

	// Called when when the abilites set duration has ran out. Is not called in favor of CancelAbility if the ability is forcibly ended early.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameplayAbility")
	void EndAbility();
	virtual void EndAbility_Implementation();

	// Called when the ability is forced to end early. EndAbility is not called after this, so it will end up sharing work that EndAbility would normally do.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameplayAbility")
	void CancelAbility();
	virtual void CancelAbility_Implementation();
	
	// Applies the AbilityEndedModifiers if they have not yet been applied. Returns true if applied, false if already applied.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool TryApplyAbilityEndedModifiers();

	// Applies any modifiers that might affect the activation of other abilities. Always called when the ability ends or is cancelled.
	// Do not call directly, instead call TryApplyAbilityEndedModifiers!
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameplayAbility")
	void ApplyAbilityEndedModifiers();
	virtual void ApplyAbilityEndedModifiers_Implementation();

	// Removes the AbilityEndedModifiers if they have not yet been removed. Returns true if removed, false if already removed or not yet applied.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool TryRemoveAbilityEndedModifiers();

	// Removes any modifiers applied in ApplyAbilityEndedModifiers.
	// Do not call directly, instead call TryRemoveAbilityEndedModifiers!
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "GameplayAbility")
	void RemoveAbilityEndedModifiers();
	virtual void RemoveAbilityEndedModifiers_Implementation();
	
	// Supplies the AbilityHandle with initial values. Supply with more in your derived ActivateAbility if needed.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	void SetupAbilityHandle(FActiveGameplayAbility& OutActiveGameplayAbility);

	// Returns the cooldown in seconds for this Ability.
	UFUNCTION(BlueprintCallable, BlueprintPure ,Category = "GameplayAbility")
	float GetCooldown() const;

	// Returns the duration in seconds for this Ability.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	float GetDuration() const;

	// Returns true if the Ability allows itself to be cancelled.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	bool IsCancellable() const;

	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void SetIsCancellable(bool bInIsCancellable);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	const FGameplayTagContainer& GetAbilityTags() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	EInstancingPolicy GetInstancingPolicy() const;

	// Any derived class should override this to provide relevant information as part of this string
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	virtual FString ToString() const;

	// Any derived class should override this to provide relevant information with text tags for readability as part of this string
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	virtual FString ToStringWithDebugTags() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	FString GetDisplayName() const;

	// Tags that block this Ability from activating if present in the activating GameplaySystemComponent.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameplayAbility|Tags")
	FGameplayTagContainer ActivationBlockedTags;

	// Any active Abilities with these tags in the activating GameplaySystemComponent are cancelled when this Ability is activated.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameplayAbility|Tags")
	FGameplayTagContainer CancelAbilitiesWithTag;

	// Will forcibly cancel any matching abilities if true, disregarding whether the ability currently allows cancelling or not.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameplayAbility|Tags")
	bool bIsAuthoritativeCancel = false;


	// Any Abilities with these tags are blocked from activating in the activating GameplaySystemComponent.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameplayAbility|Tags")
	FGameplayTagContainer BlockAbilitiesWithTag;

protected:

	// Returns a readable name for this ability. 
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameplayAbility")
	FString DisplayName = "Not set.";

	// How long the ability is active until it's temporary effects are reverted.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayAbility|Costs")
	float Duration = 0;

	// Starts counting on ability activation. Measured in seconds.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (EditCondition = "bSameDurationAndCooldown == false"), Category = "GameplayAbility|Costs")
	float Cooldown = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayAbility|Costs")
	bool bSameDurationAndCooldown = false;

	// Does the ability permit being cancelled? Can still be overwritten by an authoritative cancel request.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayAbility")
	bool bIsCancellable = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayAbility")
	EInstancingPolicy InstancingPolicy = EInstancingPolicy::EIP_InstancedPerActor;

	// The tags applied to the Ability itself and any instances created from it.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayAbility")
	FGameplayTagContainer AbilityTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayAbility|UI")
	TObjectPtr<UTexture2D> ActiveIcon;

	UPROPERTY(BlueprintReadWrite, Category = "GameplayAbility")
	bool bHasAppliedAbilityEndedModifiers = false;

private:

	TWeakObjectPtr<AActor> OwningActor;

	TWeakObjectPtr<UGameplaySystemComponent> OwningComponent;

	// Allows us to tweak some properties by hand after in-editor changes
	void CurateProperties();
};
