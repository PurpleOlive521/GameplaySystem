// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplaySystemDebugWidget.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "EnhancedInputSubsystems.h"
#include "GameplaySystemComponent.h"
#include "InputMappingContext.h"
#include "GameplaySystemOwnerInterface.h"
#include "GameplayEventSubsystem.h"
#include "GameplayEvent.h"

#include "GameplaySystemBlueprintLibrary.h"
#include "DevelopmentTypes.h"

using namespace DebugTypes;

FAttributeDisplayInfo FAttributeDisplayInfo::MakeInvalid()
{
	FAttributeDisplayInfo Info = {};
	Info.TypeInfo = "Invalid";
	Info.ValueInfo = "0";

	return Info;
}

FEventsDisplayInfo FEventsDisplayInfo::MakeInvalid()
{
	FEventsDisplayInfo Info = {};
	Info.ActiveEvents = "Invalid";
	Info.InactiveEvents = "Invalid";

	return Info;
}

TArray<TWeakObjectPtr<UGameplaySystemComponent>> UGameplaySystemDebugWidget::ActiveSystemCandidates = {};

void UGameplaySystemDebugWidget::CleanSystemCandidates()
{
	TArray<TWeakObjectPtr<UGameplaySystemComponent>> InvalidCandidates;

	for (TWeakObjectPtr<UGameplaySystemComponent> Candidate : ActiveSystemCandidates)
	{
		if (!Candidate.IsValid())
		{
			InvalidCandidates.Emplace(Candidate);
			continue;
		}
	}

	for (TWeakObjectPtr<UGameplaySystemComponent> InvalidSystem : InvalidCandidates)
	{
		ActiveSystemCandidates.Remove(InvalidSystem);
	}
}

void UGameplaySystemDebugWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindToPlayer();
	BindToGameplayEventSubsystem();
}

void UGameplaySystemDebugWidget::NativeDestruct()
{
	// This can be null on PIE close, so we need to check for that first before trying to load assets for unbinding
	UEnhancedInputLocalPlayerSubsystem* DerefInputSystem = UsedInputSystem.Get();
	UEnhancedInputComponent* DerefInputComponent = UsedInputComponent.Get();

	if (DerefInputSystem && DerefInputComponent)
	{
		DerefInputSystem->RemoveMappingContext(InputMapping.LoadSynchronous());
		DerefInputComponent->RemoveBindingByHandle(ToggleMenuHandle);
		DerefInputComponent->RemoveBindingByHandle(CycleTargetHandle);
		DerefInputComponent->RemoveBindingByHandle(CycleMenuHandle);
	}

	Super::NativeDestruct();
}

void UGameplaySystemDebugWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	K2_TickGenericDisplay(InDeltaTime);

	if (bEnabled && BoundGameplaySystem.IsValid() && BoundGameplaySystem->GetOwner())
	{
		K2_TickGameplaySystemDisplay(InDeltaTime);
		DrawDebugLines();
	}

	if (BoundEventSubsystem.IsValid())
	{
		K2_TickGameplayEventDisplay(InDeltaTime);
	}

	OnEnabledTick(MyGeometry, InDeltaTime);
}

FString UGameplaySystemDebugWidget::GetGenericDisplayInfo() const
{
	FString DisplayInfo = TextTag_Header + TEXT("Gameplay System Debug Info:") + TextTag_End + ENDL;
	DisplayInfo += TextTag_Accept + TEXT("Press P to disable the UI, O to cycle the target Actor") + TextTag_End + ENDL;
	DisplayInfo += TextTag_Accept + TEXT("and L to switch the displayed page.") + TextTag_End + ENDL;
	DisplayInfo += TEXT("Has GameplaySystemComponent: ") + TextTag_Highlight + (BoundGameplaySystem.IsValid() ? FString(TEXT("Yes")) : FString(TEXT("No"))) + TextTag_End + ENDL;

	if (BoundGameplaySystem.IsValid())
	{
		AActor* Owner = BoundGameplaySystem->GetOwner();
		DisplayInfo += TEXT("Target Actor: ") + TextTag_Highlight + (Owner ? Owner->GetActorNameOrLabel() : TEXT("Invalid")) + TextTag_End + ENDL;
	}

	return DisplayInfo;
}

