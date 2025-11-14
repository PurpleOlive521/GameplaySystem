// Copyright (c) 2025, Oliver Österlund Stare. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AttributeEffect.h"
#include "GameplayEffect.h"
#include "GameplayAbility.h"
#include "GameplayTagSystem.h"
#include "GameplayTagOwnerInterface.h"
#include "GameplaySystemTypes.h"
#include "functional"

#include "GameplaySystemComponent.generated.h"

struct FGameplaySystemSaveObject;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, EAttributeType, ChangedAttribute);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnyAttributeChangedSignature, EAttributeType, AttributeType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLeveledUpSignature, int, PreviousLevel, int, CurrentLevel, float, NextLevelExp);


// A collection of delegates that are accessed and managed through a single entry-point. 
// The delegates are created on-demand, and removed when no longer bound to.
// 
// Note: Due to the lack of concise API descriptions for the delegates themselves and types buried in Unreals templates, it is not possible to clean 
// the collection per-use, but are instead removed the next time the collection is accessed. We are not able to shadow the delegates own calls.
// This means that at any given time up to one delegate will be left in excess until accessed again. 
USTRUCT(BlueprintType)
struct GAMEPLAYSYSTEM_API FDelegateCollection
{
	GENERATED_BODY()

	TMap<EAttributeType, FOnAttributeChangedSignature> DelegateMap;

	// Returns the associated delegate. A delegate will be created if not already present, so avoid using to query the existance of delegates.
	// Instead use HasDelegate to query for delegates. 
	FOnAttributeChangedSignature& GetDelegate(EAttributeType Attribute);

	// Returns the associated delegates to the given keys.
	void GetMultipleDelegates(TArray<EAttributeType> Attributes, TArray<FOnAttributeChangedSignature*> OutDelegates);

	// Returns true if a delegate is present for the AttributeType
	inline bool HasDelegate(EAttributeType Attribute);

private:
	// Cleans up any delegates that are no longer bound to.
	void CheckDelegates();

	bool bIsDirty = false;
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
//
// - GameplayTags

UCLASS(Blueprintable, ClassGroup = GameplaySystem, hidecategories = (Object, LOD, Lighting, Transform, Sockets, TextureStreaming), meta=(BlueprintSpawnableComponent) )
class GAMEPLAYSYSTEM_API UGameplaySystemComponent : public UActorComponent, public IGameplayTagOwnerInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGameplaySystemComponent();

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// Called every frame
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

	// --- Save System

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SaveSystem")
	FGameplaySystemSaveObject SaveToObject() const;

	UFUNCTION(BlueprintCallable, Category = "SaveSystem")
	void LoadFromObject(const FGameplaySystemSaveObject& GameplaySystemSaveData);



	// --- Attributes

	// Get the value of an attribute, affected by any modifiers
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	float GetAttributeValue(EAttributeType AttributeType);

	// Get the value of an attribute, unaffected by any modifiers
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Attributes")
	float GetAttributeBaseValue(EAttributeType AttributeType);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void ModifyAttributeBaseValue(EAttributeType AttributeType, float ValueChange);

	// Current value is volatile! Use BaseValue for persistent changes
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void ModifyAttributeValue(EAttributeType AttributeType, float ValueChange);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeBaseValue(EAttributeType AttributeType, float NewValue);

	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SetAttributeValue(EAttributeType AttributeType, float NewValue);

	void ClampAttributeBaseValue(EAttributeType AttributeType, float Min, float Max);

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
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void SimulateAttributes(const TArray<FAttributeEffect>& AttributeEffectsToSimulate, TMap<EAttributeType, FAttribute>& GeneratedAttributesOut);



	// --- AttributeEffects

	void ApplyAttributeEffect(FAttributeEffect EffectToApply);

	// Does not register the AttributeEffect to allow for removal and instead applies the effect immediately.
	void ApplyAttributeEffectNoRemoval(FAttributeEffect EffectToApply);

	// Removes the first instance of the AttributeEffect.
	void RemoveAttributeEffect(FAttributeEffect& EffectToRemove);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AttributeEffects")
	int GetActiveEffectsCount();
	
	// Gets all the AttributesEffects in this GameplaySystemComponent
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AttributeEffects")
	void GetActiveAttributeEffects(TArray<FAttributeEffect>& ActiveEffectsOut) const;

