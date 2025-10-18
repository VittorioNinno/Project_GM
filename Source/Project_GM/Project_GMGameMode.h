// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Project_GMGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class AProject_GMGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	AProject_GMGameMode();
};



