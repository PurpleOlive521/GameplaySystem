// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTasksComponent.h"

#include "AttributeEffect.h"
#include "GameplayEffect.h"
#include "GameplayAbility.h"
#include "GameplayAbilityHandle.h"
#include "GameplayTagSystem.h"
#include "GameplayTagOwnerInterface.h"
#include "GameplaySystemTypes.h"
#include "functional"
#include "Animation/AnimInstance.h"
#include "SaveableObjectInterface.h"

#include "GameplaySystemComponent.generated.h"

struct FGameplaySystemSaveObject;
class UGameplaySystemDeveloperSettings;

constexpr UGameplayAbility* GET_ABILITY_NO_IGNORE = nullptr;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, EAttributeType /* ChangedAttribute*/);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnyAttributeChangedSignature, EAttributeType, AttributeType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLeveledUpSignature, int, PreviousLevel, int, CurrentLevel, float, NextLevelExp);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnyAbilityEndedSignature, const FGameplayAbilityHandle&, Handle);


// A collection of delegates that are accessed and managed through a single entry-point. 
// The delegates are created on-demand, and removed when no longer bound to.
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FDelegateCollection
{
	GENERATED_BODY()

	FDelegateCollection() = default;

	TMap<EAttributeType, FOnAttributeChangedSignature> DelegateMap;

	// Returns the associated delegate, or creates one if one doesn't exist. Only use to bind to delegates, see FDelegateCollection::Broadcast for broadcasting!
	FOnAttributeChangedSignature& GetDelegate(EAttributeType Attribute);

	// Returns the associated delegates, or creates ones if they don't exist. Only use to bind to delegates, see FDelegateCollection::BroadcastMultiple for broadcasting!
	void GetMultipleDelegates(const TArray<EAttributeType>& Attributes, TArray<FOnAttributeChangedSignature*>& OutDelegates);

	// Returns true if a delegate is present for the AttributeType
	inline bool HasDelegate(EAttributeType Attribute);

	// Broadcasts the AttributeType delegate if it exists.
	inline void Broadcast(EAttributeType Attribute);

	// Broadcasts the AttributeType delegates if they exists.
	inline void BroadcastMultiple(const TArray<EAttributeType>& Attributes);

	inline void BroadcastAll();

	// Broadcasts are queued while the lock is active, and performed once the lock is removed
	struct FBroadcastLock
	{
		FDelegateCollection& Collection;

		FBroadcastLock(FDelegateCollection& InCollection) : Collection(InCollection)
		{
			Collection.BroadcastLockCount++;
		}

		~FBroadcastLock()
		{
			Collection.BroadcastLockCount--;

			if (Collection.BroadcastLockCount <= 0)
			{
				Collection.BroadcastQueue();
			}
		}
	};

	[[nodiscard]] inline FBroadcastLock CreateBroadcastLock()
	{
		return FBroadcastLock(*this);
	}

	// If true, won't broadcast directly requested or queued delegates.
	uint32 bIsSilenced : 1 = false;

private:

	// Cleans up any delegates that are no longer bound to.
	void CheckDelegates();

	inline void EnqueueBroadcast(EAttributeType Attribute);

	inline void EnqueueBroadcasts(const TArray<EAttributeType>& Attributes);

	void BroadcastQueue();

	uint32 bIsDirty : 1 = false;

	uint32 BroadcastLockCount = 0;

	TArray<EAttributeType> QueuedBroadcasts;
};

USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FPlayMontageParams
{
	GENERATED_BODY()

	FPlayMontageParams() = default;

	UPROPERTY(BlueprintReadWrite, EditAnywhere);
	float PlayRate = 1.0f;

	UPROPERTY(BlueprintReadWrite, EditAnywhere);
	EMontagePlayReturnType PlayReturnType = EMontagePlayReturnType::Duration;

	// Starts the Montage at the beginning of this section.
	UPROPERTY(BlueprintReadWrite, EditAnywhere);
	FName StartSection;

	// Starts the Montage at the end of the section instead.
	UPROPERTY(BlueprintReadWrite, EditAnywhere);
	bool bUseEndOfSection = false;

	FOnMontageBlendingOutStarted MontageBlendOutDelegate = {};

	FOnMontageEnded MontageEndedDelegate = {};
};