	// Overwrites any present AttributeEffects with AttributeEffectsIn
	UFUNCTION(BlueprintCallable, Category = "AttributeEffects")
	void SetActiveAttributeEffects(const TArray<FAttributeEffect>& AttributeEffectsIn);



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

	// Use only for runtime-created GameplayEffects, where there is no custom UGameplayEffect derivative or instance to source it from.
	// Returns true if applied successfully, and false otherwise.
	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	bool AddGameplayEffectByHandle(const FActiveGameplayEffect& EffectToAdd, FGameplayEffectHandle& OutHandle);

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

	// Gets all the Effects currently active in this Component
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayEffects")
	void GetActiveGameplayEffects(TMap<FGameplayEffectHandle, FActiveGameplayEffect>& EffectsOut) const;

	// Overwrites any currently applied Effects with EffectsIn
	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	void SetActiveGameplayEffects(const TMap<FGameplayEffectHandle, FActiveGameplayEffect>& EffectsIn);

	UFUNCTION(BlueprintCallable, Category = "GameplayEffects")
	int GetActiveGameplayEffectsCount() const;

	bool HasGameplayEffectOfInstance(const UGameplayEffect* EffectToCheck, FGameplayEffectHandle& OutHandle) const;

	TMap<FGameplayEffectHandle, FActiveGameplayEffect>::TConstIterator GetConstGameplayEffectIterator() const;



	// --- GameplayTags

