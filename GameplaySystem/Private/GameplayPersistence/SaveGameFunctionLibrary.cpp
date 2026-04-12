// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "SaveGameFunctionLibrary.h"
#include "SaveGameProxyArchive.h"

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
		return (bIsLoading || bIsMovable) && Archive.SerializeField(TEXT("ActorTransform"), [&](FStructuredArchive::FSlot Slot)
			{
				FTransform ActorTransform;

				if (!bIsLoading)
				{
					ActorTransform = Actor->GetActorTransform();
				}

				// Serialize the transform
				Slot << ActorTransform;

				if (bIsLoading && bIsMovable)
				{
					// If the actor is movable, set its transform
					Actor->SetActorTransform(ActorTransform, false, nullptr, ETeleportType::TeleportPhysics);
				}
			});
	}

	return false;
}