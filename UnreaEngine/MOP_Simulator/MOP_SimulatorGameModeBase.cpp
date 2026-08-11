// Copyright Epic Games, Inc. All Rights Reserved.


#include "MOP_SimulatorGameModeBase.h"

#include "MOP_Simulator.h"


void AMOP_SimulatorGameModeBase::BeginPlay() {
	Super::BeginPlay();

	// 1. Define where the bomb spawns (e.g., 10,000 meters in the air)
	FVector SpawnLocation(0.0f, 0.0f, 10000.0f);
	FRotator SpawnRotation(0.0f, 0.0f, 0.0f);

	// 2. Spawn the bomb!
	AProjectile* MyBomb = GetWorld()->SpawnActor<AProjectile>(
		AProjectile::StaticClass(), SpawnLocation, SpawnRotation);
};