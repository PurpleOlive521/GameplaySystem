// Copyright (c) 2026, Oliver Österlund Stare. All rights reserved.


#include "GameplayEffectTest.h"
#include "Kismet/GameplayStatics.h"

UWorld* GetWorld()
{
    if (GEngine) 
    {
		if (FWorldContext* WorldContext = GEngine->GetWorldContextFromPIEInstance(0))
		{
			return WorldContext->World();
		}
    }

    return nullptr;
}

void ExitWorld()
{
	if (UWorld* World = GetWorld())
	{
		if (APlayerController* TargetPC = UGameplayStatics::GetPlayerController(World, 0))
		{
			TargetPC->ConsoleCommand(TEXT("Exit"), true);
		}
	}
}
