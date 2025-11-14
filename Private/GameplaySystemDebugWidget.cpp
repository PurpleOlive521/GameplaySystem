// Copyright (c) 2025, Heavy Duty Tape Studios. All rights reserved.


#include "GameplaySystemDebugWidget.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "EnhancedInputSubsystems.h"
#include "GameplaySystemComponent.h"
#include "InputMappingContext.h"
#include "GameplaySystemOwnerInterface.h"

#include "GameplaySystemBlueprintLibrary.h"
#include "DevelopmentTypes.h"

// Helps make the tags shorter, making the FStrings more readable
using namespace DebugTypes;

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
	}

	Super::NativeDestruct();
}

void UGameplaySystemDebugWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	TickGenericDisplay(InDeltaTime);

	if (!bEnabled || BoundGameplaySystem.IsExplicitlyNull())
	{
		return;
	}

	TickGameplaySystemDisplay(InDeltaTime);
	DrawDebugLines();
}

FString UGameplaySystemDebugWidget::GetGenericDisplayInfo() const
{
	FString DisplayInfo = TextTag_Header + TEXT("Gameplay System Debug Info:") + TextTag_End + ENDL;
	DisplayInfo += TextTag_Accept + TEXT("Press P to disable the UI, or O to cycle the target Actor.") + TextTag_End + ENDL;
	DisplayInfo += TEXT("Target Actor: ") + TextTag_Highlight + BoundGameplaySystem->GetOwner()->GetActorNameOrLabel() + TextTag_End + ENDL;
	DisplayInfo += TEXT("Has GameplaySystemComponent: ") + TextTag_Highlight + (BoundGameplaySystem.IsValid() ? FString(TEXT("Yes")) : FString(TEXT("No")) ) + TextTag_End + ENDL;

	return DisplayInfo;
}

void UGameplaySystemDebugWidget::GetAttributeDisplayInfo(TArray<FString>& OutArray) const
{
	UGameplaySystemComponent* GameplaySystem = BoundGameplaySystem.Get();
	if (!GameplaySystem)
	{
		return;
	}

	FString TypeInfo = TextTag_Header + TEXT("Current Attributes:") + TextTag_End + ENDL;
	FString ValueInfo = TextTag_Header + TextTag_End + ENDL;

	TArray<FString> OutString;
	for (auto AtrIt = GameplaySystem->GetConstAttributeIterator(); AtrIt; ++AtrIt)
	{
		AtrIt->Value.ToStringArray(OutString);
		TypeInfo += OutString[0] + TEXT(": ") + ENDL;
		ValueInfo += TextTag_Highlight + OutString[1] + TextTag_End + TEXT(" | ");
		ValueInfo += TextTag_Highlight + OutString[2] + TextTag_End + ENDL;
	}

	OutArray.SetNumZeroed(2, true);
	OutArray.EmplaceAt(0, TypeInfo);
	OutArray.EmplaceAt(1, ValueInfo);
}

FString UGameplaySystemDebugWidget::GetGameplayEffectsDisplayInfo() const
{
	UGameplaySystemComponent* GameplaySystem = BoundGameplaySystem.Get();
	if (!GameplaySystem)
	{
		return FString();
	}
	
	FString DisplayInfo = TextTag_Header + FString::Printf(TEXT("Current GameplayEffects: %d"), GameplaySystem->GetActiveGameplayEffectsCount()) + TextTag_End + ENDL;

	for (auto EffectIt = GameplaySystem->GetConstGameplayEffectIterator(); EffectIt; ++EffectIt)
	{
		DisplayInfo += EffectIt->Value.ToString() + ENDL;
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

	FString DisplayInfo = TextTag_Header + FString::Printf(TEXT("Available Abilities: %d"), GameplaySystem->GetAbilityCount()) + TextTag_End + ENDL;

	for(auto AbilityIt = GameplaySystem->GetConstAbilityIterator(); AbilityIt; ++AbilityIt)
	{
		DisplayInfo += AbilityIt->Value->ToStringWithDebugTags() + ENDL;
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

	for (auto AbilityIt = GameplaySystem->GetConstActiveAbilityIterator(); AbilityIt; ++AbilityIt)
	{
		DisplayInfo += AbilityIt->Value.ToStringWithDebugTags() + ENDL;
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
		GS_LOG(Error, TEXT("No input mapping set for debug menu!"));
		return;
	}

	ACharacter* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!Player || !PlayerController)
	{
		GS_LOG(Error, TEXT("No player found!"));
		return;
	}

	BindToGameplaySystem(Player->FindComponentByClass<UGameplaySystemComponent>());

	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();

	if (!LocalPlayer)
	{
		GS_LOG(Error, TEXT("No local player found!"));
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if (!InputSystem)
	{
		GS_LOG(Error, TEXT("No EnhancedInputSubsystem found!"));
		return;
	}

	InputSystem->AddMappingContext(InputMapping.LoadSynchronous(), 1);

	UsedInputSystem = InputSystem; // Store it so we can remove the mapping later

	if (UEnhancedInputComponent* PlayerInputComponent = Cast<UEnhancedInputComponent>(Player->InputComponent))
	{
		FEnhancedInputActionEventBinding& ToggleMenuBinding = PlayerInputComponent->BindAction(ToggleMenuAction.LoadSynchronous(), ETriggerEvent::Started, this, &UGameplaySystemDebugWidget::ToggleWidget);
		ToggleMenuHandle = ToggleMenuBinding.GetHandle();

		FEnhancedInputActionEventBinding& CycleTargetBinding = PlayerInputComponent->BindAction(CycleTargetAction.LoadSynchronous(), ETriggerEvent::Started, this, &UGameplaySystemDebugWidget::CycleDebugTarget);
		CycleTargetHandle = CycleTargetBinding.GetHandle();

		UsedInputComponent = PlayerInputComponent; // Store it so we can unbind the actions later
	}
}

void UGameplaySystemDebugWidget::BindToGameplaySystem(UGameplaySystemComponent* NewSystem)
{
	if(!NewSystem)
	{
		GS_LOG(Warning, TEXT("Attempted to bind to null GameplaySystemComponent!"));
		return;
	}

	BoundGameplaySystem = NewSystem;

	SystemCandidateIndex = ActiveSystemCandidates.IndexOfByKey(NewSystem);

	if (SystemCandidateIndex == INDEX_NONE)
	{
		ActiveSystemCandidates.Add(NewSystem);
		SystemCandidateIndex = ActiveSystemCandidates.Num() - 1;
	}

	CleanSystemCandidates();
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
		GS_LOG(Warning, TEXT("No more GameplaySystemComponents could be found. Do they implement IGameplaySystemOwnerInterface?"));
		return;
	}

	// Add any newly created GameplaySystems to the list of candidates 
	for (const AActor* Actor : GameplaySystemActors)
	{	
		if (UGameplaySystemComponent* GameplaySystem = Actor->GetComponentByClass<UGameplaySystemComponent>())
		{
			ActiveSystemCandidates.AddUnique(GameplaySystem);
		}
		else
		{
			GS_LOG(Error, TEXT("Found Actor did not own GameplaySystem: %s"), *Actor->GetName());
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
			ClearDisplay();
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
