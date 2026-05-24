// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "GameplayTagContainer.h"
#include "GameplaySystemTypes.h"
#include "GameplayAbilityHandle.h"
#include "GameplayTaskOwnerInterface.h"

#include "GameplayAbility.generated.h"

class UGameplaySystemComponent;
class UGameplayAbility;
class UGameplayAbilityTask;
class UGameplayEffect;

DECLARE_LOG_CATEGORY_EXTERN(LogGameplayAbility, Log, All)

// Macro for logging in the LogGameplayAbility category
#define GA_LOG(Verbosity, Format, ...)								\
{																	\
	UE_LOG(LogGameplayAbility, Verbosity, Format, ##__VA_ARGS__);	\
}

// Invoked when the gameplay ability ended
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilityEndedSignature, UGameplayAbility* /* Ending Ability */);

// Invoked when the gameplay ability is cancelled
DECLARE_MULTICAST_DELEGATE(FOnAbilityCancelledSignature);

// Invoked when the ability is either cancelled or ended, whichever one happens first.
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilityFinishedSignature, UGameplayAbility* /* Finished Ability */);

// Used to notify ability state tasks that a state is being ended
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAbilityStateEndedSignature, FName /* Ending State */);

UENUM(BlueprintType)
enum class EInstancingPolicy : uint8
{
	// The Ability has no lifetime, and performs all it's logic on ActivateAbility. EndAbility is not called for these abilities. No latent logic should be used!
	EIP_NoLifetime				UMETA(DisplayName = "Not Instanced"),

	// One instance is created per GameplaySystemComponent. This is the most performant option, but any state needs to be reset and designed for reusability.
	EIP_InstancedPerActor		UMETA(DisplayName = "Instanced Per Actor"),

	// A new instance is created for each execution. This is the most flexible but also expensive option, depending on the frequency of activation and the setup logic required (loading assets or heavy initialisation).
	EIP_InstancedPerExecution	UMETA(DisplayName = "Instanced Per Execution"),
};

