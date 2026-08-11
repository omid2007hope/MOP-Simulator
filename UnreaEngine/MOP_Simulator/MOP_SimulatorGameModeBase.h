// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MOP_SimulatorGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class MOP_SIMULATOR_API AMOP_SimulatorGameModeBase :



    public AGameModeBase {
	GENERATED_BODY()

protected:
	virtual void beginPlay() override;
};