FAttributeDisplayInfo UGameplaySystemDebugWidget::GetAttributeDisplayInfo(TArray<FString>& OutArray) const
{
	UGameplaySystemComponent* GameplaySystem = BoundGameplaySystem.Get();
	if (!GameplaySystem)
	{
		return FAttributeDisplayInfo();
	}

	if (FORCE_RECALCULATE_ATTRIBUTES)
	{
		GameplaySystem->ForceEvaluateAttributes();
	}

	FString TypeInfo = TextTag_Header + TEXT("Current Attributes:") + TextTag_End + ENDL;
	FString ValueInfo = TextTag_Header + TextTag_End + ENDL;

	FAttributeDisplayInfo DisplayInfo;

	for (auto AtrIt = GameplaySystem->GetConstAttributeIterator(); AtrIt; ++AtrIt)
	{
		FAttributeString AtrStruct = AtrIt->Value.ToStringStruct();
		DisplayInfo.TypeInfo += AtrStruct.Type + TEXT(": ") + ENDL;
		DisplayInfo.ValueInfo += TextTag_Highlight + AtrStruct.BaseValue + TextTag_End + TEXT(" | ");
		DisplayInfo.ValueInfo += TextTag_Highlight + AtrStruct.CurrentValue + TextTag_End + ENDL;
	}

	return DisplayInfo;
}

FString UGameplaySystemDebugWidget::GetGameplayEffectsDisplayInfo() const
{
	UGameplaySystemComponent* GameplaySystem = BoundGameplaySystem.Get();
	if (!GameplaySystem)
	{
		return FString();
	}
	
	FString DisplayInfo = TextTag_Header + FString::Printf(TEXT("Current GameplayEffects: %d"), GameplaySystem->GetGameplayEffectsCount()) + TextTag_End + ENDL;

	for (const auto& [Handle, ActiveEffect] : GameplaySystem->ActiveGameplayEffects)
	{
		DisplayInfo += ActiveEffect.ToString() + ENDL;
	}

	return DisplayInfo;
}

FString UGameplaySystemDebugWidget::GetLevelSystemDisplayInfo() const
{
	UGameplaySystemComponent* GameplaySystem = BoundGameplaySystem.Get();
	if (!GameplaySystem)
	{
		return FString();
	}

	FString DisplayInfo = TextTag_Header + TEXT("Level System") + TextTag_End + ENDL;
	DisplayInfo += TEXT("Current Level: ") + TextTag_Highlight + FString::FromInt(GameplaySystem->GetEntityLevel()) + TextTag_End + ENDL;
	DisplayInfo += TEXT("Current Exp: ") + TextTag_Highlight + FString::FromInt(GameplaySystem->GetEntityExperience()) + TextTag_End + ENDL;
	DisplayInfo += TEXT("Exp required: ") + TextTag_Highlight + FString::FromInt(GameplaySystem->GetRequiredExperienceForNextLevel()) + TextTag_End;
	DisplayInfo += TEXT(" | ") + TextTag_Highlight + FString::FromInt(GameplaySystem->GetExperienceRemainingForNextLevel()) + TEXT(" left") + TextTag_End + ENDL;

	return DisplayInfo;
}

FString UGameplaySystemDebugWidget::GetAvailableAbilitiesDisplayInfo() const
{
	UGameplaySystemComponent* GameplaySystem = BoundGameplaySystem.Get();
	if (!GameplaySystem)
	{
		return FString();
	}

	FString DisplayInfo = TextTag_Header + FString::Printf(TEXT("Ability Instances: %d"), GameplaySystem->GetAbilityInstanceCount()) + TextTag_End + ENDL;

	for(const auto& [Handle, Instance] : GameplaySystem->AbilityInstanceMap)
	{
		DisplayInfo += Instance->ToStringWithDebugTags() + ENDL;
	}

	return DisplayInfo;
}