	// This gets passed as a copy in Blueprint, so avoid using unless necessary for impermanent changes.
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DisplayName = "Get GameplayTag System"), Category = "GameplayTags")
	void K2_GetGameplayTagSystem(FGameplayTagSystem& OutGameplayTagSystem) const;

	UFUNCTION(BlueprintCallable, Category = "GameplayTags")
	void ApplyBlockingAndCancellingTags(const FGameplayTagContainer& BlockingTags, const FGameplayTagContainer& CancellingTags);

	UFUNCTION(BlueprintCallable, Category = "GameplayTags")
	void RemoveBlockingTags(const FGameplayTagContainer& BlockingTags);

	// Returns any tags that are currently blocking ability activation
	UFUNCTION(BlueprintCallable, Category = "GameplayTags")
	FGameplayTagContainer GetBlockingAbilityTags() const;

	FGameplayTagSystem* GetGameplayTagSystem();

	FGameplayTagSystem& GetGameplayTagSystemAsRef();



	// --- Abilities

	// Checks if a cooldown is attached to this Ability Instance. Returns true if a cooldown is present, false if not.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool HasCooldown(const UGameplayAbility* AbilityToQuery) const;

	// Call to attempt to use the specified Ability. Registers the generated handle.
	// Can fail if Actor does not meet the ability's activation requirements or if a cooldown for said ability is present.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool UseAbility(TSubclassOf<UGameplayAbility> AbilityToUse, FActiveGameplayAbility& GeneratedHandle);

	// Forces the Ability to activate regardless of requirements or cooldowns. Will cancel any abilities that are blocking the activation, 
	// and will not call ApplyAbilityRequirements on the activating Ability.
	void UseAbility_NoRequirements(TSubclassOf<UGameplayAbility> AbilityToUse, FActiveGameplayAbility& GeneratedHandle);

	// Adds the ability to set up it's mutable instance. Required for the ability to be activatable
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void AddAbility(TSubclassOf<UGameplayAbility> AbilityToAdd);

	// Removes the ability and its mutable instance. Will fail if a handle is currently active.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool RemoveAbility(TSubclassOf<UGameplayAbility> AbilityToRemove);

	// Get the Ability Instance of the given Ability. Returns nullptr if none exists.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	UGameplayAbility* GetAbilityInstance(TSubclassOf<UGameplayAbility> Ability);

	// Get the handle for the Ability, if one exists. Returns true if a handle is found.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	bool GetAbilityHandle(TSubclassOf<UGameplayAbility> Ability, FActiveGameplayAbility& OutHandle);

	// Adds an already created Ability Instance and it's handle to the tables. 
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void AddAbilityHandle(UGameplayAbility* AbilityInstance, const FActiveGameplayAbility& Handle);

	// Gets the handle for the Ability, or creates one if it doesn't exist. Returns the Ability Instance.
	UGameplayAbility* GetOrAddAbilityInstance(TSubclassOf<UGameplayAbility> Ability, bool& OutGeneratedNewHandle);

	// Get the handle for the given Ability Instance, if one exists. Returns true if a handle is found.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	bool GetAbilityHandleFromInstance(UGameplayAbility* AbilityInstance, FActiveGameplayAbility& OutHandle);

	// Get a pointer to the handle for the given Ability Instance, if one exists. Returns nullptr if not found.
	FActiveGameplayAbility* GetAbilityHandlePtrFromInstance(UGameplayAbility* AbilityInstance);

	// Searches available abilities and returns any with TagToCheck.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	void GetAllAbilitiesWithTag(FGameplayTag TagToCheck, TArray<UGameplayAbility*>& AbilitiesOut) const;

	// Searches only in active abilities and returns any with TagToCheck.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	void GetAllActiveAbilitiesWithTag(FGameplayTag TagToCheck, TArray<UGameplayAbility*>& AbilitiesOut) const;

	// Searches available abilities and returns any with any tag from TagsToCheck.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	void GetAllAbilitiesWithTags(const FGameplayTagContainer& TagsToCheck, TArray<UGameplayAbility*>& AbilitiesOut) const;

	// Searches only in active abilities and returns any with any tag from TagsToCheck
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	void GetAllActiveAbilitiesWithTags(const FGameplayTagContainer& TagsToCheck, TArray<UGameplayAbility*>& AbilitiesOut) const;

	// Searches available abilities and returns that matches the predicate.
	void GetAllAbilitiesByPredicate(std::function<bool(const UGameplayAbility*)> Predicate, TArray<UGameplayAbility*>& AbilitiesOut) const;
	
	// Searches only in active abilities and returns that matches the predicate.
	void GetAllActiveAbilitiesByPredicate(std::function<bool(const UGameplayAbility*)> Predicate, TArray<UGameplayAbility*>& AbilitiesOut) const;

	// Gets the amount of abilities available. These may or may not be active.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GameplayAbility")
	int GetAbilityCount() const;

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

	// --- For all Cancel functions: bIsAuthoritative set to true will ignore the abilities IsCancellable value, and force the cancellation anyways. Use with caution!

	// Cancels the Ability if it is currently active. Returns true if the Ability was found and cancelled. 
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool CancelAbility(UGameplayAbility* AbilityToCancel, bool bIsAuthoritative = false);

	// Cancels all passed abilities, if they are active. Returns true if at least one ability was found and cancelled.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool CancelAbilities(TArray<UGameplayAbility*> AbilitiesToCancel, bool bIsAuthoritative = false);

	// Cancels all currently active Ability with this GameplayTag.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool CancelAllAbilitiesWithTag(FGameplayTag Tag, bool bIsAuthoritative = false);

	// Cancels all currently active Abilities with any of the passed GameplayTags.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	bool CancelAllAbilitiesWithTags(const FGameplayTagContainer& Tags, bool bIsAuthoritative = false);

	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void EndAbility(UGameplayAbility* AbilityToEnd);

	// Intended for when a Ability is requested to end early rather than ending by duration expiring. Allows us to track such calls in the future if necessary.
	UFUNCTION(BlueprintCallable, Category = "GameplayAbility")
	void EndAbilityEarly(UGameplayAbility* AbilityToEnd);
	
	FGameplaySystemActorInfo* GetActorInfo() const;

	TMap<TSubclassOf<UGameplayAbility>, TObjectPtr<UGameplayAbility>>::TConstIterator GetConstAbilityIterator() const;

	TMap<TObjectPtr<UGameplayAbility>, FActiveGameplayAbility>::TConstIterator GetConstActiveAbilityIterator() const;



	// --- AnimMontage
	// Playing through the GameplaySystem interface allows us to keep track of what ability played what montage, which can then be used by abilities
	// to route AnimNotifies and Montage events to the correct ability.

	// Plays the Montage in the GameplaySystems AnimInstance. Returns the length of the played montage. 
	float PlayMontage(UGameplayAbility* PlayingAbility, UAnimMontage* MontageToPlay, float PlayRate = 1.0f, FName StartSection = FName(), FName EndSection = FName());

	UFUNCTION(BlueprintCallable, Category = "AnimMontage")
	// Gets the ability that is currently animating, if any.
	UGameplayAbility* GetAnimatingAbility() const;

	UFUNCTION(BlueprintCallable, Category = "AnimMontage")
	void ClearAnimMontageInfo();

	FGameplaySystemAnimMontageInfo* GetAnimMontageInfo();


