// Copyright (c) 2026 Omid Teimory. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MOPTypes.h"
#include "MOPProjectileDataAsset.generated.h"


// -----------------------------------------------------------------------
// UDataAsset for projectile configuration.
// Create instances in the Content Browser and assign them to
// UMOPSimulatorComponent to swap between different projectile specs
// (GBU-57 MOP, Rods from God, custom) without touching any C++ code.
// -----------------------------------------------------------------------
UCLASS(BlueprintType)
class MOP_SIMULATOR_API UMOPProjectileDataAsset : public UDataAsset {
	GENERATED_BODY()

public:
	// Full projectile physical specification — all fields editable in the UE Editor.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MOP Projectile Configuration")
	FMOPProjectile ProjectileData;

	UMOPProjectileDataAsset() {
		// Pre-fill with GBU-57 MOP (Midnight Hammer) defaults so a newly
		// created asset is immediately usable without manual data entry.
		ProjectileData.Name                   = TEXT("GBU-57 Massive Ordnance Penetrator (MOP)");
		ProjectileData.Length                 = 6.2;
		ProjectileData.Diameter               = 0.8;
		ProjectileData.CurvatureNoseReduce    = 4.8;
		ProjectileData.TotalMass              = 13600.0;
		ProjectileData.ExplosiveMass          = 2423.0;
		ProjectileData.CasingDensity          = 7850.0;
		ProjectileData.YieldStrength          = 2.5e9;
		ProjectileData.AreaMomentInertia      = 0.0201;
		ProjectileData.ElasticModulus         = 200.0e9;
		ProjectileData.CasingWallThickness    = 0.1;
		ProjectileData.HugoniotC0             = 4570.0;
		ProjectileData.HugoniotS              = 1.49;
		ProjectileData.ExplosiveCriticalEnergy = 3.0e15;
		ProjectileData.ExplosiveEnergyJPerKg  = 5.2e6;
		ProjectileData.SpecificHeat           = 460.0;
		ProjectileData.MeltingPoint           = 1800.0;
		ProjectileData.HeatOfFusion           = 272000.0;
	}
};