// --- Component that handles Attributes, Level & Exp, GameplayEffects, Abilities and GameplayTags.
// Can be added to any Actor, but is primarily designed for Pawns and Characters.
// 
// - Attributes
// Attributes are managed stats that can be modified with GameplayEffects or AttributeEffects, and can scale based on a supplied CurveTable.
// They are recalculated only when modified, consisting of a BaseValue which is unaffected by temporary modifiers, or Value which is with them applied.
// 
// - GameplayEffects
// Contained modifiers that can be applied to a GameplaySystem. Durations and periodical applications of the effects are supported, and the
// applied modifiers are undone on removal. Keeps track of it's own made modifiers for an accurate removal. 
// Fit for buffs or temporary markers for states and durations.
// * GameplayEffectHandle is used to refer to a specific applied instance, since the GameplayEffects are not instantiated on use.
//
// - Abilities
// Abilities are their own bundles of logic with specified activation-requirements, and provide an API to manage state and life-cycle events such as Activation, Cancellation, and Ending.
// * ActiveGameplayAbility are the handles for activated abilities, keeping track of duration and calling the necessary life-cycle events.
// * Are instanced on use according to their InstancingPolicy. Allows for reusable abilities, or per-activation created instances for flexibility and performance.

UCLASS(Blueprintable, ClassGroup = GameplaySystem, hidecategories = (Object, LOD, Lighting, Transform, Sockets, TextureStreaming), meta=(BlueprintSpawnableComponent) )
class GAMEPLAYSYSTEM_API UGameplaySystemComponent : public UGameplayTasksComponent, public IGameplayTagOwnerInterface, public ISaveableObjectInterface
{
	GENERATED_BODY()

	friend class UGameplayEffect;
	friend class UGameplayEffectExecutor;
	friend struct FActiveGameplayAbility;
	friend class UGameplayAbility;
	friend class UGameplaySystemDebugWidget;

public:	
	UGameplaySystemComponent(const FObjectInitializer& ObjectInitializer);

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// --- Begin GameplayTagOwnerInterface
	void AddTag(const FGameplayTag& TagToAdd) override;
	void RemoveTag(const FGameplayTag& TagToRemove) override;
	void ClearTag(const FGameplayTag& TagToClear) override;
	void AppendTags(FGameplayTagContainer const& Other) override;
	bool HasTag(const FGameplayTag& TagToCheck) override;
	bool HasAllTags(const FGameplayTagContainer& TagsToCheckAgainst) override;
	int GetTagCount(const FGameplayTag& TagToCheck) override;
	int GetTotalTagCount() override;
	// --- End GameplayTagOwnerInterface

	// --- Begin GameplayTasksComponent Interface
	// TODO: Look at emulating the same logic as the GameplayTasksComponent, where we disable ticking when we have nothing to tick, and enable it when tickables are added?
	virtual bool GetShouldTick() const override;
	// --- End GameplayTasksComponent Interface

	// --- Begin SaveableObject Interface
	virtual bool OnSerialize(FSaveGameArchive& Archive, bool bIsLoading) override;
	// --- Begin SaveableObject Interface

	// --- Helpers

	// Tries to get the GameplaySystemComponent from this Actor.
	// Will first try through the GameplaySystemOwnerInterface, otherwise will resort to a component search.
	UFUNCTION(BlueprintCallable)
	static UGameplaySystemComponent* GetGameplaySystemFromActor(AActor* Actor);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SaveSystem")
	FGameplaySystemSaveObject Save() const;

	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void Load(const FGameplaySystemSaveObject& GameplaySystemSaveData);



	// --- Attributes

	// Get the value of an attribute.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	float GetAttributeValue(EAttributeType AttributeType, EAttributeValue TargetValue);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void ModifyAttributeValue(EAttributeType AttributeType, EAttributeValue TargetValue, float ValueChange);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeValue(EAttributeType AttributeType, EAttributeValue TargetValue, float NewValue);

	// For constant clamping of values, see FAttributeConfiguration.
	void ClampAttributeValue(EAttributeType AttributeType, EAttributeValue TargetValue, float Min, float Max);

	// Returns true if it contains the AttributeType
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	bool HasAttributeType(EAttributeType AttributeType);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeDataSet(UAttributeDataSet* InAttributeDataSet);

	// Gets all the Attributes in this Component
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	void GetAttributes(TMap<EAttributeType, FAttribute>& AttributesOut) const;

	// Overwrites any present Attributes with AttributesIn
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributes(const TMap<EAttributeType, FAttribute>& AttributesIn);

	TMap<EAttributeType, FAttribute>::TConstIterator GetConstAttributeIterator() const;

	// Add an attribute to the component. Will overwrite any existing attribute with the same type!
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void AddAttribute(FAttribute Attribute);