FString UGameplaySystemDebugWidget::GetActiveAbilitiesDisplayInfo() const
{
	UGameplaySystemComponent* GameplaySystem = BoundGameplaySystem.Get();
	if (!GameplaySystem)
	{
		return FString();
	}

	FString DisplayInfo = TextTag_Header + FString::Printf(TEXT("Active Abilities: %d"), GameplaySystem->GetActiveAbilityCount()) + TextTag_End + ENDL;

	for (const auto& [Handle, ActiveAbility] : GameplaySystem->ActiveAbilityMap)
	{
		DisplayInfo += ActiveAbility.ToStringWithDebugTags() + ENDL;
	}

	return DisplayInfo;
}

FString UGameplaySystemDebugWidget::GetGameplayTagSystemDisplayInfo() const
{
	UGameplaySystemComponent* GameplaySystem = BoundGameplaySystem.Get();
	if (!GameplaySystem)
	{
		return FString();
	}

	FString DisplayInfo = TextTag_Header + FString::Printf(TEXT("Applied Tags: %d"), GameplaySystem->GetGameplayTagSystem()->GetTotalTagCount()) + TextTag_End + ENDL;
	
	TArray<FString> Tags;
	GameplaySystem->GetGameplayTagSystem()->ToStringArrayWithDebugTags(Tags);

	for (const FString& Tag : Tags)
	{
		DisplayInfo += Tag + ENDL;
	}
	
	return DisplayInfo;
}

FEventsDisplayInfo UGameplaySystemDebugWidget::GetGlobalGameplayEventDisplayInfo() const
{
	UGameplayEventSubsystem* EventSubsystem = BoundEventSubsystem.Get();
	if (!EventSubsystem)
	{
		return FEventsDisplayInfo();
	}

	int ActiveEventCount = 0;
	FString ActiveContent = {};

	int InactiveEventCount = 0;
	FString InactiveContent = {};
	for (const auto& [Handle, Event] : EventSubsystem->EventMap)
	{
		if (Event->IsActive())
		{
			ActiveContent += Event->ToStringWithDebugTags() + ENDL;
			ActiveEventCount++;
		}
		else
		{
			InactiveContent += Event->ToStringWithDebugTags() + ENDL;
			InactiveEventCount++;
		}
	}

	FEventsDisplayInfo DisplayInfo;

	FString ActiveTitle = TextTag_Header + FString::Printf(TEXT("Total Active GameplayEvents: %d"), ActiveEventCount) + TextTag_End + ENDL;
	DisplayInfo.ActiveEvents = ActiveTitle + ActiveContent;

	FString InactiveTitle = TextTag_Header + FString::Printf(TEXT("Total Inactive GameplayEvents: %d"), InactiveEventCount) + TextTag_End + ENDL;
	DisplayInfo.InactiveEvents = InactiveTitle + InactiveContent;

	return DisplayInfo;
}

FEventsDisplayInfo UGameplaySystemDebugWidget::GetActorGameplayEventDisplayInfo() const
{
	UGameplayEventSubsystem* EventSubsystem = BoundEventSubsystem.Get();
	if (!EventSubsystem)
	{
		return FEventsDisplayInfo::MakeInvalid();
	}

	UGameplaySystemComponent* GameplaySystem = BoundGameplaySystem.Get();
	if (!GameplaySystem)
	{
		return FEventsDisplayInfo::MakeInvalid();
	}

	AActor* OwningActor = GameplaySystem->GetOwner();
	if (!GameplaySystem || !OwningActor)
	{
		return FEventsDisplayInfo::MakeInvalid();
	}

	int ActiveEventCount = 0;
	FString ActiveContent;

	int InactiveEventCount = 0;
	FString InactiveContent;

	FActorGameplayEventContainer EventContainer = EventSubsystem->PerActorEventMap.FindRef(OwningActor);
	TArray<UGameplayEvent*> ActorEvents;
	EventSubsystem->GetEventsFromHandles(EventContainer.GameplayEvents, ActorEvents);
	for (const UGameplayEvent* Event : ActorEvents)
	{
		if (!Event)
		{
			// Event is GC'd
			continue;
		}

		if (Event->IsActive())
		{
			ActiveContent += Event->ToStringWithDebugTags() + ENDL;
			ActiveEventCount++;
		}
		else
		{
			InactiveContent += Event->ToStringWithDebugTags() + ENDL;
			InactiveEventCount++;
		}
	}

	FEventsDisplayInfo DisplayInfo;

	FString ActiveTitle = TextTag_Header + FString::Printf(TEXT("Target Actor Active GameplayEvents: %d"), ActiveEventCount) + TextTag_End + ENDL;
	DisplayInfo.ActiveEvents = ActiveTitle + ActiveContent;

	FString InactiveTitle = TextTag_Header + FString::Printf(TEXT("Target Actor Inactive GameplayEvents: %d"), InactiveEventCount) + TextTag_End + ENDL;
	DisplayInfo.InactiveEvents = InactiveTitle + InactiveContent;

	return DisplayInfo;
}