namespace GameplayAbilityConstants
{
	constexpr float NO_COOLDOWN = 0.0f;
}

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FActiveGameplayAbility
{
	GENERATED_BODY()

	FActiveGameplayAbility() = default;

	// Sets up handle for generic Ability
	FActiveGameplayAbility(UGameplayAbility* BaseAbility, FGameplayAbilityHandle SourceHandle);

	// Updates duration and cooldown
	void Tick(float DeltaTime, UGameplaySystemComponent* GameplaySystem);

	// Returns the remaining cooldown in seconds.
	float GetRemainingCooldown() const;

	// Returns the remaining cooldown as a percentage with 1 (100%) being the entire cooldown and 0 (0%) being none remaining.
	float GetRemainingCooldownAsPercentage() const;

	void SetCooldown(float Value);

	// Returns true if the ability is currently active, e.g. not ended or cancelled.
	bool IsAbilityActive() const;

	// Returns true if the ability has active state either through active duration or cooldown.
	bool HasActiveState() const;

	// Returns true if the ability is finished, either through duration elapsing or being cancelled.
	bool ShouldBeRemoved() const;

	bool IsValid() const;

	// Returns true if the Ability has an active cooldown.
	bool HasCooldown() const;

	// Creates a display friendly string with information about the abilities state.
	FString ToString() const;
	
	// Creates a display friendly string with text tags for readability and information about the abilities state.
	FString ToStringWithDebugTags() const;

	UPROPERTY(BlueprintReadOnly, Category = "ActiveGameplayAbility")
	FGameplayAbilityHandle Handle;

	UPROPERTY(BlueprintReadOnly, Category = "ActiveGameplayAbility")
	UGameplayAbility* Ability = nullptr;

	// Don't modify directly, use SetCooldown
	UPROPERTY(BlueprintReadOnly, Category = "ActiveGameplayAbility")
	float Cooldown = 0;

	UPROPERTY(BlueprintReadOnly, Category = "ActiveGameplayAbility")
	float ElapsedTime = 0;

	uint32 bHasCooldownElapsed : 1 = false;
	
	uint32 bIsValid : 1 = false;

	uint32 bHasActivated : 1 = false;

	// Tags applied to the Instance itself. Can be modified at runtime.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ActiveGameplayAbility")
	FGameplayTagContainer AbilityTags;

	bool operator==(const FActiveGameplayAbility& Other) const
	{
		return Handle == Other.Handle;
	}

	bool operator!=(const FActiveGameplayAbility& Other) const
	{
		return Handle != Other.Handle;
	}
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
class GAMEPLAYSYSTEM_API UGameplayAbility : public UObject, public IGameplayTaskOwnerInterface
{
	GENERATED_BODY()

	friend struct FActiveGameplayAbility;

public:
	UGameplayAbility();

	// --- Begin UObject 

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual UWorld* GetWorld() const override;

	virtual void FinishDestroy() override;

	// --- End UObject

	// --- Begin IGameplayTaskOwnerInterface
	virtual UGameplayTasksComponent* GetGameplayTasksComponent(const UGameplayTask& Task) const override;
	virtual AActor* GetGameplayTaskOwner(const UGameplayTask* Task) const override;
	virtual AActor* GetGameplayTaskAvatar(const UGameplayTask* Task) const override;
	virtual void OnGameplayTaskInitialized(UGameplayTask& Task) override;
	virtual void OnGameplayTaskActivated(UGameplayTask& Task) override;
	virtual void OnGameplayTaskDeactivated(UGameplayTask& Task) override;
	// --- End IGameplayTaskOwnerInterface

	void Init(AActor* OwningActor, UGameplaySystemComponent* OwningComponent);

	// Returns the Actor that owns this Ability. Can return nullptr.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	AActor* GetOwningActor() const;

	// Ensured to be valid
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	AActor* GetOwningActor_Checked() const;

	// Returns the GameplaySystemComponent that owns this Ability. Can return nullptr.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	UGameplaySystemComponent* GetOwningComponent() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	UGameplaySystemComponent* GetOwningComponent_Checked() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	[[nodiscard]] FGameplayAbilityHandle GetAbilityHandle() const;

	// Tries to activate the Ability. Can fail due to active cooldowns or lack of resources required for the ability activation.
	// Returns true if activated, otherwise returns false.
	bool TryCommitActivateAbility(const FGameplayAbilityActivationData& ActivationData, FActiveGameplayAbility& ActiveGameplayAbility);

	// Checks if the abilities activation requirements are fullfilled, without applying any costs or starting cooldowns.
	bool TryCheckAbilityRequirements(const FGameplayAbilityActivationData& ActivationData) const;

	// Applies all the requirements for the ability to be activated. This can be through modifying attributes, GameplayTags, GameplayEffects or similar.
	bool TryApplyAbilityRequirements(const FGameplayAbilityActivationData& ActivationData);

	void TryActivateAbility(const FGameplayAbilityActivationData& ActivationData, FActiveGameplayAbility& OutActiveGameplayAbility);

	// Ends the Ability.
	// Returns true if ended, false otherwise.
	bool TryEndAbility();

	// Cancels the Ability. Does not call EndAbility.
	// Returns true if cancelled, false otherwise.
	bool TryCancelAbility(bool bIsAuthoritative);
	
	// Applies the AbilityEndedModifiers if they have not yet been applied. Returns true if applied, false otherwise.
	UFUNCTION(BlueprintCallable)
	bool TryApplyAbilityEndedModifiers();

	// Removes the AbilityEndedModifiers if they have not yet been removed. Returns true if removed, false otherwise.
	UFUNCTION(BlueprintCallable)
	bool TryRemoveAbilityEndedModifiers();

	// Inform the ability that a task is waiting for the player's avatar
	virtual void NotifyAbilityTaskWaitingOnAvatar(UGameplayAbilityTask* AbilityTask);

	// Ends any active ability state task with the given name. If name is 'None' all active states will be ended.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void EndAbilityState(FName StateToEnd);

	// Meant for calling by the Ability itself.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void EndAbilityFromSelf();

	// Meant for calling by the Ability itself.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void CancelAbilityFromSelf(bool bIsAuthorative);

	// Returns the cooldown in seconds for this Ability.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	float GetCooldown() const;

	// Returns true if the Ability allows itself to be cancelled.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	bool IsCancellable() const;

	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void SetIsCancellable(bool bInIsCancellable);

	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool IsActive() const;

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

	// Multiply with the DeltaTime to get the relative DeltaTime of the Ability's owning Actor. 
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	float GetDeltaTimeCoefficient() const;

	FGameplaySystemActorInfo* GetCurrentActorInfo() const;

	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void SendAbilityNotify(FName Notify);

	// The tags applied to the Ability itself and any instances created from it.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayAbility|GameplayTags")
	FGameplayTagContainer AbilityTags;

	// Tags that block this Ability from activating if present in the activating GameplaySystemComponent.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameplayAbility|GameplayTags")
	FGameplayTagContainer ActivationBlockedTags;

	// Any active Abilities with these tags in the activating GameplaySystemComponent are cancelled when this Ability is activated.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameplayAbility|GameplayTags")
	FGameplayTagContainer CancelAbilitiesWithTag;

	// Will forcibly cancel any matching abilities if true, disregarding whether the ability currently allows cancelling or not.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameplayAbility|GameplayTags")
	bool bIsAuthoritativeCancel = false;

	// Any Abilities with these tags are blocked from activating in the activating GameplaySystemComponent.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GameplayAbility|GameplayTags")
	FGameplayTagContainer BlockAbilitiesWithTag;

protected:

	virtual bool CheckAbilityRequirements(const FGameplayAbilityActivationData& ActivationData) const;

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Blueprint Check Ability Requirements"), Category = "GameplayAbility")
	bool K2_CheckAbilityRequirements(const FGameplayAbilityActivationData& ActivationData) const;

	virtual bool ApplyAbilityRequirements(const FGameplayAbilityActivationData& ActivationData);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Blueprint Apply Ability Requirements"), Category = "GameplayAbility")
	bool K2_ApplyAbilityRequirements(const FGameplayAbilityActivationData & ActivationData);

	virtual void ActivateAbility(const FGameplayAbilityActivationData& ActivationData, FActiveGameplayAbility& OutActiveGameplayAbility);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Blueprint Activate Ability"), Category = "GameplayAbility")
	void K2_ActivateAbility(const FGameplayAbilityActivationData& ActivationData, FActiveGameplayAbility OutActiveGameplayAbility);

	virtual void EndAbility();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Blueprint End Ability"), Category = "GameplayAbility")
	void K2_EndAbility();

	virtual void CancelAbility();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Blueprint Cancel Ability"), Category = "GameplayAbility")
	void K2_CancelAbility();

	virtual void ApplyAbilityEndedModifiers();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Blueprint Apply Ability Ended Modifiers"), Category = "GameplayAbility")
	void K2_ApplyAbilityEndedModifiers();

	virtual void RemoveAbilityEndedModifiers();

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Blueprint Remove Ability Ended Modifiers"), Category = "GameplayAbility")
	void K2_RemoveAbilityEndedModifiers();

	virtual void ReceiveAbilityNotify(FName Notify);

	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "Blueprint Receive Ability Notify"), Category = "GameplayAbility")
	void K2_ReceiveAbilityNotify(FName Notify);


	// Returns true if we are a static instance that should not be modified or have state.
	bool IsStaticInstance() const;

	// Returns true if we are the ability currently animating an AnimMontage in the owning GameplaySystem.
	bool IsAnimatingAbility() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayAbility")
	FString DisplayName = "Not set.";

	// Starts counting on ability activation. Measured in seconds.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayAbility|Costs")
	float Cooldown = 0.0f;

	// The cooldown will be removed when the Ability is cancelled. 
	// Use to signify that the cooldown is only important when the Ability is active, and holds no purpose after it's cancelled.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayAbility|Costs")
	bool bRemoveCooldownWhenCancelled = false;

	// Does the ability permit being cancelled? Can still be ignored by an authoritative cancel request.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayAbility")
	bool bIsCancellable = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayAbility")
	EInstancingPolicy InstancingPolicy = EInstancingPolicy::EIP_InstancedPerActor;

	// Applied when the ability is either cancelled or ended.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayAbility|GameplayEffects")
	TArray<TSubclassOf<UGameplayEffect>> AbilityFinishedEffects;

	// Applied when the ability is activated
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GameplayAbility|GameplayEffects")
	TArray<TSubclassOf<UGameplayEffect>> AbilityActivatedEffects;

	UPROPERTY()
	TArray<TObjectPtr<UGameplayTask>> ActiveTasks;

private:

	bool StopAbility(bool bIsCancelled);

	TWeakObjectPtr<AActor> OwningActor;

	TWeakObjectPtr<UGameplaySystemComponent> OwningComponent;

	uint32 bHasAppliedAbilityEndedModifiers : 1 = false;

	uint32 bIsActive : 1 = false;

	uint32 bHasEnded : 1 = false;

	uint32 bHasCancelled : 1 = false;

	uint32 bHasBlueprintCheckAbilityRequirements : 1 = false;

	uint32 bHasBlueprintApplyAbilityRequirements : 1 = false;

public:
	// --- Delegates

	FOnAbilityEndedSignature OnAbilityEndedDelegate;

	FOnAbilityCancelledSignature OnAbilityCancelledDelegate;

	FOnAbilityFinishedSignature OnAbilityFinishedDelegate;

	FOnAbilityStateEndedSignature OnAbilityStateEndedDelegate;
};