protected:

	TMap<TObjectPtr<UGameplayAbility>, FActiveGameplayAbility>::TIterator GetActiveAbilityIterator();

	// Recalculates the values of all present attribute, taking into account any applied AttributeEffects.
	UFUNCTION(BlueprintCallable, Category = "Attributes")
	void EvaluateAttributes();

	// Sets up the ability by creating a new instance of the Ability.
	void CreateAbilityInstance(TSubclassOf<UGameplayAbility> AbilityToSetup);

	// Returns true if applied successfully, and false otherwise.
	bool AddGameplayEffect_Internal(const FActiveGameplayEffect& EffectToApply, FGameplayEffectHandle& OutHandle);

	int RemoveAllGameplayEffectsByPredicate(std::function<bool(const FActiveGameplayEffect&)> Predicate);

	// Any tags specified here are added on startup.
	UPROPERTY(EditAnywhere, Category = "GameplayTags")
	FGameplayTagContainer StartUpTags;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes")
	UAttributeDataSet* AttributeDataSet;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attributes|Level System")
	UCurveTable* LevelScalingCurveTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attributes|Level System")
	int EntityLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AbilityBuffering")
	bool bAllowAbilityQueueing = false;

	// --- Abilities

	// We reuse ability instances to make the abilities and their fields mutable. 
	// Populated automatically by FGameplayAbilitySlots or explicitly by using AddAbility
	UPROPERTY(BlueprintReadOnly, Category = "GameplayAbility")
	TMap<TSubclassOf<UGameplayAbility>, TObjectPtr<UGameplayAbility>> AvailableAbilities;

	// Table that maps an GameplayAbility instance to a handle. Can be queried to get the abilities cooldowns and duration.
	// A handle is created when the ability is activated, and removed when it's Ended or Cancelled.
	UPROPERTY(BlueprintReadOnly, Category = "GameplayAbility")
	TMap<TObjectPtr<UGameplayAbility>, FActiveGameplayAbility> ActiveAbilitesTable;

	// --- AnimMontage

	FGameplaySystemAnimMontageInfo AnimMontageInfo;

private:

	// Applies the Attribute Delta from level 0 to the current Editor-specified level.
	void InitializeLevelSystem();

	// Adds all the attributes specified in AttributeDataSet.
	void InitializeAttributes();

	// Adds all the GameplayTags in StartupTags, with a count of 1.
	void InitializeGameplayTags();

	// Does the setup for all abilities already in AvailableAbilities at startup.
	void InitializeGameplayAbilities();

	// Attempts to build the ActorInfo from this components Owner.
	void InitializeGameplaySystemActorInfo();

	// Will not call OnAttributeChangedDelegate if true
	bool bChangeSilently = false;

	bool bIsDirty = true;

	TMap<EAttributeType, FAttribute> Attributes;

	// Active effects that can be removed or modified
	TArray<FAttributeEffect> ActiveAttributeEffects;

	// Applied Gameplay effects. These are ensured to be valid, and have had their effects applied
	TMap<FGameplayEffectHandle, FActiveGameplayEffect> ActiveGameplayEffects;

	// We cache this to avoid querying the DataTable for the Exp required every time
	float CachedExpForNextLevel = 0;

	float EntityExperience = 0;

	FGameplayTagSystem GameplayTagSystem;

	FGameplayTagSystem BlockedAbilityTags;

	// Cached info about this components Owner and some frequently used properties.
	TSharedPtr<FGameplaySystemActorInfo> GameplaySystemActorInfo;

	TSubclassOf<UGameplayAbility> QueuedAbility;

public:
	// --- Delegates

	// Called when any attribute has it's value changed. Use sparingly as this is called very often.
	UPROPERTY(BlueprintAssignable)
	FOnAnyAttributeChangedSignature OnAnyAttributeChangedDelegate;

	// Collection of AttributeChangedDelegates
	FDelegateCollection OnAttributeChangedDelegateCollection;

	// Called when one or more levelups are triggered by gaining experience
	UPROPERTY(BlueprintAssignable)
	FOnLeveledUpSignature OnLeveledUpDelegate;
};