void UGameplaySystemDebugWidget::DrawDebugLines()
{
	if (UGameplaySystemComponent* GameplaySystem = BoundGameplaySystem.Get(); AActor * Owner = GameplaySystem->GetOwner())
	{
		const FVector StartPoint = Owner->GetActorLocation();
		const FVector EndPoint = StartPoint + (Owner->GetActorUpVector() * 10000.0f);
		DrawDebugLine(GetWorld(), StartPoint, EndPoint, FColor::Red, false, 0.0f, 0U, 1.0f);
	}
}

void UGameplaySystemDebugWidget::BindToPlayer()
{
	if (InputMapping.IsNull())
	{
		GS_LOG(Error, TEXT("GameplaySystemDebugWidget: No input mapping set for debug menu!"));
		return;
	}

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!Player || !PlayerController)
	{
		GS_LOG(Error, TEXT("GameplaySystemDebugWidget: No player found!"));
		return;
	}
	
	BindToGameplaySystem(UGameplaySystemComponent::GetGameplaySystemFromActor(Player));

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();

	if (!LocalPlayer)
	{
		GS_LOG(Error, TEXT("GameplaySystemDebugWidget: No local player found!"));
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if (!InputSystem)
	{
		GS_LOG(Error, TEXT("GameplaySystemDebugWidget: No EnhancedInputSubsystem found!"));
		return;
	}

	InputSystem->AddMappingContext(InputMapping.LoadSynchronous(), 1);

	UsedInputSystem = MakeWeakObjectPtr(InputSystem); // Store it so we can remove the mapping later

	if (UEnhancedInputComponent* PlayerInputComponent = Cast<UEnhancedInputComponent>(Player->InputComponent))
	{
		FEnhancedInputActionEventBinding& ToggleMenuBinding = PlayerInputComponent->BindAction(ToggleMenuAction.LoadSynchronous(), ETriggerEvent::Started, this, &UGameplaySystemDebugWidget::ToggleWidget);
		ToggleMenuHandle = ToggleMenuBinding.GetHandle();

		FEnhancedInputActionEventBinding& CycleTargetBinding = PlayerInputComponent->BindAction(CycleTargetAction.LoadSynchronous(), ETriggerEvent::Started, this, &UGameplaySystemDebugWidget::CycleDebugTarget);
		CycleTargetHandle = CycleTargetBinding.GetHandle();

		FEnhancedInputActionEventBinding& CycleMenuBinding = PlayerInputComponent->BindAction(CycleMenuAction.LoadSynchronous(), ETriggerEvent::Started, this, &UGameplaySystemDebugWidget::CycleMenu);
		CycleMenuHandle = CycleMenuBinding.GetHandle();

		UsedInputComponent = MakeWeakObjectPtr(PlayerInputComponent); // Store it so we can unbind the actions later
	}
}