	// Simulate what the attributes values would be with AttributeEffectsToSimulate applied. Generates a TMap containing all attributes, changed or not.
	// The AttributeEffectsToSimulate can be queried to see individual contributions and modifications.
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SimulateAttributes(UPARAM(ref) TArray<FAttributeEffect>& AttributeEffectsToSimulate, TMap<EAttributeType, FAttribute>& GeneratedAttributesOut);

	// Returns the value of the CoefficientAttribute in this GameplaySystem, or 0.0f if the attribute is not present.
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	float CalculateCoefficientAttribute(const FCoefficientAttribute& CoefficientAttr);

	// Forcibly reevaluates the attributes.
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void ForceEvaluateAttributes();



	// --- AttributeEffects

	void ApplyAttributeEffect(FAttributeEffect& EffectToApply, EDurationType Type);

	// Removes the first instance of the AttributeEffect.
	void RemoveAttributeEffect(FAttributeEffect& EffectToRemove);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AttributeEffects")
	int GetActiveEffectsCount() const;
	


	// --- Level System

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes|LevelSystem")
	int GetEntityLevel() const;

	// Perform silently to not broadcast change, useful for loading
	UFUNCTION(BlueprintCallable, Category = "Attributes|LevelSystem")
	void SetEntityLevel(const int& Level, bool bDoSilently = true);

	// Does level-up logic automatically.
	UFUNCTION(BlueprintCallable, Category = "Attributes|LevelSystem")
	void AddExperience(float Experience);

	// Prone to clamping.
	UFUNCTION(BlueprintCallable, Category = "Attributes|LevelSystem")
	void SetExperience(float Experience);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes|LevelSystem")
	float GetEntityExperience() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes|LevelSystem")
	float GetRequiredExperienceForNextLevel() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes|LevelSystem")
	float GetExperienceRemainingForNextLevel() const;



	// --- GameplayEffects

	// Returns true if applied successfully, and false otherwise.
	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	bool AddGameplayEffect(UGameplayEffect* EffectToApply, FGameplayEffectHandle& OutHandle);

	// Returns true if applied successfully, and false otherwise.
	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	bool AddGameplayEffectFromType(TSubclassOf<UGameplayEffect> EffectToApply, FGameplayEffectHandle& OutHandle);

	// Removes the first GameplayEffect found of this type. Use 
	// Returns true if removed successfully, and false otherwise.
	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	bool RemoveGameplayEffect(const UGameplayEffect* EffectToRemove);

	// Removes the first GameplayEffect found of this type.
	// Returns true if removed successfully, and false otherwise.
	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	bool RemoveGameplayEffectFromType(TSubclassOf<UGameplayEffect> EffectToRemove);

	// Case sensitive. Use with caution when passing more generic names.
	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	bool RemoveAllGameplayEffectsWithName(FString Name);

	// Removes all GameplayEffects that has TagToRemove.
	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	bool RemoveAllGameplayEffectsWithTag(const FGameplayTag& TagToRemove);

	// Returns true if removed successfully, and false otherwise.
	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	bool RemoveGameplayEffectByHandle(const FGameplayEffectHandle& EffectToRemove);

	// Returns the amount of GameplayEffects removed.
	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	int RemoveGameplayEffectsByHandles(const TArray<FGameplayEffectHandle>& EffectsToRemove);

	// Gets all the Effects currently active in this Component
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayEffects")
	void GetActiveGameplayEffects(TMap<FGameplayEffectHandle, FActiveGameplayEffect>& EffectsOut) const;

	// Overwrites any currently applied Effects with EffectsIn
	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	void SetActiveGameplayEffects(const TMap<FGameplayEffectHandle, FActiveGameplayEffect>& EffectsIn);

	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	int GetActiveGameplayEffectsCount() const;

	inline FActiveGameplayEffect* GetActiveGameplayEffectByHandle(const FGameplayEffectHandle& Handle);

	void GetMatchingGameplayEffects(const FGameplayTagQuery& TagQuery, TArray<FGameplayEffectHandle>& OutHandles) const;

	// Returns the first GameplayEffect found that shares the same CDO as EffectToCheck.
	bool HasGameplayEffectOfInstance(const UGameplayEffect* EffectToCheck, FGameplayEffectHandle& OutHandle) const;

	// Returns true if a GameplayEffect with the given Handle is present.
	inline bool HasGameplayEffect(const FGameplayEffectHandle& Handle) const;

	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	void PauseAllGameplayEffects();

	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	void UnpauseAllGameplayEffects();



