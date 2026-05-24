// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "SaveGameFunctionLibrary.h"
#include "SaveGameProxyArchive.h"
#include "SaveableObjectInterface.h"
#include "GameFramework/Character.h"

bool USaveGameFunctionLibrary::WasObjectLoaded(const UObject* Object)
{
	return Object && Object->HasAnyFlags(RF_WasLoaded | RF_LoadCompleted);
}

bool USaveGameFunctionLibrary::IsLoading(const FSaveGameArchive& Archive)
{
	return Archive.IsValid() && Archive.GetRecord().GetUnderlyingArchive().IsLoading();
}

bool USaveGameFunctionLibrary::SerializeActorTransform(FSaveGameArchive& Archive, AActor* Actor)
{
	if (Archive.IsValid() && IsValid(Actor))
	{
		const bool bIsMovable = Actor->IsRootComponentMovable();
		const bool bIsLoading = IsLoading(Archive);

		// Save into a slot only if the actor is movable
		return (LOADING || bIsMovable) && Archive.SerializeField(TEXT("ActorTransform"), [&](FStructuredArchive::FSlot Slot)
			{
				FTransform ActorTransform;

				if (SAVING)
				{
					ActorTransform = Actor->GetActorTransform();
				}

				Slot << ActorTransform;

				if (LOADING && bIsMovable)
				{
					// If the actor is movable, set its transform
					Actor->SetActorTransform(ActorTransform, false /* bSweep */, nullptr, ETeleportType::TeleportPhysics);
				}
			});
	}

	return false;
}

bool USaveGameFunctionLibrary::SerializeGenericController(UPARAM(ref)FSaveGameArchive& Archive, AController* Controller)
{
	if (Archive.IsValid() && IsValid(Controller))
	{
		const bool bIsLoading = IsLoading(Archive);

		return Archive.SerializeField(TEXT("GenericController"), [&](FStructuredArchive::FSlot Slot)
			{
				FRotator ControlRotation;

				if (SAVING)
				{
					ControlRotation = Controller->GetControlRotation();
				}

				Slot << ControlRotation;

				if (LOADING)
				{
					Controller->SetControlRotation(ControlRotation);
				}
			});
	}

	return false;
}