void UGameplaySystemDebugWidget::BindToGameplaySystem(UGameplaySystemComponent* NewSystem)
{
	if(!NewSystem)
	{
		GS_LOG(Warning, TEXT("GameplaySystemDebugWidget: Attempted to bind to null GameplaySystemComponent!"));
		return;
	}

	BoundGameplaySystem = MakeWeakObjectPtr(NewSystem);

	SystemCandidateIndex = ActiveSystemCandidates.IndexOfByKey(NewSystem);

	if (SystemCandidateIndex == INDEX_NONE)
	{
		ActiveSystemCandidates.Add(NewSystem);
		SystemCandidateIndex = ActiveSystemCandidates.Num() - 1;
	}

	CleanSystemCandidates();

	OnBoundToGameplaySystem(NewSystem);
}

void UGameplaySystemDebugWidget::BindToGameplayEventSubsystem()
{
	UGameplayEventSubsystem* EventSubsystem = UGameplayEventSubsystem::Get(this);
	ensure(EventSubsystem); // Should not be invalid during gameplay.

	BoundEventSubsystem = MakeWeakObjectPtr(EventSubsystem);
}

void UGameplaySystemDebugWidget::ToggleWidget()
{
	const ESlateVisibility VisibilityState = GetVisibility();

	if (VisibilityState == ESlateVisibility::Collapsed || VisibilityState == ESlateVisibility::Hidden)
	{    // Is not enabled yet, enable it
		bEnabled = true;
		SetVisibility(ESlateVisibility::HitTestInvisible);	}
	else // Is already enabled, disable it
	{
		bEnabled = false;
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UGameplaySystemDebugWidget::CycleDebugTarget()
{
	if (!bEnabled)
	{
		return;
	}

	TArray<AActor*> GameplaySystemActors;
	UGameplayStatics::GetAllActorsWithInterface(GetWorld(), UGameplaySystemOwnerInterface::StaticClass(), GameplaySystemActors);

	if (GameplaySystemActors.Num() == 0)
	{
		GS_LOG(Warning, TEXT("GameplaySystemDebugWidget: No more GameplaySystemComponents could be found. Do they implement IGameplaySystemOwnerInterface?"));
		return;
	}

	// Add any newly created GameplaySystems to the list of candidates 
	for (const AActor* Actor : GameplaySystemActors)
	{	
		if (UGameplaySystemComponent* GameplaySystem = UGameplaySystemComponent::GetGameplaySystemFromActor(Actor))
		{
			ActiveSystemCandidates.AddUnique(GameplaySystem);
		}
		else
		{
			GS_LOG(Error, TEXT("GameplaySystemDebugWidget: Found Actor did not own GameplaySystem: %s"), *Actor->GetName());
		}
	}

	SystemCandidateIndex = ActiveSystemCandidates.IsValidIndex(SystemCandidateIndex) ? SystemCandidateIndex : 0;

	// Cycle through the candidates until we find a valid new one, or have tried them all.
	int AttemptedCandidates = 0;
	const int StartingCandidates = ActiveSystemCandidates.Num();
	while (AttemptedCandidates < StartingCandidates)
	{
		const TWeakObjectPtr<UGameplaySystemComponent>& CurrentCandidate = ActiveSystemCandidates[SystemCandidateIndex];

		// We ensure it's a valid AND new candidate before switching to it
		if (CurrentCandidate.IsValid() && CurrentCandidate != BoundGameplaySystem)
		{
			K2_ClearDisplay();
			BindToGameplaySystem(CurrentCandidate.Get());
			return;
		}

		// Loop back to the start of the array if out-of-bounds.
		SystemCandidateIndex++;
		if (SystemCandidateIndex >= ActiveSystemCandidates.Num())
		{
			CleanSystemCandidates(); // Use the opportunity to clear out invalid candidates while it won't affect the index.
			SystemCandidateIndex = 0;
		}

		AttemptedCandidates++;
	}
}

void UGameplaySystemDebugWidget::CycleMenu()
{
	const int PreviousPage = DisplayedPage;

	DisplayedPage++;

	if (DisplayedPage > PageCount)
	{
		DisplayedPage = PAGE_START_INDEX;
	}
	else if (DisplayedPage > PageCount)
	{
		DisplayedPage = PAGE_START_INDEX;
	}

	if (PreviousPage != DisplayedPage)
	{
		K2_CycleMenu(DisplayedPage);
	}
}