	// --- GameplayTags

	// This gets passed as a copy in Blueprint, so avoid using unless necessary for impermanent changes.
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Get GameplayTag System"), Category = "GameplayTags")
	void K2_GetGameplayTagSystem(FGameplayTagSystem& OutGameplayTagSystem) const;

	// Applies Blocking and Cancelling tags from an Ability. Optionally specify a Callee ability to avoid cancelling it in the case that it matches the CancellingTags.
	UFUNCTION(BlueprintCallable, Category = "GameplayTags")
	void ApplyBlockingAndCancellingTags(const FGameplayTagContainer& BlockingTags, const FGameplayTagContainer& CancellingTags, UGameplayAbility* Callee = nullptr);

	UFUNCTION(BlueprintCallable, Category = "GameplayTags")
	void RemoveBlockingTags(const FGameplayTagContainer& BlockingTags);

	// Returns any tags that are currently blocking ability activation
	UFUNCTION(BlueprintCallable, Category = "GameplayTags")
	FGameplayTagContainer GetBlockingAbilityTags() const;

	FGameplayTagSystem* GetGameplayTagSystem();

	FGameplayTagSystem& GetGameplayTagSystemAsRef();



	// --- Abilities

	// Returns true if a cooldown is present, false if not.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool HasCooldown(TSubclassOf<UGameplayAbility> AbilityClass) const;

	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool CanActivateAbility(TSubclassOf<UGameplayAbility> AbilityToQuery, const FGameplayAbilityActivationData& ActivationData);

	// Attempts to activate the specified Ability.
	// Can fail if Actor does not meet the ability's activation requirements or if a cooldown for said ability is present.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool UseAbility(TSubclassOf<UGameplayAbility> AbilityToUse, FGameplayAbilityHandle& OutHandle);

	// Attempts to activate the specified Ability with the given ActivationData.
	// Can fail if Actor does not meet the ability's activation requirements or if a cooldown for said ability is present.
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Use Ability With Activation Data"), Category = GameplayAbility)
	bool UseAbility_ActivationData(TSubclassOf<UGameplayAbility> AbilityToUse, const FGameplayAbilityActivationData& ActivationData, FGameplayAbilityHandle& OutHandle);

	// Forces the Ability to activate regardless of requirements or cooldowns. Can still fail if the ability's internal activation logic prevents it.
	bool UseAbility_NoRequirements(TSubclassOf<UGameplayAbility> AbilityToUse, FGameplayAbilityHandle& OutHandle);

	// Get the Ability Instance of the Handle. Returns nullptr if none exists or if the Handle is invalid.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	UGameplayAbility* GetAbilityInstanceFromHandle(const FGameplayAbilityHandle& Handle);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	FActiveGameplayAbility GetActiveAbilityFromHandle(const FGameplayAbilityHandle& Handle);

	FActiveGameplayAbility* GetActiveAbilityFromHandle_Ptr(const FGameplayAbilityHandle& Handle);

	FActiveGameplayAbility* GetActiveAbilityFromInstance_Ptr(UGameplayAbility* Instance);

	// Each index maps 1:1 between Handles and OutAbilities. Any index can be nullptr if the corresponding handle is invalid.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	void GetAbilityInstancesFromHandles(const TArray<FGameplayAbilityHandle> Handles, TArray<UGameplayAbility*>& OutAbilities);

	// Get the Handle for the Ability. Returns a invalid Handle if none exists.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	FGameplayAbilityHandle GetAbilityHandleFromInstance(const UGameplayAbility* Instance) const;

	// Get the Handle for the Ability. Returns a invalid Handle if none exists.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	FGameplayAbilityHandle GetAbilityHandleFromActiveAbility(const FActiveGameplayAbility& ActiveAbility);

	// Searches available abilities and returns any with TagToCheck.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	void GetAbilitiesByTag(const FGameplayTag& TagToCheck, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore = nullptr) const;

	// Searches only in active abilities and returns any with TagToCheck.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	void GetActiveAbilitiesByTag(const FGameplayTag& TagToCheck, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore = nullptr) const;

	// Searches available abilities and returns any with any tag from TagsToCheck.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	void GetAbilitiesByTags(const FGameplayTagContainer& TagsToCheck, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore = nullptr) const;

	// Searches only in active abilities and returns any with any tag from TagsToCheck
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	void GetActiveAbilitiesByTags(const FGameplayTagContainer& TagsToCheck, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore = nullptr) const;

	// Searches available abilities and returns.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	void GetAbilitiesByClass(TSubclassOf<UGameplayAbility> Class, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore = nullptr) const;

	// Searches only in active abilities and returns
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	void GetActiveAbilitiesByClass(TSubclassOf<UGameplayAbility> Class, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore = nullptr) const;

	// Searches available abilities and returns that matches the predicate.
	void GetAbilitiesByPredicate(std::function<bool(const UGameplayAbility*)> Predicate, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore = nullptr) const;
	
	// Searches only in active abilities and returns that matches the predicate.
	void GetActiveAbilitiesByPredicate(std::function<bool(const FActiveGameplayAbility*)> Predicate, TArray<FGameplayAbilityHandle>& OutHandles, UGameplayAbility* Ignore = nullptr) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	int GetAbilityInstanceCount() const;

	// Gets the amount of abilities that are currently active and executing.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	int GetActiveAbilityCount() const;

	// Attempts to use any Ability that is currently in queue.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void ActivateQueuedAbility();

	// Puts the Ability in the AbilityQueue. 
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void QueueAbility(TSubclassOf<UGameplayAbility> AbilityToQueue);

	// Clears out the AbilityQueue.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void ClearAbilityQueue();

	// Cancels the Ability if it is currently active. Returns true if the Ability was found and cancelled. 
	// bIsAuthoritative set to true will ignore the abilities IsCancellable value, and force the cancellation to go through.Use with caution!
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool CancelAbility(const FGameplayAbilityHandle& Handle, bool bIsAuthoritative = false);

	// Cancels all passed abilities, if they are active. Returns true if at least one ability was found and cancelled.
	// bIsAuthoritative set to true will ignore the abilities IsCancellable value, and force the cancellation to go through.Use with caution!
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool CancelAbilities(TArray<FGameplayAbilityHandle> HandlesToCancel, bool bIsAuthoritative = false);

	void AddAbilityInstance(TSubclassOf<UGameplayAbility> AbilityClass);

	void RemoveAbilityInstance(TSubclassOf<UGameplayAbility> AbilityClass);

	void InformAbilityEnded(UGameplayAbility* AbilityInstance);
	
	FGameplaySystemActorInfo* GetActorInfo() const;

	template<class T> 
	T* GetDerivedActorInfo() const
	{
		static_assert(TPointerIsConvertibleFromTo<FGameplaySystemActorInfo, T>::Value, "'T' template parameter to GetDerivedActorInfo must be derived from FGameplaySystemActorInfo");

		return CastChecked<T>(GetActorInfo());
	}



	// --- AnimMontage
	// Playing through the GameplaySystem interface allows us to keep track of what ability played what montage, which can then be used by abilities
	// to route AnimNotifies and Montage events to the correct ability.

	// Plays the Montage in the GameplaySystems AnimInstance. Returns the length of the played montage. 
	float PlayMontage(UGameplayAbility* PlayingAbility, UAnimMontage* MontageToPlay, FPlayMontageParams& Params);

	// Gets the ability that is currently animating, if any.
	UFUNCTION(BlueprintCallable, Category = "AnimMontage")
	UGameplayAbility* GetAnimatingAbility() const;

	UFUNCTION(BlueprintCallable, Category = "AnimMontage")
	void ClearAnimMontageInfo();

	FGameplaySystemAnimMontageInfo* GetAnimMontageInfo();

	UFUNCTION(BlueprintCallable, Category = "AnimMontage")
	UAnimMontage* GetCurrentAnimMontage() const;

	// Stops the current AnimMontage. Uses the AnimMontages own BlendOutTime if OverrideBlendOutTime is not passed.
	UFUNCTION(BlueprintCallable, Category = "AnimMontage")
	void StopCurrentMontage(float OverrideBlendOutTime = -1.0f);

	// --- General

	// Cancels any remaining abilities and destroys instances.
	UFUNCTION(BlueprintCallable)
	void DestroyActiveState();

	// Gets a snapshot of the system state.
	UFUNCTION(BlueprintCallable)
	FGameplaySystemSnapshot GetSnapshot();


protected:

	// Recalculates the values of all present attribute, taking into account any applied AttributeEffects.
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void EvaluateAttributes();

	// Sets up the ability by creating a new instance of the Ability.
	UGameplayAbility* CreateNewAbilityInstance(TSubclassOf<UGameplayAbility> AbilityToSetup);

	// Returns true if applied successfully, and false otherwise.
	bool AddGameplayEffect_Internal(FActiveGameplayEffect& EffectToApply, FGameplayEffectHandle& OutHandle);

	bool RemoveGameplayEffect_Internal(const FGameplayEffectHandle& EffectToRemove);

	void ApplyAttributeEffect_Internal(const FAttributeEffect& EffectToApply);

	void ApplyAttributeEffect_Internal_Instant(FAttributeEffect EffectToApply);

	void RegisterGameplayEffect(const FGameplayEffectHandle& Handle, FActiveGameplayEffect& ActiveEffect);

	int RemoveAllGameplayEffectsByPredicate(std::function<bool(const FActiveGameplayEffect&)> Predicate);

	bool UseAbility_Internal(TSubclassOf<UGameplayAbility> AbilityToUse, const FGameplayAbilityActivationData& ActivationData, FGameplayAbilityHandle& OutHandle);

	void EndAbility(const FGameplayAbilityHandle& Handle);

	void CollectModifiers(TArray<FAttributeEffect*>& AttributeEffects);

	// Creates a new Instance if required by it's EInstancingPolicy.
	UGameplayAbility* GetOrCreateAbilityInstance(TSubclassOf<UGameplayAbility> Ability, bool& bOutCreatedNew);

	// Boilerplate setup for a ability that is about to be activated. 
	FGameplayAbilityHandle PreActivateAbility(UGameplayAbility* AbilityInstance);

	// Any tags specified here are added on startup.
	UPROPERTY(EditAnywhere, Category = "GameplayTags")
	FGameplayTagContainer StartUpTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	TObjectPtr<UAttributeDataSet> AttributeDataSet = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes|Level System")
	TObjectPtr<UCurveTable> LevelScalingCurveTable = nullptr;

	// A value of -1 means we don't use the Level system, and won't respond to level-ups.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category = "Attributes|Level System")
	int EntityLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilityBuffering")
	bool bAllowAbilityQueueing = false;

	// --- Abilities

	UPROPERTY(BlueprintReadOnly, Category = "GameplayAbility")
	TMap<FGameplayAbilityHandle, TObjectPtr<UGameplayAbility>> AbilityInstanceMap;

	UPROPERTY(BlueprintReadOnly, Category = "GameplayAbility")
	TMap<FGameplayAbilityHandle, FActiveGameplayAbility> ActiveAbilityMap;

	// --- AnimMontage

	FGameplaySystemAnimMontageInfo AnimMontageInfo;

private:

	// Applies the Attribute Delta from level 0 to the current Editor-specified level.
	void InitializeLevelSystem();

	// Adds all the attributes specified in AttributeDataSet.
	void InitializeAttributes();

	// Adds all the GameplayTags in StartupTags, with a count of 1.
	void InitializeGameplayTags();

	// Attempts to build the ActorInfo from this components Owner.
	void InitializeGameplaySystemActorInfo();

	uint32 bIsDirty : 1 = true;

	uint32 bIsGameplayEffectsPaused : 1 = false;

	float ExpRequiredForNextLevel = 0.0f;

	UPROPERTY(SaveGame)
	float EntityExperience = 0.0f;

	uint32 CachedAttributeEffectCount = 0U;

	UPROPERTY(SaveGame)
	TMap<EAttributeType, FAttribute> Attributes;

	TMap<FGameplayEffectHandle, FActiveGameplayEffect> ActiveGameplayEffects;

	UPROPERTY(SaveGame)
	FGameplayTagSystem GameplayTagSystem;

	FGameplayTagSystem BlockedAbilityTags;

	TSubclassOf<UGameplayAbility> QueuedAbility = nullptr;

	TSharedPtr<FGameplaySystemActorInfo> GameplaySystemActorInfo = nullptr;

	UGameplaySystemDeveloperSettings const* GameplaySystemSettings = nullptr;


public:
	// --- Delegates

	// Invoked when any attribute has it's value changed. Use sparingly as this is called very often.
	UPROPERTY(BlueprintAssignable)
	FOnAnyAttributeChangedSignature OnAnyAttributeChangedDelegate;

	// Collection of AttributeChangedDelegates
	FDelegateCollection OnAttributeChangedDelegateCollection;

	// Invoked when one or more levelups are triggered by gaining experience.
	UPROPERTY(BlueprintAssignable)
	FOnLeveledUpSignature OnLeveledUpDelegate;

	// Invoked both on Cancel and End.
	UPROPERTY(BlueprintAssignable)
	FOnAnyAbilityEndedSignature OnAnyAbilityEndedDelegate;
};
